#include <algorithm>
#include "Processor.h"
#include "TaskDispatcher.h"
#include "Sender.h"
#include "ProtoException.h"
#include "scag/util/Time.h"
#include "scag/util/PtrLess.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

Processor::Processor( Sender& sender, TaskDispatcher& disp ) :
log_(smsc::logger::Logger::getInstance(taskName())),
sender_(&sender),
dispatcher_(&disp)
{
    smsc_log_debug(log_,"ctor");
}


Processor::~Processor()
{
    checkDestroy();
    inactiveTasks_.clear();
    activeTasks_.clear();
    deadTasks_.clear();
    for ( GuardedTaskList::iterator i = allTasks_.begin();
          i != allTasks_.end();
          ) {
        i = allTasks_.erase(i);
    }
    taskMap_.clear();
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
    releaseMon_.notify();
}


void Processor::notify()
{
    MutexGuard mg(releaseMon_);
    releaseMon_.notify();
}


int Processor::doExecute()
{
    const util::msectime_type startTime = util::currentTimeMillis();
    smsc_log_info(log_,"started");

    util::msectime_type currentTime = startTime;
    util::msectime_type lastStatTime = startTime;
    unsigned wantToSleep = 0;

    while ( ! stopping() ) {

        const util::msectime_type nextWakeTime = currentTime + wantToSleep;
        currentTime = util::currentTimeMillis();
        const unsigned deltaTime = unsigned(currentTime - startTime);
        smsc_log_debug(log_,"new pass at %u",deltaTime);

        // 1. dumping statistics
        if ( log_->isInfoEnabled() && ( currentTime - lastStatTime > 10000 ) ) {
            dumpStatistics(deltaTime);
            lastStatTime = currentTime;
        }

        // 2. waiting on the releaseMon
        {
            int waitTime = int(nextWakeTime - currentTime);
            if ( waitTime > 10 ) {
                MutexGuard mg(releaseMon_);
                if ( stopping() ) break;
                releaseMon_.wait(waitTime);
                continue;
            }
        }


        {
            // 2b. meanwhile we may add tasks that were requested to be activated
            // FIXME: make a fast bool flag if such task exists and set it in setActive
            MutexGuard mg(releaseMon_);
            for ( TaskList::iterator i = inactiveTasks_.begin();
                  i != inactiveTasks_.end();
                  ) {

                Task* task = *i;
                if ( task->isDestroyed() ) {
                    deadTasks_.push_back(task);
                    i = inactiveTasks_.erase(i);
                    smsc_log_debug(log_,"moving task %s to dead list",task->getName().c_str());
                    continue;
                }

                if ( task->isActive() && task->hasMessages() ) {
                    // const Task* firstTask = activeTasks_.empty() ? 0 : activeTasks_.front();
                    // task->normalizeScore( firstTask, deltaTime );
                    /*
                    ActiveTaskList::iterator j = std::lower_bound( activeTasks_.begin(),
                                                                   activeTasks_.end(),
                                                                   task, PtrLess() );
                    activeTasks_.insert(j,task);
                     */
                    dispatcher_->addTask( *task );
                    i = inactiveTasks_.erase(i);
                    smsc_log_debug(log_,"moving task %s to active list",task->getName().c_str());
                    continue;
                }
                ++i;
            }
        }

        // 4. send a request to process one region
        wantToSleep = sender_->send( deltaTime, 500 );

        /*
        if ( wantToSleep > 0 ) {

            // 4a. sender really want to sleep, no regions are ready
            if ( wantToSleep > 500 ) { wantToSleep = 500; }
            
            // going to sleep on regions
            continue;
        }
        
        // here a regionId is ready
        smsc_log_debug(log_,"region %u is ready, fetching a task",regionId);

        if ( log_->isDebugEnabled() ) {
            unsigned idx = 0;
            for ( ActiveTaskList::iterator i = activeTasks_.begin();
                  i != activeTasks_.end();
                  ++i ) {
                smsc_log_debug(log_,"%3u %s",idx,(*i)->toString().c_str());
                if ( ++idx > 10 ) break;
            }
        }

        // 5. the loop over all active tasks
        wantToSleep = 500;
        const time_t now = time(0);
        TaskList makeInactiveList;
        bool activeListChanged = false;
        Task* movedTask = 0;
        for ( ActiveTaskList::iterator i = activeTasks_.begin();
              i != activeTasks_.end();
              ++i ) {

            Task* task = *i;
            if ( ! task ) {
                activeListChanged = true;
                continue;
            }

            // 5a. task is dead
            if ( task->isDestroyed() ) {
                activeListChanged = true;
                deadTasks_.push_back(*i);
                *i = 0;
                continue;
            }

            // 5b. task is not active, remove it
            if ( !task->isActive() || !task->hasMessages() ) {
                activeListChanged = true;
                makeInactiveList.push_back(*i);
                *i = 0;
                continue;
            }
            
            // 5c. restriction on the speed of the task
            unsigned taskWantToSleep = task->wantToSleep( deltaTime );
            if ( taskWantToSleep > 0 ) {
                if ( taskWantToSleep < wantToSleep ) wantToSleep = taskWantToSleep;
                continue;
            }

            Message msg;
            if ( ! task->getMessage(now,regionId,msg) ) {
                // cannot get a message for the region
                continue;
            }


            // 5d. task wants to send a message
            wantToSleep = 0;
            int reason = MessageState::UNKNOWN;
            try {
                reason = sender_->send(deltaTime, msg);
            } catch ( std::exception& e ) {
                smsc_log_warn(log_,"sending failed: %s", e.what());
                reason = MessageState::UNKNOWN;
            }


            if ( reason == MessageState::OK ) {
                // 5e. message has been sent
                task->incrementScore(msg);
                movedTask = task;  // remember the task to move
                // reorder it
                if ( activeListChanged ) {
                    *i = 0;
                } else {
                    // simply erase the position
                    activeTasks_.erase(i);
                }
                break;
            }

            // 5f. failed to send
            if ( reason == MessageState::NOCONN ) {
                task->finalizeMessage(msg,reason);
                if ( ! task->hasMessages() ) {
                    makeInactiveList.push_back(task);
                    *i = 0;
                    activeListChanged = true;
                }
                break;
            } else {
                task->suspendMessage(msg);
                break;
            }

        } // loop over tasks

        // 6. suspend the region
        if ( wantToSleep > 0 ) {
            sender_->suspendConnector(deltaTime+wantToSleep,regionId);
            wantToSleep = 0; // to prevent sleeping on regions
        }

        // 6. remove tasks from the list
        if ( activeListChanged ) {
            activeTasks_.erase( std::remove( activeTasks_.begin(),
                                             activeTasks_.end(),
                                             static_cast<Task*>(0) ),
                                activeTasks_.end() );
        }
        if ( movedTask ) {
            // a task has sent something
            ActiveTaskList::iterator i = std::lower_bound( activeTasks_.begin(),
                                                           activeTasks_.end(),
                                                           movedTask, PtrLess() );
            const Task* firstTask = ( activeTasks_.empty() ? 0 : activeTasks_.front() );
            movedTask->normalizeScore( firstTask, 0 );
            activeTasks_.insert(i,movedTask);
        }

        MutexGuard mg(releaseMon_);

        // 7. add all inactiveTasks to inactive and destroy all deadTasks
        inactiveTasks_.splice(inactiveTasks_.end(),makeInactiveList);

        // 8. destroy all tasks waiting destruction
        for ( TaskList::iterator i = deadTasks_.begin();
              i != deadTasks_.end(); ++i ) {
            const unsigned taskId = (*i)->getId();
            TaskMap::iterator j = taskMap_.find(taskId);
            if ( j == taskMap_.end() ) {
                smsc_log_warn(log_,"task %u is not found", taskId);
                continue;
            }
            allTasks_.erase(j->second);
            taskMap_.erase(j);
        }
         */

    } // main loop
    smsc_log_info(log_,"finishing");
    dumpStatistics( unsigned(util::currentTimeMillis() - startTime) );
    return 0;
}


void Processor::dumpStatistics( unsigned deltaTime )
{
    std::string stat;
    stat.reserve(10000);
    {
        char buf[100];
        sprintf(buf,"statistics, time=%u ",deltaTime);
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

}
}
}
