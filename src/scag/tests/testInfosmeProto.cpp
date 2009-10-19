#include <string>
#include <iostream>
#include <list>
#include <cassert>
#include <algorithm>
#include <vector>
#include <map>
#include <set>

#include "logger/Logger.h"
#include "scag/util/Time.h"
#include "scag/util/PtrDestroy.h"
#include "scag/util/PtrLess.h"
#include "scag/util/io/Drndm.h"
#include "scag/util/WatchedThreadedTask.h"
#include "core/threads/ThreadPool.hpp"
#include "util/timeslotcounter.hpp"

using namespace scag2;

// ==========================================

class ProtoException : public smsc::util::Exception
{
public:
    ProtoException( const char* fmt, ... ) {
        SMSC_UTIL_EX_FILL(fmt);
    }
};


// ==========================================

class MessageState
{
public:
    enum {
            UNKNOWN = -1,
            OK = 0,
            NOCONN,
            LIMITED,
            FAIL
    };
};


// ==========================================


class Message
{
private:
    static unsigned getNextId();

public:
    Message() {}
    Message(unsigned regionId, time_t start) :
    id_(getNextId()), regionId_(regionId), start_(start) {}

    unsigned getId() const { return id_; }
    unsigned getRegionId() const { return regionId_; }
    time_t startTime() const { return start_; }

    bool operator < ( const Message& other ) const {
        if ( start_ < other.start_ ) return true;
        if ( start_ > other.start_ ) return false;
        if ( id_ < other.id_ ) return true;
        return false;
    }

private:
    unsigned    id_;
    unsigned    regionId_;
    time_t start_;
};


smsc::core::synchronization::Mutex messageIdLock;
unsigned Message::getNextId() {
    MutexGuard mg(messageIdLock);
    static unsigned id = 0;
    return id++;
}


// ======================================


class Connector
{
private:
    static unsigned getNextId();

public:
    Connector( unsigned bandwidth ) :
    id_(getNextId()),
    log_(0),
    bandwidth_(bandwidth),
    speedLimiter_(1,1),
    sent_(0)
    {
        char buf[40];
        sprintf(buf,"conn.%03u",id_);
        name_ = buf;
        log_ = smsc::logger::Logger::getInstance(name_.c_str());
        random_.setSeed(time(0));
    }

    int send( Message& msg );

    unsigned getId() const { return id_; }

    std::string toString() const {
        char buf[256];
        std::sprintf(buf,"%s band=%u sent=%u",name_.c_str(),bandwidth_,sent_);
        return buf;
    }

private:
    unsigned id_;
    smsc::logger::Logger* log_;
    std::string name_;
    unsigned bandwidth_;    // msg per sec
    unsigned sent_;
    smsc::util::TimeSlotCounter<int> speedLimiter_;
    scag2::util::Drndm random_;
};


smsc::core::synchronization::Mutex connIdLock;
unsigned Connector::getNextId()
{
    MutexGuard mg(connIdLock);
    static unsigned id = 0;
    return id++;
}


int Connector::send( Message& msg )
{
    int out = speedLimiter_.Get();
    bool limitReached = ( out >= bandwidth_ );
    if ( limitReached ) {
        // smsc_log_debug(log_,"limit reached");
        return MessageState::LIMITED;
    }

    // making additional failures
    uint64_t r = random_.getNextNumber();
    if ( random_.uniform(1000,r) < 2 ) {
        smsc_log_debug(log_,"failed");
        return MessageState::FAIL;
    }

    // sending
    ++sent_;
    speedLimiter_.Inc();
    return MessageState::OK;
}


// ======================================



class TaskGuard;

class Task
{
private:
    // messages sorted by sending time
    typedef std::list< Message >          MessageList;
    struct RegionMapValue {
        bool        enabled;
        MessageList list;
    };
    typedef std::map< int, RegionMapValue >  RegionMap;

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
        std::sprintf(buf, "%s %c score=%u prio=%u spd=%u sent=%u wdsn=%u",
                     name_.c_str(),
                     isActive_ ? '+' : '-',
                     normScore_, priority_, speed_,
                     sent_, wouldSend_ );
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

