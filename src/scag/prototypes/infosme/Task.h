#ifndef _SCAG_PROTOTYPES_INFOSME_TASK_H
#define _SCAG_PROTOTYPES_INFOSME_TASK_H

#include <list>
#include <vector>
#include <map>
#include "Message.h"
#include "Speed.h"
#include "logger/Logger.h"
#include "scag/util/io/Drndm.h"

namespace scag2 {
namespace prototypes {
namespace infosme {

class TaskGuard;

class Task
{
private:
    // messages sorted by sending time
    typedef std::list< Message >          MessageList;
    typedef std::vector< unsigned >       StatVector;
    typedef std::map< unsigned, MessageList >  RegionMap;

    static unsigned getNextId();

public:
    friend class TaskGuard;

    Task( unsigned priority,
          unsigned speed,
          unsigned nregions,
          unsigned messages );

    inline unsigned getId() const { return id_; }
    inline const std::string& getName() const { return name_; }
    inline unsigned getPriority() const { return priority_; }

    std::string toString() const {
        char buf[256];
        std::string stat;
        stat.reserve(100);
        {
            const char* sep = "";
            char b[30];
            stat.push_back('[');
            for ( StatVector::const_iterator i = stats_.begin(); i != stats_.end(); ++i ) {
                std::sprintf(b,"%s%u",sep,*i);
                stat.append(b);
                sep = ",";
            }
            stat.push_back(']');
        }
        std::sprintf(buf, "%s %c prio=%u spd=%u sent=%u left=%u next=%u stat=%s",
                     name_.c_str(),
                     isActive_ ? '+' : '-',
                     priority_, speed_.getSpeed(),
                     sent_,
                     messages_,
                     speed_.getNextTime(), stat.c_str() );
        return buf;
    }

    inline bool operator < ( const Task& other ) const {
        if ( priority_ > other.priority_ ) return true;
        if ( priority_ < other.priority_ ) return false;
        int cmp = speed_.compare(other.speed_);
        if ( cmp < 0 ) return true;
        if ( cmp > 0 ) return false;
        if ( name_ < other.name_ ) return true;
        return false;
    }

    inline bool isActive() const { return isActive_; }
    inline bool isDestroyed() const { return isDestroyed_; }

    /// checking if task has more messages
    inline bool hasMessages() const { return messages_ > 0; }

    // --- modifiers

    // --- methods invoked from admin, processor and such

    void setActive( bool active ) {
        if ( isActive_ == active ) return;
        MutexGuard mg(taskLock_);
        if ( isDestroyed_ ) { isActive_ = false; }
        else { isActive_ = active; }
    }

    void setDestroy() {
        MutexGuard mg(taskLock_);
        isDestroyed_ = true;
        isActive_ = false;
    }

    /// adding more messages to the task
    void addMessages( unsigned nregions, unsigned msgs );


    // --- methods invoked from dispatcher

    /// method checks how many messges the task has sent up to deltaTime from start.
    /// if returned value is 0, it means that a new message should be sent,
    /// otherwise that task wants to sleep RV msec until sending a message.
    unsigned isReady( unsigned deltaTime );

    /// get the next message for given region.
    /// Typically prefetch is invoked first.
    /// the method may fail for a number of reasons:
    /// no more messages in task, all regions are suspended, all messages are in future, etc.
    bool prefetchMessage( time_t now, unsigned regionId );
    bool getMessage( time_t now, unsigned regionId, Message& msg );

    /// Set message state.
    /// This method is guaranteed to be invoked after the processing of the message.
    /// @param state is equal to one of MessageState.
    void setMessageState( Message& msg, int state );

private:
    bool changeUsage( bool v ) {
        MutexGuard mg(taskLock_);
        if ( v ) {
            ++users_;
            return false;
        } else {
            if ( users_ > 0 ) --users_;
            return (users_ == 0);
        }
    }

    bool setPrefetch( MessageList& list );
    void doSuspendMessage( Message& msg );
    bool doPrefetchMessage( time_t now, unsigned regionId );

private:
    unsigned    id_;
    smsc::logger::Logger* log_;
    std::string name_;

    bool        isActive_;   // if the task is active
    bool        isDestroyed_;
    util::Drndm random_;     // random number generator

    SpeedControl speed_;
    unsigned priority_;  // priority of the task
    unsigned sent_;      // actual number of messages sent
    unsigned messages_;  // total number of unsent messages in task
    unsigned users_;

    smsc::core::synchronization::Mutex taskLock_;

    // messages sorted by regions
    RegionMap   regionMap_;

    // these structure represent a CsvStore
    MessageList messageList_; // messages not in processing

    StatVector  stats_; // statistics by regions

    bool    prefetched_;
    Message prefetch_;
};


class TaskGuard
{
public:
    inline TaskGuard() : task_(0) {}
    inline TaskGuard( Task* task ) : task_(task) {
        if (task_) task_->changeUsage(true);
    }
    inline ~TaskGuard() {
        if (task_) {
            if ( task_->changeUsage(false) ) delete task_;
        }
    }
    inline TaskGuard( const TaskGuard& tg ) : task_(tg.task_) {
        if (task_) task_->changeUsage(true);
    }
    inline TaskGuard& operator = ( const TaskGuard& tg ) {
        if ( &tg != this ) {
            task_ = tg.task_;
            if (task_) task_->changeUsage(true);
        }
        return *this;
    }

    inline Task* get() const {
        return task_;
    }

private:
    Task* task_;
};

}
}
}

#endif
