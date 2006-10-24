/***************************************************************************
  tag: Peter Soetens  Mon Jun 10 14:43:39 CEST 2002  fosi.h 

                        fosi.h -  description
                           -------------------
    begin                : Mon June 10 2002
    copyright            : (C) 2002 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 
/**
 * This file translates the FOSI (Framework Operating System Interface) from
 * orocos calls to native RTOS calls  
 *
 */

#ifndef __FOSI_H
#define __FOSI_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE   // use all Posix features (and then some).
#endif

#define HAVE_FOSI_API

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sched.h>
#include <assert.h> 

#ifdef __cplusplus
extern "C" {
#endif

	// Orocos Implementation (i386 specific)
#include "oro_atomic.h"
#include "oro_bitops.h"

#include "os.h"
#include "os_lxrt.h"
#if !defined(OROBLD_OS_AGNOSTIC) || defined(OROBLD_OS_LXRT_INTERNAL) // define the latter to include nevertheless the RTAI header files

	// force inlining if internal and internal to avoid an extra indirection.
	// otherwise, the inlining is selected by the user in his rtai configuration
#if defined(OROBLD_OS_LXRT_INTERNAL)
#define CONFIG_RTAI_LXRT_INLINE 1
#endif

// include custom redirect-like include
#if ORONUM_RTAI_VERSION == 3
#include <rtai_config.h>
#define ORO_RTAI_CONFIG_VERSION CONFIG_RTAI_VERSION_MAJOR.CONFIG_RTAI_VERSION_MINOR
#else
#include "rtai_config.h"
#define ORO_RTAI_CONFIG_VERSION 2.0
#endif


#if ORONUM_RTAI_VERSION == 3
#include <rtai_lxrt.h>
#include <rtai_posix.h>
#else
#define KEEP_STATIC_INLINE
#include <rtai_declare.h>
#include <rtai_usp_posix.h>
#include <rtai_lxrt_user.h> 

    /**
     * About KEEP_STATIC_INLINE:
     *  What is below is questionable, sometimes it crashes, sometimes not. I disassembled the C and C++ object code,
     *  they were the same for 3.x compilers. (ps)
     *  crashes on C++ stuff when using 2.95 compiler, thus you can not include rtai_lxrt.h or you get multiple
     * defined symbols. Missing defines from rtai_lxrt.h need to be redefined in other headerfile.
     */
//#include <rtai_fifos_lxrt_user.h> 
#endif

	// Finally, define the types we use :
	typedef RT_TASK RTOS_RTAI_TASK;
	typedef SEM     RTOS_RTAI_SEM;
	typedef CND     RTOS_RTAO_CND;

#else // AGNOSTIC

	// For PeriodicTask.cxx :
	// we need to define the types without the headers,
	// this is RTAI version dependent.
#if ORONUM_RTAI_VERSION == 3
    // v3.x :
	typedef struct oro_lxrt_t {
		int opaque;
	} __LXRT_HANDLE_STRUCT;

    typedef __LXRT_HANDLE_STRUCT RTOS_RTAI_TASK;
    typedef __LXRT_HANDLE_STRUCT RTOS_RTAI_SEM;
    typedef __LXRT_HANDLE_STRUCT RTOS_RTAI_CND; 
#else
    // v24.1.x :
	typedef struct oro_lxrt_t {
		int opaque;
	} __LXRT_HANDLE_STRUCT;
    typedef void RTOS_RTAI_TASK;
    typedef __LXRT_HANDLE_STRUCT RTOS_RTAI_SEM;
    typedef void RTOS_RTAI_CND; 
#endif
#endif // OROBLD_OS_AGNOSTIC // for RTAI header files.

	// this is required because the rtos_sem_init function takes a pointer to RTOS_SEM,
	// which contains a pointer to the real RTAI sem.
	typedef struct oro_rtai_sem_t {
		RTOS_RTAI_SEM* sem;
	} rt_sem_t;

	/**
	 * Typdefs
	 */
	int lock_all(int stk, int heap);

#define __LXRT_USERSPACE__


	typedef pthread_mutex_t rt_mutex_t;
    typedef pthread_mutex_t rt_rec_mutex_t;
	
	// Time Related
	
	typedef long long NANO_TIME;
	typedef long long TICK_TIME;
	typedef struct timespec TIME_SPEC;

	typedef struct {
		pthread_t thread;
		char * name;
		int priority;

		RTOS_RTAI_TASK* rtaitask;
	} RTOS_TASK;

#define SCHED_LXRT_HARD 0 /** LXRT Hard real-time */
#define SCHED_LXRT_SOFT 1 /** LXRT Soft real-time */
#define ORO_SCHED_RT    0 /** LXRT Hard real-time */
#define ORO_SCHED_OTHER 1 /** LXRT Soft real-time */




// rtai undef cfr boost::graph library adjacency_list.hpp:443
#undef DS	
#undef OEL
#undef VL
#undef VP
#undef EP
#undef GP
#undef EL

#ifndef OROBLD_OS_AGNOSTIC
	
	// hrt is in ticks
inline TIME_SPEC ticks2timespec(TICK_TIME hrt)
{
	TIME_SPEC timevl;
	timevl.tv_sec = nano2count(hrt) / 1000000000LL;
	timevl.tv_nsec = nano2count(hrt) % 1000000000LL;
	return timevl;
}

// turn this on to have maximum detection of valid system calls.
#ifdef OROSEM_OS_LXRT_CHECK
#define CHK_LXRT_CALL() do { if(rt_buddy() == 0) { \
        printf("LXRT NOT INITIALISED IN THIS THREAD pid=%d,\n\
    BUT TRIES TO INVOKE LXRT FUNCTION >>%s<< ANYWAY\n", getpid(), __FUNCTION__ );\
        assert( rt_buddy() != 0 ); }\
        } while(0)