    void normalizeScore( unsigned deltaTime, unsigned normScoreToSet ) {
        MutexGuard mg(taskLock_);
        normScore_ = normScoreToSet;
        score_ = normScore_ * priority_ / 100;
        wouldSend_ = deltaTime * speed_;
    }

    inline unsigned getNormalizedScore() const { return normScore_; }

    /// checking if task has more messages
    bool hasMessages() const { return messages_ > 0; }

    /// adding more messages to the task
    void addMessages( unsigned nregions, unsigned msgs );

    /// method checks how many messges the task has sent up to deltaTime from start.
    /// if returned value is 0, it means that a new message should be sent,
    /// otherwise that task wants to sleep RV msec until sending a message.
    unsigned wantToSleep( unsigned deltaTime );

    /// reset all suspended regions for this task
    void resetSuspendedRegions();

    /// suspend region
    void suspendRegion( Message& msg );

    /// get the next message.
    /// the method may fail for a number of reasons:
    /// no more messages in task, all regions are suspended, all messages are in future, etc.
    bool getMessage( Message& msg );

    /// finalize message
    void finalizeMessage( Message& msg, int state );

    /// message is sent, it should be removed from the list and
    /// the score should be updated.
    void incrementScore( Message& msg );

private:
    // NOTE: not locked
    RegionMap::iterator findRegion( unsigned regionId );

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
    RegionMap::iterator regionMapIter_;

    // these two structures represent a storage
    MessageList messageList_; // messages not in processing
};


smsc::core::synchronization::Mutex taskIdMutex;
unsigned Task::getNextId() {
    MutexGuard mg(taskIdMutex);
    static unsigned taskId = 0;
    return taskId++;
}


Task::Task( unsigned priority,
            unsigned speed,
            unsigned nregions,
            unsigned messages ) :
id_(getNextId()),
log_(0),
isActive_(false),
isDestroyed_(false),
priority_(priority),
speed_(speed),
kilospeed_(speed*1000),
sent_(0),
score_(0),
normScore_(0),
wouldSend_(0),
messages_(0),
users_(0),
regionMapIter_(regionMap_.begin()) 
{
    char buf[50];
    sprintf(buf,"task.%03u",id_);
    name_ = buf;
    log_ = smsc::logger::Logger::getInstance(name_.c_str());
    random_.setSeed( time(0) );
    assert( priority_ > 0 );
    assert( speed_ > 0 );
    addMessages( nregions, messages );
}


class TaskGuard
{
public:
    TaskGuard() : task_(0) {}
    TaskGuard( Task* task ) : task_(task) {
        if (task_) task_->changeUsage(true);
    }
    ~TaskGuard() {
        if (task_) {
            if ( task_->changeUsage(false) ) delete task_;
        }
    }
    TaskGuard( const TaskGuard& tg ) : task_(tg.task_) {
        if (task_) task_->changeUsage(true);
    }
    TaskGuard& operator = ( const TaskGuard& tg ) {
        if ( &tg != this ) {
            task_ = tg.task_;
            if (task_) task_->changeUsage(true);
        }
        return *this;
    }

    Task* get() const {
        return task_;
    }

private:
    Task* task_;
};


void Task::addMessages( unsigned nregions, unsigned msgs )
{
    time_t now = time(0);
    MutexGuard mg(taskLock_);
    if ( ! messageList_.empty() ) {
        now = messageList_.back().startTime();
    }
    unsigned packsize = 0;
    // looking for a position to insert
    Message tmp(0,now);
    MessageList::iterator iter = std::lower_bound( messageList_.begin(),
                                                   messageList_.end(),
                                                   tmp );
    for ( unsigned n = 0; n < msgs; ++n ) {
        if ( packsize == 0 ) {
            now += 1;
            uint64_t r = random_.getNextNumber();
            packsize = 1 + unsigned(random_.uniform(speed_,r));
        }
        --packsize;
        uint64_t r = random_.getNextNumber();
        unsigned regionId = unsigned(random_.uniform(nregions,r));
        Message msg(regionId,now);
        while ( iter != messageList_.end() && !(msg < *iter) ) {
            ++iter;
        }
        iter = messageList_.insert(iter,msg);
        ++iter;
    }
    messages_ += msgs;
}


