#include <cassert>
#include <algorithm>
#include "Task.h"
#include "ProtoException.h"

namespace {
smsc::core::synchronization::Mutex taskIdMutex;
}

namespace scag2 {
namespace prototypes {
namespace infosme {

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
wouldSend_(0),
messages_(0),
users_(0),
prefetched_(false)
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
            packsize = 1 + speed_/2 + unsigned(random_.uniform(speed_,r));
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


void Task::suspendMessage( Message& msg )
{
    MutexGuard mg(taskLock_);
    doSuspendMessage( msg );
}


bool Task::prefetchMessage( time_t now, unsigned regionId )
{
    if ( isDestroyed_ || !isActive_ || !messages_ ) return false;
    MutexGuard mg(taskLock_);
    return doPrefetchMessage(now,regionId);
}


bool Task::getMessage( time_t now, unsigned regionId, Message& msg )
{
    MutexGuard mg(taskLock_);
    if ( !prefetched_ && !doPrefetchMessage(now,regionId) ) return false;
    prefetched_ = false;
    msg = prefetch_;
    return true;
}


void Task::finalizeMessage( Message& msg, int state )
{
    unsigned msgs;
    {
        MutexGuard mg(taskLock_);
        if ( messages_ > 0 ) --messages_;
        msgs = messages_;
        if ( state == MessageState::OK ) {
            ++sent_;
            wouldSend_ += 1000;
            unsigned regid = msg.getRegionId();
            if ( regid >= stats_.size() ) stats_.resize(regid+1);
            ++stats_[regid];
        }
    }
    smsc_log_debug(log_,"msg %u %s, msgs left=%u", msg.getId(),
                   state == MessageState::OK ? "sent" : "failed",
                   msgs );
}


/*
void Task::normalizeScore( const Task* lowestScoreTask, unsigned deltaTime )
{
    unsigned lowestScore = ( lowestScoreTask ? lowestScoreTask->normScore_ : normScore_ );
    MutexGuard mg(taskLock_);
    const static unsigned maxdiff = 100;
    if ( normScore_ < lowestScore ) { 
        if ( normScore_+maxdiff < lowestScore ) normScore_ = lowestScore-maxdiff;
    } else {
        const unsigned highestScore = normScore_ + maxdiff;
        if ( normScore_ > highestScore ) normScore_ = highestScore;
    }
    score_ = normScore_ * priority_ / 100;
    if ( deltaTime > 0 ) wouldSend_ = deltaTime * speed_;
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
        unsigned regid = msg.getRegionId();
        if ( regid >= stats_.size() ) stats_.resize(regid+1);
        ++stats_[regid];
    }
    smsc_log_debug(log_,"message %u/%u sent, msgs left=%u, new score=%u", msg.getId(), msg.getRegionId(), msgs, score );
}
 */


bool Task::setPrefetch( MessageList& list ) 
{
    if ( prefetched_ ) {
        doSuspendMessage(prefetch_);
    }
    prefetch_ = list.front();
    list.pop_front();
    return (prefetched_ = true);
}


void Task::doSuspendMessage( Message& msg )
{
    unsigned regionId = msg.getRegionId();
    {
        RegionMap::iterator i = regionMap_.lower_bound(regionId);
        if ( i == regionMap_.end() || i->first != regionId ) {
            i = regionMap_.insert(i,std::make_pair(regionId,MessageList()));
        }
        i->second.push_front(msg);
    }
    // smsc_log_debug(log_,"region %u suspended",regionId);
}


bool Task::doPrefetchMessage( time_t now, unsigned regionId )
{
    if ( isDestroyed_ || !isActive_ || !messages_ ) return false;

    if ( prefetched_ && prefetch_.getRegionId() == regionId ) return true;

    RegionMap::iterator iter = regionMap_.find(regionId);
    if ( iter != regionMap_.end() ) {
        MessageList& list = iter->second;
        if ( ! list.empty() ) {
            return setPrefetch(list);
        }
    }

    while ( ! messageList_.empty() ) {
        Message& msg = messageList_.front();
        if ( msg.startTime() > now ) {
            smsc_log_debug(log_,"all messages in task %u/%s for conn=%u are in future",
                           getId(), getName().c_str(), regionId );
            return false;
        }
        unsigned rid = msg.getRegionId();
        if ( rid == regionId ) {
            return setPrefetch(messageList_);
        }
        RegionMap::iterator i = regionMap_.lower_bound(rid);
        if ( i == regionMap_.end() || i->first != rid ) {
            i = regionMap_.insert(i,std::make_pair(rid,MessageList()));
        }
        i->second.push_back(msg);
        messageList_.pop_front();
    }
    smsc_log_debug(log_,"no messages in task %u/%s for conn=%u",
                   getId(), getName().c_str(), regionId );
    return false;
}


}
}
}