#define CHK_LXRT_PTR(ptr) do { if(ptr == 0) { \
        printf("TRIED TO PASS NULL POINTER TO LXRT IN THREAD pid=%d,\n\
    IN TRYING TO INVOKE LXRT FUNCTION >>%s<<\n", getpid(), __FUNCTION__ );\
        assert( ptr != 0 ); }\
        } while(0)
#else
#define CHK_LXRT_CALL()
#define CHK_LXRT_PTR( a )
#endif
    
inline NANO_TIME rtos_get_time_ns(void) { return rt_get_time_ns(); }

inline TICK_TIME rtos_get_time_ticks(void) { return rt_get_time(); }

inline TICK_TIME ticksPerSec(void) { return nano2count( 1000 * 1000 * 1000 ); }

	inline TICK_TIME nano2ticks(NANO_TIME t) { return nano2count(t); }
	inline NANO_TIME ticks2nano(TICK_TIME t) { return count2nano(t); }

inline int rtos_nanosleep(const TIME_SPEC *rqtp, TIME_SPEC *rmtp) 
{
    CHK_LXRT_CALL();
    nanosleep(rqtp,rmtp); // rtai 24.1.9
    return 0;
}

    static inline int rtos_sem_init(rt_sem_t* m, int value )
    {
        CHK_LXRT_CALL();
		// store the pointer in m->opaque...
        m->sem = rt_sem_init( rt_get_name(0) , value);
		return m->sem == 0 ? -1 : 0;
    }

    static inline int rtos_sem_destroy(rt_sem_t* m )
    {
        CHK_LXRT_CALL();
        return rt_sem_delete( m->sem );
    }

    static inline int rtos_sem_signal(rt_sem_t* m )
    {
        CHK_LXRT_CALL();
        return rt_sem_signal( m->sem );
    }

    static inline int rtos_sem_wait(rt_sem_t* m )
    {
        CHK_LXRT_CALL();
        return rt_sem_wait( m->sem );
    }

    static inline int rtos_sem_trywait(rt_sem_t* m )
    {
        CHK_LXRT_CALL();
        return rt_sem_wait_if(m->sem);
    }

    static inline int rtos_sem_value(rt_sem_t* m )
    {
        CHK_LXRT_CALL();
        return rt_sem_count(m->sem);
    }

    static inline int rtos_sem_wait_timed(rt_sem_t* m, NANO_TIME delay )
    {
        CHK_LXRT_CALL();
        return rt_sem_wait_timed(m->sem, nano2count(delay) );
    }

    static inline int rtos_mutex_init(rt_mutex_t* m)
    {
        CHK_LXRT_CALL();
        return pthread_mutex_init_rt(m, 0);
    }

    static inline int rtos_mutex_destroy(rt_mutex_t* m )
    {
        CHK_LXRT_CALL();
        return pthread_mutex_destroy_rt(m);
    }

    static inline int rtos_mutex_rec_init(rt_rec_mutex_t* m)
    {
        CHK_LXRT_CALL();
        pthread_mutexattr_t ma_t;
        pthread_mutexattr_init(&ma_t);
		pthread_mutexattr_settype(&ma_t,PTHREAD_MUTEX_RECURSIVE_NP);
        return pthread_mutex_init_rt(m, 0);
    }

    static inline int rtos_mutex_rec_destroy(rt_rec_mutex_t* m )
    {
        CHK_LXRT_CALL();
        return pthread_mutex_destroy_rt(m);
    }

    static inline int rtos_mutex_lock( rt_mutex_t* m)
    {
        CHK_LXRT_CALL();
        return pthread_mutex_lock_rt(m);
    }

    static inline int rtos_mutex_rec_lock( rt_rec_mutex_t* m)
    {
        CHK_LXRT_CALL();
        return pthread_mutex_lock_rt(m);
    }

    static inline int rtos_mutex_trylock( rt_mutex_t* m)
    {
        CHK_LXRT_CALL();
        return pthread_mutex_trylock_rt(m);
    }

    static inline int rtos_mutex_rec_trylock( rt_rec_mutex_t* m)
    {
        CHK_LXRT_CALL();
        return pthread_mutex_trylock_rt(m);
    }

    static inline int rtos_mutex_unlock( rt_mutex_t* m)
    {
        CHK_LXRT_CALL();
        return pthread_mutex_unlock_rt(m);
    }

    static inline int rtos_mutex_rec_unlock( rt_rec_mutex_t* m)
    {
        CHK_LXRT_CALL();
        return pthread_mutex_unlock_rt(m);
    }