unsigned Task::wantToSleep( unsigned deltaTime )
{
    const unsigned wouldSend = deltaTime * speed_;
    if ( wouldSend < wouldSend_ ) {
        // task has sent more than it should be, return ms
        unsigned want = (wouldSend_ - wouldSend) / speed_ + 1;
        smsc_log_debug(log_,"wouldSend=%u task.wouldSend=%u, want to sleep %u ms",
                       wouldSend, wouldSend_, want );
        return want;
    }

    // task wants to send a message
    if ( wouldSend > wouldSend_ + kilospeed_ ) {
        // difference is more than one second, correcting to prevent too many
        // messages from this task
        unsigned newval = wouldSend - kilospeed_;
        smsc_log_debug(log_,"wouldSend=%u task.wouldSend=%u fixing=%u",
                       wouldSend, wouldSend_, newval );
        wouldSend_ = newval;
    }
    return 0;
}


void Task::resetSuspendedRegions()
{
    MutexGuard mg(taskLock_);
    for ( RegionMap::iterator i = regionMap_.begin(); i != regionMap_.end(); ++i ) {
        i->second.enabled = true;
    }
}


void Task::suspendRegion( Message& msg )
{
    unsigned regionId = msg.getRegionId();
    {
        MutexGuard mg(taskLock_);
        RegionMap::iterator i = findRegion( regionId );
        i->second.enabled = false;
        i->second.list.push_front(msg);
    }
    // smsc_log_debug(log_,"region %u suspended",regionId);
}


bool Task::getMessage( Message& msg )
{
    if ( isDestroyed_ || !isActive_ || !messages_ ) return false;
    MutexGuard mg(taskLock_);
    if ( isDestroyed_ || !isActive_ || !messages_ ) return false;
    
    // looking for regions
    bool res = false;
    if ( ! regionMap_.empty() ) {
        RegionMap::iterator oldIter = regionMapIter_;
        while ( ! res ) {
            while ( regionMapIter_->second.enabled ) {
                MessageList& list = regionMapIter_->second.list;
                if ( list.empty() ) break;
                msg = list.front();
                list.pop_front();
                res = true;
                break;
            }
            ++regionMapIter_;
            if ( regionMapIter_ == regionMap_.end() ) regionMapIter_ = regionMap_.begin();
            if ( regionMapIter_ == oldIter ) break;
        }
    }

    if ( !res ) {
        // trying to get messages from storage
        while ( ! messageList_.empty() ) {
            msg = messageList_.front();
            messageList_.pop_front();
            RegionMap::iterator i = findRegion(msg.getRegionId());
            if ( i->second.enabled ) {
                res = true;
                break;
            }
            i->second.list.push_back(msg);
        }
    }
    return res;
}


void Task::finalizeMessage( Message& msg, int state )
{
    unsigned msgs;
    {
        MutexGuard mg(taskLock_);
        if ( messages_ > 0 ) --messages_;
        msgs = messages_;
    }
    smsc_log_debug(log_,"message %u failed, msgs left=%u", msg.getId(), msgs );
}


void Task::incrementScore( Message& msg )
{
    unsigned score, msgs;
    {
        MutexGuard mg(taskLock_);
        if ( messages_ > 0 ) --messages_;
        msgs = messages_;
        ++sent_;
        ++score_;
        wouldSend_ += 1000; // one event * (1s/1ms)
        score = normScore_ = score_ * 100 / priority_;
    }
    smsc_log_debug(log_,"message %u/%u sent, msgs left=%u, new score=%u", msg.getId(), msg.getRegionId(), msgs, score );
}


