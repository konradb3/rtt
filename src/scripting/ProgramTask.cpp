/***************************************************************************
  tag: Peter Soetens  Wed Jan 18 14:11:40 CET 2006  ProgramTask.cpp 

                        ProgramTask.cpp -  description
                           -------------------
    begin                : Wed January 18 2006
    copyright            : (C) 2006 Peter Soetens
    email                : peter.soetens@mech.kuleuven.be
 
 ***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public                   *
 *   License as published by the Free Software Foundation;                 *
 *   version 2 of the License.                                             *
 *                                                                         *
 *   As a special exception, you may use this file as part of a free       *
 *   software library without restriction.  Specifically, if other files   *
 *   instantiate templates or use macros or inline functions from this     *
 *   file, or you compile this file and link it with other files to        *
 *   produce an executable, this file does not by itself cause the         *
 *   resulting executable to be covered by the GNU General Public          *
 *   License.  This exception does not however invalidate any other        *
 *   reasons why the executable file might be covered by the GNU General   *
 *   Public License.                                                       *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public             *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/
 
 

#include "rtt/scripting/ProgramTask.hpp"

#include "rtt/CommandDS.hpp"
#include "rtt/Method.hpp"
#include "rtt/TaskContext.hpp"
#include "rtt/FactoryExceptions.hpp"
#include <rtt/DataSources.hpp>

namespace RTT
{
    
    using namespace detail;

    ProgramTask::ProgramTask(ProgramInterfacePtr prog, ExecutionEngine* ee)
        : TaskContext( prog->getName(), ee ),
          program( new ValueDataSource<ProgramInterfaceWPtr>(prog) ) // was: VariableDataSource.
    {
        this->clear();

        DataSource<ProgramInterfaceWPtr>* ptr = program.get();
        // Commands :
        commands()->addCommandDS( ptr,
                                  command_ds("start",&ProgramInterface::start, &ProgramInterface::isRunning,engine()->commands()),
                                  "Start or continue this program.");
        commands()->addCommandDS( ptr,
                                  command_ds("pause",&ProgramInterface::pause, &ProgramInterface::isPaused,engine()->commands()),
                                  "Pause this program.");
        commands()->addCommandDS( ptr,
                                  command_ds("step", &ProgramInterface::step, &ProgramInterface::stepDone,engine()->commands()),
                                  "Step a paused program.");
        commands()->addCommandDS( ptr,
                                  command_ds("stop", &ProgramInterface::stop, &ProgramInterface::isStopped,engine()->commands()),
                                  "Stop and reset this program.");

        // DataSources:

        methods()->addMethodDS( ptr,
                                method_ds("isRunning",&ProgramInterface::isRunning),
                                "Is this program being executed and not paused ?");
        methods()->addMethodDS( ptr,
                                method_ds("inError", &ProgramInterface::inError),
                                "Has this program executed an erroneous command ?");
        methods()->addMethodDS( ptr,
                                method_ds("isPaused", &ProgramInterface::isPaused),
                                "Is this program running but paused ?");
    }

    ProgramTask::~ProgramTask() {
    }
}

