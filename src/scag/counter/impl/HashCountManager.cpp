#include <map>
#include <algorithm>
#include "HashCountManager.h"
#include "util/Exception.hpp"
#include "core/buffers/XHash.hpp"

namespace {

template < class T > struct select2nd
{
    typename T::second_type& operator () ( T& pair ) const { return pair.second; }
};

}


namespace scag2 {
namespace counter {
namespace impl {

using namespace smsc::core::synchronization;

HashCountManager::HashCountManager() :
Manager(),
smsc::core::threads::Thread(),
log_(smsc::logger::Logger::getInstance("count.mgr")),
stopping_(true)
{
    smsc_log_debug(log_,"ctor");
}


HashCountManager::~HashCountManager()
{
    smsc_log_debug(log_,"dtor");
    {
        MutexGuard mg(disposeMon_);
        stopping_ = true;
        disposeMon_.notifyAll();
        disposeQueue_.clear();
    }
    this->WaitFor();
    MutexGuard mg(hashMutex_);
    char* key;
    Counter* ptr;
    for ( smsc::core::buffers::Hash<Counter*>::Iterator iter(&hash_); iter.Next(key,ptr); ) {
        delete ptr;
    }
    hash_.Empty();
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

CounterPtrAny HashCountManager::registerAnyCounter( Counter* ccc )
{
    if (!ccc) throw smsc::util::Exception("CountManager: null counter to register");
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
    hash_.Insert(ccc->getName().c_str(),cnt.release());
    return CounterPtrAny(ccc);
}


void HashCountManager::scheduleDisposal( counttime_type dt, Counter& c )
{
    MutexGuard mg(disposeMon_);
    if ( dt < nextTime_ ) {
        nextTime_ = dt;
        disposeMon_.notify();
    }
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
    typedef smsc::core::buffers::XHash< Counter*, ExpireMapType::iterator > ExpireHashType;

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
            if ( nextTime_ < nextTime ) nextTime = nextTime_;
            nextTime_ = now + timeToSleep;
            if ( ! disposeQueue_.empty() ) {
                workingQueue.swap(disposeQueue_);
            } else if ( nextTime > now ) {
                // we did not reached the time
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