Task::RegionMap::iterator Task::findRegion( unsigned regionId )
{
    RegionMap::iterator i = regionMap_.lower_bound( regionId );
    if ( i == regionMap_.end() || i->first != regionId ) {
        unsigned oldreg = ( regionMapIter_ == regionMap_.end() ? 0 : regionMapIter_->first );
        i = regionMap_.insert( i, std::make_pair(regionId,RegionMapValue()) );
        i->second.enabled = true;
        regionMapIter_ = regionMap_.find(oldreg);
        if ( regionMapIter_ == regionMap_.end() ) { regionMapIter_ = i; }
    }
    return i;
}


// ===============================================

/// sender class
class Sender
{
private:
    typedef std::vector< Connector* > ConnectorList;

public:
    Sender();
    ~Sender();
    void addConnector( Connector* conn );
    unsigned connectorCount();
    int send( Message& msg );
    void dumpStatistics( std::string& s );

private:
    smsc::logger::Logger* log_;
    smsc::core::synchronization::Mutex lock_;
    ConnectorList connectors_;
};


Sender::Sender() :
log_(smsc::logger::Logger::getInstance("sender"))
{
    connectors_.reserve(200);
}


Sender::~Sender()
{
    std::for_each( connectors_.begin(), connectors_.end(), PtrDestroy() );
    connectors_.clear();
}


void Sender::addConnector( Connector* conn )
{
    unsigned id = conn->getId();
    MutexGuard mg(lock_);
    if ( id >= connectors_.size() ) {
        connectors_.resize(id+1);
    }
    if ( connectors_[id] ) {
        smsc_log_warn(log_,"connector %u is replaced",id);
        delete connectors_[id];
    }
    connectors_[id] = conn;
}


unsigned Sender::connectorCount()
{
    MutexGuard mg(lock_);
    return unsigned(connectors_.size());
}


int Sender::send( Message& msg )
{
    unsigned regid = msg.getRegionId();
    MutexGuard mg(lock_);
    if ( regid >= connectors_.size() ) return MessageState::NOCONN;
    Connector* conn = connectors_[regid];
    if ( ! conn ) {
        smsc_log_warn(log_,"connector %u is not found, trying to find the default one",regid);
        regid = 0;
        while ( ! conn && regid < connectors_.size() ) {
            conn = connectors_[regid];
        }
        if ( !conn ) return MessageState::NOCONN;
    }
    return conn->send(msg);
}


void Sender::dumpStatistics( std::string& s )
{
    MutexGuard mg(lock_);
    for ( ConnectorList::const_iterator i = connectors_.begin();
          i != connectors_.end(); ++i ) {
        s.append("\n  ");
        s.append( (*i)->toString());
    }
}


// ===============================================

class Processor : public util::WatchedThreadedTask
{
public:
    Processor( Sender& sender );
    virtual ~Processor();

    TaskGuard getTask( unsigned idx );
    void addTask( Task* task );
    void notify();

protected:
    virtual const char* taskName() { return "processor"; }
    virtual int doExecute();
    void dumpStatistics( unsigned deltaTime );

private:
    typedef std::list< Task* > TaskList;
    typedef std::list< TaskGuard > GuardedTaskList;
    typedef std::map< unsigned, GuardedTaskList::iterator > TaskMap;

private:
    smsc::logger::Logger*     log_;
    Sender*                   sender_; // not owned

    GuardedTaskList        allTasks_; // all tasks
    TaskMap                taskMap_;  // mapping from id

    // the task may be in only one of the following lists

    TaskList   inactiveTasks_;  // not active tasks

    // these lists are accessed only from doExecute
    TaskList   activeTasks_;    // active tasks, sorted by normScore
    TaskList   deadTasks_;      // scheduled to be destroyed
};


