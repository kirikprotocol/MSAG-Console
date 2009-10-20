#ifndef _SCAG_PROTOTYPES_INFOSME_TASK_H
#define _SCAG_PROTOTYPES_INFOSME_TASK_H

#include <list>
#include <vector>
#include <map>
#include "Message.h"
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
    typedef std::map< int, MessageList >  RegionMap;

    static unsigned getNextId();

public:
    friend class TaskGuard;

    Task( unsigned priority,
          unsigned speed,
          unsigned nregions,
          unsigned messages );

    unsigned getId() const { return id_; }
    const std::string& getName() const { return name_; }

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
        std::sprintf(buf, "%s %c score=%u prio=%u spd=%u sent=%u wdsn=%u stat=%s",
                     name_.c_str(),
                     isActive_ ? '+' : '-',
                     normScore_, priority_, speed_,
                     sent_, wouldSend_, stat.c_str() );
        return buf;
    }

    inline bool operator < ( const Task& other ) const {
        if ( normScore_ < other.normScore_ ) return true;
        if ( other.normScore_ < normScore_ ) return false;
        if ( priority_ > other.priority_ ) return true;
        if ( priority_ < other.priority_ ) return false;
        if ( speed_ > other.speed_ ) return true;
        if ( speed_ < other.speed_ ) return false;
        if ( name_ < other.name_ ) return true;
        return false;
    }

    bool isActive() const { return isActive_; }

    void setActive( bool active ) {
        if ( isActive_ == active ) return;
        MutexGuard mg(taskLock_);
        if ( isDestroyed_ ) { isActive_ = false; }
        else { isActive_ = active; }
    }

    bool isDestroyed() const { return isDestroyed_; }

    void setDestroy() {
        MutexGuard mg(taskLock_);
        isDestroyed_ = true;
    }

    /// checking if task has more messages
    bool hasMessages() const { return messages_ > 0; }

    /// adding more messages to the task
    void addMessages( unsigned nregions, unsigned msgs );

    /// method checks how many messges the task has sent up to deltaTime from start.
    /// if returned value is 0, it means that a new message should be sent,
    /// otherwise that task wants to sleep RV msec until sending a message.
    unsigned wantToSleep( unsigned deltaTime );

    /// suspend message
    void suspendMessage( Message& msg );

    /// get the next message for given region.
    /// the method may fail for a number of reasons:
    /// no more messages in task, all regions are suspended, all messages are in future, etc.
    bool getMessage( time_t now, unsigned regionId, Message& msg );

    /// finalize message
    void finalizeMessage( Message& msg, int state );

    /// normalize score according to the task with lowest score
    void normalizeScore( const Task* lowestScoreTask, unsigned deltaTime = 0 );

    /// message is sent, it should be removed from the list and
    /// the score should be updated.
    void incrementScore( Message& msg );

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

private:
    unsigned    id_;
    smsc::logger::Logger* log_;
    std::string name_;

    bool        isActive_;   // if the task is active
    bool        isDestroyed_;
    util::Drndm random_;     // random number generator

    unsigned priority_;  // priority of the task
    unsigned speed_;     // number of messages per second
    unsigned kilospeed_; // number of messages*1000 per second, derived from speed_
    unsigned sent_;      // actual number of messages sent

    unsigned score_;     // score of the task
    unsigned normScore_; // normalized score of the task
    unsigned wouldSend_; // how many messages would be sent

    unsigned messages_;  // total number of messages in task
    unsigned users_;

    smsc::core::synchronization::Mutex taskLock_;

    // messages in processing (those which were obtained by get)
    MessageList processing_;

    // messages suspended by regions
    RegionMap           regionMap_;

    // these two structures represent a storage
    MessageList messageList_; // messages not in processing

    StatVector  stats_; // statistics by regions
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