inline
int rtos_printf(const char *fmt, ...)
{
    va_list list;
    char printkbuf [2000];
    printkbuf[0] = '\0';
    va_start (list, fmt);
    vsprintf(printkbuf, fmt, list);
    va_end (list);
    // XXX revert to print to screen when debugging is over
    return rtai_print_to_screen(printkbuf);
    //return printf(printkbuf);
}

#else  // OSBLD_OS_AGNOSTIC

/**
 * Only function prototypes.
 */

TIME_SPEC ticks2timespec(TICK_TIME hrt);

NANO_TIME rtos_get_time_ns(void);

TICK_TIME rtos_get_time_ticks(void);

TICK_TIME ticksPerSec(void);

TICK_TIME nano2ticks(NANO_TIME t);

NANO_TIME ticks2nano(TICK_TIME t);

int rtos_nanosleep(const TIME_SPEC *rqtp, TIME_SPEC *rmtp) ;

int rtos_mutex_init(rt_mutex_t* m);

int rtos_mutex_destroy(rt_mutex_t* m );

int rtos_mutex_rec_init(rt_mutex_t* m);

int rtos_mutex_rec_destroy(rt_mutex_t* m );

int rtos_mutex_lock( rt_mutex_t* m);

int rtos_mutex_trylock( rt_mutex_t* m);

int rtos_mutex_unlock( rt_mutex_t* m);

int rtos_mutex_rec_lock( rt_rec_mutex_t* m);

int rtos_mutex_rec_trylock( rt_rec_mutex_t* m);

int rtos_mutex_rec_unlock( rt_rec_mutex_t* m);

int rtos_printf(const char *fmt, ...);

int rtos_sem_init(rt_sem_t* m, int value );
int rtos_sem_destroy(rt_sem_t* m );
int rtos_sem_signal(rt_sem_t* m );
int rtos_sem_wait(rt_sem_t* m );
int rtos_sem_trywait(rt_sem_t* m );
int rtos_sem_value(rt_sem_t* m );
int rtos_sem_wait_timed(rt_sem_t* m, NANO_TIME delay );

#endif // OSBLD_OS_AGNOSTIC

#ifdef __cplusplus
}
#endif

#endif