Processor::Processor( Sender& sender ) :
log_(smsc::logger::Logger::getInstance(taskName())),
sender_(&sender)
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

    util::msectime_type nextWakeTime = startTime;
    util::msectime_type lastStatTime = startTime;

    while ( ! stopping() ) {

        const util::msectime_type currentTime = util::currentTimeMillis();
        const unsigned deltaTime = unsigned(currentTime - startTime);
        smsc_log_debug(log_,"new pass at %u",deltaTime);

        if ( currentTime - lastStatTime > 10000 ) {
            dumpStatistics(deltaTime);
            lastStatTime = currentTime;
        }

        // 1. adding tasks that were requested to be activated
        {
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
                    const Task* firstTask = activeTasks_.empty() ? 0 : activeTasks_.front();
                    task->normalizeScore( deltaTime, firstTask ? firstTask->getNormalizedScore() : 0 );
                    TaskList::iterator j = std::lower_bound( activeTasks_.begin(),
                                                             activeTasks_.end(),
                                                             task, PtrLess() );
                    activeTasks_.insert(j,task);
                    i = inactiveTasks_.erase(i);
                    smsc_log_debug(log_,"moving task %s to active list",task->getName().c_str());
                    nextWakeTime = currentTime;
                    continue;
                }
                ++i;
            }
        }

        {
            int waitTime = int(nextWakeTime - currentTime);
            if ( waitTime > 10 ) {
                MutexGuard mg(releaseMon_);
                if ( stopping() ) break;
                releaseMon_.wait(waitTime);
                continue;
            }
        }

        // 2. next wake up time is initially set to 500 ms later
        unsigned wantToSleep = 500;
        
        {
            unsigned idx = 0;
            for ( TaskList::iterator i = activeTasks_.begin();
                  i != activeTasks_.end();
                  ++i ) {
                smsc_log_debug(log_,"%3u %s",idx,(*i)->toString().c_str());
                if ( ++idx > 10 ) break;
            }
        }

        // 3. the loop over all active tasks
        TaskList makeInactiveList;
        typedef std::set< unsigned > SuspendedRegions;
        SuspendedRegions suspendedRegions;
        for ( TaskList::iterator i = activeTasks_.begin();
              i != activeTasks_.end();
              ) {

            Task* task = *i;
            if ( task->isDestroyed() ) {
                deadTasks_.push_back(*i);
                i = activeTasks_.erase(i);
                continue;
            }

            if ( !task->isActive() || !task->hasMessages() ) {
                makeInactiveList.push_back(*i);
                i = activeTasks_.erase(i);
                continue;
            }
            
            // calculating the number of messages would be sent
            unsigned taskWantToSleep = task->wantToSleep( deltaTime );
            if ( taskWantToSleep > 0 ) {
                if ( taskWantToSleep < wantToSleep ) {
                    wantToSleep = taskWantToSleep;
                }
                ++i;
                continue;
            }

            // task has sent less messages that it should be,
            // so it wants to send a message
            task->resetSuspendedRegions();

            bool sent = false;
            do {
                Message msg;
                if ( ! task->getMessage(msg) ) {
                    // cannot get a message
                    break;
                }

                int reason = MessageState::UNKNOWN;
                try {
                    reason = sender_->send(msg);
                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"sending failed: %s", e.what());
                    reason = MessageState::UNKNOWN;
                }

                if ( reason == MessageState::OK ) {
                    // if message has been sent
                    task->incrementScore(msg);
                    // reorder it
                    i = activeTasks_.erase(i);
                    while ( i != activeTasks_.end() && !(*task < **i) ) {
                        ++i;
                    }
                    activeTasks_.insert(i,task);
                    wantToSleep = 0;
                    sent = true;
                    break;
                } else if ( reason == MessageState::NOCONN ) {
                    task->finalizeMessage(msg,reason);
                } else {
                    unsigned regionId = msg.getRegionId();
                    SuspendedRegions::iterator si = suspendedRegions.lower_bound(regionId);
                    if ( si == suspendedRegions.end() || *si != regionId ) {
                        smsc_log_debug(log_,"region %u is suspended",regionId);
                        suspendedRegions.insert(si,regionId);
                    }
                    task->suspendRegion(msg);
                }

            } while ( true );
            
            // if task has sent a message
            if ( sent ) break;

            if ( ! task->hasMessages() ) {
                makeInactiveList.push_back(task);
                i = activeTasks_.erase(i);
                continue;
            }

            ++i;

        } // loop over tasks

        nextWakeTime = currentTime + wantToSleep;

        MutexGuard mg(releaseMon_);

        // add all inactiveTasks to inactive and destroy all deadTasks
        inactiveTasks_.splice(inactiveTasks_.end(),makeInactiveList);

        // destroy all tasks waiting destruction
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
        sprintf(buf,"statistics, time=%u",deltaTime);
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


