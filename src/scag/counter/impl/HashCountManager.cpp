#include <cassert>
#include <map>
#include <algorithm>
#include "HashCountManager.h"
#include "NotificationManager.h"
#include "util/Exception.hpp"
#include "core/buffers/IntHash64.hpp"
#include "core/buffers/XHash.hpp"
#include "scag/counter/TimeSliceGroup.h"

namespace {

template < class T > struct select2nd
{
    typename T::second_type& operator () ( T& pair ) const { return pair.second; }
};

struct PtrHashFunc
{
    template <class T> static inline unsigned int CalcHash( const T* key )
    {
        return unsigned(reinterpret_cast<uint64_t>
                        (reinterpret_cast<const void*>(key)));
    }
};

}


namespace scag2 {
namespace counter {
namespace impl {

using namespace smsc::core::synchronization;

/*
class HashCountManager::TimeSliceMgrImpl : public TimeSliceManager
{
private:
    static const usec_type idleSlice = 1*minSlice*usecFactor;
    
public:
    TimeSliceMgrImpl( usec_type             curTime,
                      smsc::logger::Logger* logger ) :
    log_(logger),
    lastTime_(curTime) {
        smsc_log_info(log_,"ctor");
    }
    virtual ~TimeSliceMgrImpl();

    usec_type lastTime() const { return lastTime_; }

    /// process all groups up to curtime
    usec_type process( usec_type curTime );

    virtual void addItem( TimeSliceItem& item, unsigned slices )
    {
        const usec_type slice = slices * minSlice * usecFactor;
        MutexGuard mg(lock_);
        TimeSliceGroup* grp;
        {
            TimeSliceGroup** ptr = groupHash_.GetPtr(slice);
            if ( ptr ) {
                grp = *ptr;
            } else {
                // no such group
                smsc_log_debug(log_,"group %u is created",slices);
                grp = createGroup(slice);
                groupHash_.Insert(slice,grp);
                groupMap_.insert(std::make_pair(lastTime_+slice,grp));
            }
        }
        grp->addItem( item );
    }

private:
    typedef std::multimap< usec_type, TimeSliceGroup* > GroupMap;
    typedef smsc::core::buffers::IntHash64< TimeSliceGroup* > GroupHash;
    
private:
    Mutex                     lock_;
    smsc::logger::Logger*     log_;
    GroupHash                 groupHash_;
    GroupMap                  groupMap_; // owned
    usec_type                 lastTime_;
};


usec_type HashCountManager::TimeSliceMgrImpl::process( usec_type curTime )
{
    usec_type retval;
    MutexGuard mg(lock_);
    if ( groupMap_.empty() ) {
        retval = curTime + idleSlice;
    } else {
        while ( true ) {
            GroupMap::iterator i = groupMap_.begin();
            if ( i->first > curTime ) {
                retval = i->first;
                break;
            }
            TimeSliceGroup* grp = i->second;
            const usec_type nextTime = grp->advanceTime(curTime);
            groupMap_.erase(i);
            groupMap_.insert(std::make_pair(nextTime,grp));
        }
    }
    lastTime_ = curTime;
    return retval;
}


HashCountManager::TimeSliceMgrImpl::~TimeSliceMgrImpl()
{
    smsc_log_info(log_,"dtor");
    groupHash_.Empty();
    for ( GroupMap::iterator i = groupMap_.begin(); i != groupMap_.end(); ++i ) {
        delete i->second;
    }
}

// ======================================================================
*/

HashCountManager::HashCountManager( unsigned notifySlices ) :
Manager(),
smsc::core::threads::Thread(),
log_(smsc::logger::Logger::getInstance("count.mgr")),
notificationManager_(new NotificationManager),
timeSliceManager_(new TimeSliceManagerImpl(smsc::logger::Logger::getInstance("count.tmgr"),
                                           notificationManager_,notifySlices)),
stopping_(true)
{
    smsc_log_debug(log_,"ctor");
}


HashCountManager::~HashCountManager()
{
    smsc_log_debug(log_,"dtor");
    stop();
    /*
    {
        MutexGuard mg(disposeMon_);
        stopping_ = true;
        disposeMon_.notifyAll();
        disposeQueue_.clear();
    }
     */
    this->WaitFor();
    bool isempty;
    unsigned npass = 0;
    do {
        isempty = true;
        MutexGuard mg(hashMutex_);
        char* key;
        Counter** ptr;
        for ( smsc::core::buffers::Hash<Counter*>::Iterator iter(&hash_); iter.Next(key,ptr); ) {
            if ( *ptr ) {
                if ( checkDisposal(*ptr,counttime_max) ) {
                    delete *ptr;
                    *ptr = 0;
                } else {
                    smsc_log_debug(log_,"counter '%s' is still locked",(*ptr)->getName().c_str());
                    isempty = false;
                }
            }
        }
    } while ( !isempty && ++npass < 20 );
    if ( !isempty ) {
        smsc_log_warn(log_,"manager has non-free counters");
    }
    hash_.Empty();
    delete timeSliceManager_; timeSliceManager_ = 0;
    delete notificationManager_; notificationManager_ = 0;
}


void HashCountManager::start()
{
    if ( !stopping_ ) return;
    MutexGuard mg(disposeMon_);
    if ( !stopping_ ) return;
    timeSliceManager_->start();
    stopping_ = false;
    this->Start();
}


void HashCountManager::stop()
{
    if ( stopping_ ) return;
    {
        MutexGuard mg(disposeMon_);
        stopping_ = true;
        timeSliceManager_->stop();
        disposeMon_.notifyAll();
        disposeQueue_.clear();
    }
}


CounterPtrAny HashCountManager::getAnyCounter( const char* name )
{
    {
        MutexGuard mg(hashMutex_);
        Counter** ptr = hash_.GetPtr(name);
        if (ptr) return CounterPtrAny(*ptr);
    }
    return CounterPtrAny();
}

CounterPtrAny HashCountManager::registerAnyCounter( Counter* ccc, bool& wasRegistered )
{
    if (!ccc) throw smsc::util::Exception("CountManager: null counter to register");
    wasRegistered = false;
    std::auto_ptr<Counter> cnt(ccc);
    MutexGuard mg(hashMutex_);
    Counter** ptr = hash_.GetPtr(ccc->getName().c_str());
    if ( ptr ) {
        if ( ccc->getType() != (*ptr)->getType() ) {
            throw smsc::util::Exception("CountManager: counter '%s' already registered with different type %d != %d",
                                        ccc->getName().c_str(), (*ptr)->getType(), ccc->getType());
        }
        return CounterPtrAny(*ptr);
    }
    wasRegistered = true;
    hash_.Insert(ccc->getName().c_str(),cnt.release());
    return CounterPtrAny(ccc);
}


void HashCountManager::scheduleDisposal( counttime_type dt, Counter& c )
{
    MutexGuard mg(disposeMon_);
    // if ( dt < wakeTime_ ) {
    // nextTime_ = dt;
    // disposeMon_.notify();
    // }
    if ( disposeQueue_.size() > 200 ) {
        disposeMon_.notify();
    }
    if ( disposeQueue_.size() < 20000 ) {
        disposeQueue_.push_back(&c);
    }
}


counttime_type HashCountManager::getWakeTime() const
{
    return wakeTime_;
}


int HashCountManager::Execute()
{
    typedef std::multimap< counttime_type, Counter* > ExpireMapType;
    typedef smsc::core::buffers::XHash< Counter*, ExpireMapType::iterator, PtrHashFunc > ExpireHashType;

    smsc_log_info(log_,"started");

    DisposeQueueType workingQueue, expireList;
    ExpireMapType expireMap;
    ExpireHashType expireHash;
    counttime_type nextTime = getCurrentTime();
    const counttime_type timeToSleep = 10;

    while ( ! stopping_ ) {

        const counttime_type now = getCurrentTime();
        smsc_log_debug(log_,"new pass at %u",unsigned(now));
        {
            MutexGuard mg(disposeMon_);
            if ( stopping_ ) break;
            // if ( nextTime_ < nextTime ) nextTime = nextTime_;
            // nextTime_ = now + timeToSleep;
            if ( ! disposeQueue_.empty() ) {
                workingQueue.swap(disposeQueue_);
            } else if ( nextTime > now ) {
                // we did not reached the time
                // smsc_log_debug(log_,"next pass will be at %u",unsigned(nextTime));
                int waitTime = int(nextTime - now) * 1000;
                if ( waitTime < 100 ) waitTime = 100;
                wakeTime_ = nextTime;
                disposeMon_.wait( waitTime );
                continue;
            }
            wakeTime_ = now;
        }
        
        // processing working queue
        if ( ! workingQueue.empty() ) {
            smsc_log_debug(log_,"processing input queue sz=%u", unsigned(workingQueue.size()));

            // removing duplicates
            std::sort( workingQueue.begin(), workingQueue.end() );
            workingQueue.erase(std::unique(workingQueue.begin(),workingQueue.end()),
                               workingQueue.end());

            // removing elements from the queue that are not expired,
            // adding those with high dt to the expiration structures.
            for ( std::vector< Counter* >::iterator i = workingQueue.begin();
                  i != workingQueue.end();
                  ++i ) {
                const counttime_type dt = (*i)->getDisposeTime();
                if ( dt == counttime_locked ) {
                    // locked
                    *i = 0; continue;
                } else if ( dt > now ) {
                    // not expired yet
                    ExpireMapType::iterator* ptr = expireHash.GetPtr(*i);
                    if ( !ptr ) {
                        // not inserted
                        expireHash.Insert(*i,expireMap.insert(std::make_pair(dt,*i)));
                    } else if ( dt == (*ptr)->first ) {
                        // expiration time is not changed, do nothing
                    } else {
                        // expiration time has changed
                        expireMap.erase(*ptr);
                        *ptr = expireMap.insert(std::make_pair(dt,*i));
                    }
                    *i = 0; continue;
                } else {
                    ExpireMapType::iterator* ptr = expireHash.GetPtr(*i);
                    if ( ptr ) {
                        expireMap.erase(*ptr);
                        expireHash.Delete(*i);
                    }
                }
            }

            // removing all zeros
            workingQueue.erase( std::remove( workingQueue.begin(),
                                             workingQueue.end(),
                                             static_cast<Counter*>(0) ),
                                workingQueue.end() );
        }

        {
            // taking all elements from expireMap that are expired
            ExpireMapType::iterator iexp = expireMap.upper_bound(now);
            if ( iexp != expireMap.begin() ) {
                // there are elements that are supposed to be expired
                // expiredList.reserve( std::distance(expireMap.begin(),iexp) );
                expireList.clear();
                std::transform( expireMap.begin(), iexp,
                                std::back_inserter(expireList),
                                ::select2nd< ExpireMapType::value_type >() );
                smsc_log_debug(log_,"expireMap has %u supposed-to-expire elements",unsigned(expireList.size()));
                expireMap.erase( expireMap.begin(), iexp );
                workingQueue.reserve( workingQueue.size() + expireList.size() );
                for ( DisposeQueueType::iterator i = expireList.begin();
                      i != expireList.end(); ++i ) {
                    ExpireMapType::iterator* ptr = expireHash.GetPtr(*i);
                    if ( !ptr ) {
                        smsc_log_fatal(log_,"logic error: ptr==0, for counter=%s", (*i)->getName().c_str() );
                        ::abort();
                    }
                    const counttime_type dt = (*i)->getDisposeTime();
                    if ( dt == counttime_locked || dt > now ) {
                        // the counter is grabbed or changed its expiration time
                        *ptr = expireMap.insert( std::make_pair(dt,*i) );
                    } else {
                        expireHash.Delete(*i);
                        workingQueue.push_back(*i);
                    }
                }
            }
        }

        // so, now we have formed an expiration workingQueue list.
        // it is time to remove elements from main hash.
        if ( ! workingQueue.empty() ) {
            MutexGuard mg(hashMutex_);
            for ( DisposeQueueType::iterator i = workingQueue.begin();
                  i != workingQueue.end();
                  ++i ) {
                if ( checkDisposal(*i,now) ) {
                    hash_.Delete( (*i)->getName().c_str() );
                    delete *i;
                }
            }
            workingQueue.clear();
        }
        
        if ( expireMap.empty() ) {
            nextTime = now + timeToSleep;
        } else {
            nextTime = expireMap.begin()->first;
        }
    }
    smsc_log_info(log_,"stopped");
    return 0;
}

}
}
}
