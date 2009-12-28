#include <algorithm>
#include "Processor.h"
#include "TaskDispatcher.h"
#include "Sender.h"
#include "ProtoException.h"
#include "scag/util/Time.h"
#include "util/PtrLess.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

Processor::Processor( Sender& sender, TaskDispatcher& disp ) :
log_(smsc::logger::Logger::getInstance(taskName())),
sender_(&sender),
dispatcher_(&disp),
notified_(true)
{
    smsc_log_debug(log_,"ctor");
}


Processor::~Processor()
{
    checkDestroy();
    inactiveTasks_.clear();
    taskMap_.clear();
    for ( GuardedTaskList::iterator i = allTasks_.begin();
          i != allTasks_.end();
          ) {
        i = allTasks_.erase(i);
    }
    smsc_log_debug(log_,"dtor");
}


TaskGuard Processor::getTask( unsigned idx )
{
    if ( stopping() ) return TaskGuard();
    MutexGuard mg(releaseMon_);
    TaskMap::iterator i = taskMap_.find(idx);
    if ( i == taskMap_.end() ) return TaskGuard();
    return *(i->second);
}


void Processor::addTask( Task* task )
{
    if ( stopping() ) {
        delete task;
        return;
    }
    MutexGuard mg(releaseMon_);
    GuardedTaskList::iterator i = allTasks_.insert(allTasks_.end(),TaskGuard(task));
    taskMap_.insert(std::make_pair(task->getId(),i));
    inactiveTasks_.push_back(task);
    notified_ = true;
    releaseMon_.notify();
}


void Processor::notify()
{
    MutexGuard mg(releaseMon_);
    notified_ = true;
    releaseMon_.notify();
}


void Processor::setTaskEnabled( unsigned idx, bool active )
{
    MutexGuard mg(releaseMon_);
    TaskMap::iterator i = taskMap_.find(idx);
    if ( i == taskMap_.end() ) return;
    Task* task = i->second->get();
    if (task) {
        task->setEnabled(active);
        notified_ = true;
    }
    releaseMon_.notify();
}


int Processor::doExecute()
{
    const util::msectime_type startTime = util::currentTimeMillis();
    smsc_log_info(log_,"started");

    util::msectime_type currentTime = startTime;
    util::msectime_type startForDelta = startTime;
    util::msectime_type lastStatTime = startTime;
    util::msectime_type nextWakeTime = startTime;
    util::msectime_type lastNotifyTime = startTime;

    while ( ! stopping() ) {

        currentTime = util::currentTimeMillis();
        // 2. waiting on the releaseMon
        {
            int waitTime = int(nextWakeTime - currentTime);
            if ( waitTime > 0 ) {
                if ( waitTime < 10 ) waitTime = 10;
                smsc_log_debug(log_,"want to sleep %u ms",waitTime);
                MutexGuard mg(releaseMon_);
                if ( stopping() ) break;
                if ( ! notified_ ) {
                    releaseMon_.wait(waitTime);
                    if ( ! notified_ ) continue;
                    if ( stopping() ) break;
                }
            }
        }

        const unsigned deltaTime = unsigned(currentTime - startForDelta);
        if ( deltaTime > 1000000 ) {
            // if passed more that 1000 seconds then move startTime
            startForDelta = currentTime;
            smsc_log_info(log_,"making a flip of startTime to %llu",currentTime);
            continue;
        }

        smsc_log_debug(log_,"new pass at %u, notified=%u",deltaTime,notified_);

        // 1. dumping statistics
        if ( log_->isInfoEnabled() && ( currentTime - lastStatTime > 10000 ) ) {
            dumpStatistics(unsigned((currentTime-startTime)/1000));
            lastStatTime = currentTime;
        }

        while ( notified_ || (currentTime - lastNotifyTime) > 30000 ) {
            // 2b. meanwhile we may add tasks that were requested to be activated
            MutexGuard mg(releaseMon_);
            notified_ = false;
            lastNotifyTime = currentTime;
            TaskList inactive = dispatcher_->collectInactiveTasks();
            if ( log_->isDebugEnabled() && ! inactive.empty() ) {
                smsc_log_debug(log_,"dispatcher has collected inactive tasks:");
                for ( TaskList::iterator i = inactive.begin(); i != inactive.end(); ++i ) {
                    smsc_log_debug(log_," %s",(*i)->toString().c_str());
                }
            }
            std::copy( inactive.begin(), inactive.end(),
                       std::back_inserter(inactiveTasks_));
            processInactiveTasks();
        }
            
        // 4. send a request to process one region
        nextWakeTime = currentTime + sender_->send( deltaTime, 500 );

        if ( dispatcher_->hasInactiveTask() ) {
            MutexGuard mg(releaseMon_);
            notified_ = true;
        }

    } // main loop
    smsc_log_info(log_,"finishing");
    dumpStatistics( unsigned((util::currentTimeMillis()-startTime)/1000));
    return 0;
}


void Processor::dumpStatistics( unsigned liveTime )
{
    std::string stat;
    stat.reserve(10000);
    {
        char buf[100];
        sprintf(buf,"statistics, working=%u s", liveTime);
        stat.append(buf);
    }
    sender_->dumpStatistics(stat);
    {
        typedef std::vector< Task* > TaskVector;
        TaskVector all;
        all.reserve(100);
        MutexGuard mg(releaseMon_);
        for ( GuardedTaskList::const_iterator i = allTasks_.begin();
              i != allTasks_.end(); ++i ) {
            all.push_back(i->get());
        }
        std::sort( all.begin(), all.end(), PtrLess() );
        unsigned count = 0;
        for ( TaskVector::const_iterator i = all.begin(); i != all.end(); ++i ) {
            char buf[20];
            sprintf(buf,"\n  %3u. ",count++);
            stat.append(buf);
            stat.append((*i)->toString());
        }
    }
    smsc_log_info(log_,"%s",stat.c_str());
}


void Processor::processInactiveTasks()
{
    if ( inactiveTasks_.empty() ) return;
    smsc_log_debug(log_,"processing inactive tasks");
    TaskList deadTasks;
    for ( TaskList::iterator i = inactiveTasks_.begin();
          i != inactiveTasks_.end();
          ) {
        
        Task* task = *i;
        if ( !task ) {
            i = inactiveTasks_.erase(i);
            continue;
        }
        smsc_log_debug(log_,"task %s",task->toString().c_str());
        if ( task->isDestroyed() ) {
            deadTasks.push_back(task);
            i = inactiveTasks_.erase(i);
            smsc_log_debug(log_,"moving task %s to dead list",task->getName().c_str());
            continue;
        }
        if ( task->isActive() ) {
            dispatcher_->addTask( *task );
            i = inactiveTasks_.erase(i);
            smsc_log_debug(log_,"moving task %s to active list",task->getName().c_str());
            continue;
        }
        ++i;
    }

    if ( ! deadTasks.empty() ) {
        for ( TaskList::iterator i = deadTasks.begin(); i != deadTasks.end(); ++i ) {
            TaskMap::iterator iter = taskMap_.find((*i)->getId());
            if ( iter == taskMap_.end() ) continue;
            Task* task = iter->second->get();
            if ( task ) { smsc_log_debug(log_,"destroying task %s",task->toString().c_str()); }
            allTasks_.erase(iter->second);
            taskMap_.erase(iter);
        }
    }
}

}
}
}