// ==============================================

class Controller : public util::WatchedThreadedTask
{
public:
    Controller( smsc::core::threads::ThreadPool& pool ) :
    log_(smsc::logger::Logger::getInstance(taskName())),
    pool_(pool)
    {}

    ~Controller();

    void createTask( unsigned priority, unsigned speed, unsigned msgs );
    void setTaskActive( unsigned index, bool active );
    void addMessages( unsigned index, unsigned msgs );
    void addConnector( unsigned bandwidth );

    class Tokenizer {
    public:
        Tokenizer() : whitespaces_(" \t\f\v\r\n") {}
        std::vector< std::string > tokenize( const std::string& line );
        std::string toString( const std::vector< std::string >& words );
        std::vector< unsigned > getInts( unsigned n, const std::vector< std::string >& words );
        
    private:
        std::string whitespaces_;
    };

protected:
    virtual int doExecute();
    virtual const char* taskName() { return "control"; }

private:
    smsc::logger::Logger* log_;
    smsc::core::threads::ThreadPool& pool_;
    std::auto_ptr< Processor > processor_;
    std::auto_ptr< Sender > sender_;
};


Controller::~Controller()
{
    processor_->stop();
    stop();
    checkDestroy();
}


void Controller::createTask( unsigned priority, unsigned speed, unsigned msgs )
{
    MutexGuard mg(releaseMon_);
    if ( processor_.get() ) {
        Task* task = new Task( priority, speed, sender_->connectorCount(), msgs );
        processor_->addTask( task );
    }
}


void Controller::setTaskActive( unsigned index, bool active )
{
    MutexGuard mg(releaseMon_);
    if ( processor_.get() ) {
        TaskGuard taskGuard = processor_->getTask(index);
        Task* task = taskGuard.get();
        if ( task ) {
            task->setActive( active );
            processor_->notify();
        }
    }
}


void Controller::addMessages( unsigned index, unsigned msgs )
{
    MutexGuard mg(releaseMon_);
    if ( processor_.get() ) {
        TaskGuard taskGuard = processor_->getTask(index);
        Task* task = taskGuard.get();
        if ( task ) {
            task->addMessages( sender_->connectorCount(), msgs );
            processor_->notify();
        }
    }
}


void Controller::addConnector( unsigned bandwidth )
{
    sender_->addConnector( new Connector(bandwidth ) );
}


int Controller::doExecute()
{
    smsc_log_info(log_,"started");

    sender_.reset( new Sender() );
    addConnector(10);

    // create a processor and start it
    processor_.reset( new Processor(*sender_.get()) );
    pool_.startTask(processor_.get(),false);

    while ( ! stopping() ) {

        {
            MutexGuard mg(releaseMon_);
            releaseMon_.wait(300);
        }

        /*
        const uint64_t r = util::Drndm::uniform(100);
        if ( r > 97 ) {
            int taskSpeed = int(util::Drndm::uniform(10));
            int taskPrio = int(util::Drndm::uniform(10));
            char taskname[100];
            sprintf(taskname,"%u",int(tasks_.size()));
            std::auto_ptr<Task> task(new Task(taskname,taskSpeed,taskPrio));
            processor_->addTask(task);
        }
         */
    }
    smsc_log_info(log_,"finishing");
    return 0;
}


std::vector< std::string > Controller::Tokenizer::tokenize( const std::string& line )
{
    std::vector< std::string > words;
    size_t pos = 0;
    while ( pos < line.size() ) {
        const size_t found = line.find_first_not_of(whitespaces_,pos);
        if ( found == std::string::npos ) break;
        pos = line.find_first_of(whitespaces_,found);
        if ( pos == std::string::npos ) { pos = line.size(); }
        words.push_back( line.substr(found,pos-found) );
    }
    return words;
}

std::string Controller::Tokenizer::toString( const std::vector<std::string>& words ) {
    size_t size = words.size();
    for ( std::vector<std::string>::const_iterator i = words.begin(); i != words.end(); ++i ) {
        size += i->size();
    }
    std::string line;
    line.reserve(size);
    for ( std::vector<std::string>::const_iterator i = words.begin(); i != words.end(); ++i ) {
        if ( !line.empty() ) line.push_back(' ');
        line.append(*i);
    }
    return line;
}


std::vector< unsigned > 
    Controller::Tokenizer::getInts( unsigned n,
                                    const std::vector< std::string >& words )
{
    if ( n+1 != words.size() ) throw ProtoException("cmd '%s': number of words mismatch: %u, required %u",
                                                    words[0].c_str(), unsigned(words.size()-1), n );
    std::vector< unsigned > res;
    res.reserve(n);
    for ( std::vector< std::string >::const_iterator i = words.begin()+1;
          i != words.end();
          ++i ) {
        char* endptr;
        unsigned long v = strtoul(i->c_str(),&endptr,0);
        if ( *endptr != '\0' ) {
            throw ProtoException("cmd '%s': not int: '%s'",words[0].c_str(),i->c_str());
        }
        res.push_back( unsigned(v) );
    }
    return res;
}


// ===================================


int main()
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* log = smsc::logger::Logger::getInstance("main");

    smsc::core::synchronization::EventMonitor sleepMon;

    smsc_log_info(log,"main started");
    smsc::core::threads::ThreadPool pool;
    {
        std::auto_ptr< Controller > controller( new Controller(pool) );
        smsc_log_info(log,"starting controller");
        pool.startTask( controller.get() );

        Controller::Tokenizer tok;
        while ( true ) {

            std::string line;
            std::getline(std::cin,line);
            if ( std::cin.eof() ) { break; }

            const std::vector< std::string > words(tok.tokenize(line));
            if ( words.empty() ) continue;
            smsc_log_info(log,"input line: %s", tok.toString(words).c_str());

            // processing
            try {
                std::vector< unsigned > ints;
                const std::string& action = words[0];
                if ( action == "conn" ) {
                    ints = tok.getInts(1,words);
                    controller->addConnector(ints[0]);
                } else if ( action == "create" ) {
                    ints = tok.getInts(3,words);
                    controller->createTask(ints[0],ints[1],ints[2]);
                } else if ( action == "disable" ) {
                    ints = tok.getInts(1,words);
                    controller->setTaskActive(ints[0],false);
                } else if ( action == "enable" ) {
                    ints = tok.getInts(1,words);
                    controller->setTaskActive(ints[0],true);
                } else if ( action == "addmsg" ) {
                    ints = tok.getInts(2,words);
                    controller->addMessages(ints[0],ints[1]);
                } else if ( action == "sleep" ) {
                    ints = tok.getInts(1,words);
                    MutexGuard mg(sleepMon);
                    sleepMon.wait(ints[0]);
                } else {
                    throw ProtoException("unknown command %s",action.c_str());
                }
            } catch ( std::exception& e ) {
                smsc_log_warn(log,"exc: %s", e.what());
            }
        } // while
    }
    smsc_log_info(log,"destroying the pool");
    pool.shutdown();
    return 0;
}
