#include <cassert>
#include <map>
#include <algorithm>
#include "HashCountManager.h"
#include "NotificationManager.h"
#include "util/Exception.hpp"
#include "core/buffers/IntHash64.hpp"
#include "core/buffers/XHash.hpp"
#include "scag/counter/TimeSliceGroup.h"
#include "scag/counter/TemplateManager.h"
#include "ConfigReader.h"

namespace {

const char* confFilename = "conf/counters.xml";

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

HashCountManager::HashCountManager( TemplateManager* tmplmgr, unsigned notifySlices ) :
Manager(),
smsc::core::threads::Thread(),
log_(smsc::logger::Logger::getInstance("cnt.mgr")),
notificationManager_(new NotificationManager),
timeSliceManager_(new TimeSliceManagerImpl(smsc::logger::Logger::getInstance("cnt.timgr"),
                                           notificationManager_,notifySlices)),
templateManager_(tmplmgr),
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

    smsc_log_debug(log_,"destroying notification manager, it may hold some cntrs");
    delete notificationManager_; notificationManager_ = 0;

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
                    releaseAndDestroy(*ptr);
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
    smsc_log_debug(log_,"destroying template manager");
    delete templateManager_; templateManager_ = 0;
    smsc_log_debug(log_,"destroying time slice manager");
    delete timeSliceManager_; timeSliceManager_ = 0;
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


void HashCountManager::reloadObserver( const char* id )
{
    if (!templateManager_) throw smsc::util::Exception("template manager is not attached");
    if (!id) throw smsc::util::Exception("action table id is not specified");
    ConfigReader reader;
    try {
        reader.readConfig( ::confFilename, true );
    } catch ( std::exception& e ) {
        throw smsc::util::Exception( "cannot read %s: %s", ::confFilename, e.what() );
    }
    reader.replaceObserver( *templateManager_, id );
}


void HashCountManager::reloadTemplate( const char* id )
{
    if (!templateManager_) throw smsc::util::Exception("template manager is not attached");
    if (!id) throw smsc::util::Exception("template id is not specified");
    ConfigReader reader;
    try {
        reader.readConfig( ::confFilename, true );
    } catch ( std::exception& e ) {
        throw smsc::util::Exception( "cannot read %s: %s", ::confFilename, e.what() );
    }
    reader.replaceTemplate( *templateManager_, id );
}


void HashCountManager::loadConfigFile()
{
    ConfigReader reader;
    if ( !reader.readConfig(confFilename) ) {
        smsc_log_warn(log_,"cannot read counters config file '%s', using default",confFilename);
    } else if (templateManager_) {
        reader.reload(*templateManager_);
    }
}


MsagCounterTableElement* HashCountManager::updateSnmpCounterList( MsagCounterTableElement* list )
{
    smsc_log_debug(log_,"request for snmp counter table update");
    MutexGuard mg(hashMutex_);
    if ( snmpCounterList_.empty() ) return list;
    smsc_log_debug(log_,"request for snmp counter table %p update, sz=%u",
                   list, unsigned(snmpCounterList_.size()));
    MsagCounterTableElement *head = list;
    MsagCounterTableElement *prev = list;
    for ( std::map< std::string, Counter* >::const_iterator i = snmpCounterList_.begin();
          i != snmpCounterList_.end();
          ++i ) {

        if ( ! i->second ) continue; // skipping destroyed

        const std::string& cname = i->second->getName();

        if ( !list ) {
            list = new MsagCounterTableElement;
            memset(list,0,sizeof(MsagCounterTableElement));
            if ( prev ) { prev->next = list; }
            else { head = list; }
            smsc_log_debug(log_,"adding %s, val=%llu",cname.c_str(),i->second->getValue());
        }

        list->namelen = int(std::min(cname.size(),sizeof(list->name)-1));
        memcpy(list->name, cname.c_str(), list->namelen);
        list->enabled = true;
        list->value = i->second->getValue();

        prev = list;
        list = prev->next;
    }
    // destroying the tail of the list
    if (prev) { prev->next = 0; }
    if ( head == list ) { head = 0; }
    while ( list ) {
        prev = list->next;
        delete list;
        list = prev;
    }
    return head;
}


CounterPtrAny HashCountManager::getAnyCounter( const char* name )
{
    if (!name) return CounterPtrAny();
    smsc_log_debug(log_,"asking to fetch counter '%s'",name);
    {
        MutexGuard mg(hashMutex_);
        Counter** ptr = hash_.GetPtr(name);
        if (ptr) return CounterPtrAny(*ptr);
    }
    return CounterPtrAny();
}

CounterPtrAny HashCountManager::doRegisterAnyCounter( Counter* ccc, bool& wasRegistered )
{
    if (!ccc) throw smsc::util::Exception("CountManager: null counter to register");
    smsc_log_debug(log_,"asking to register a counter %s '%s'",
                   ccc->getTypeName(), ccc->getName().c_str());
    wasRegistered = false;
    std::auto_ptr<Counter> cnt(ccc);
    MutexGuard mg(hashMutex_);
    Counter** ptr = hash_.GetPtr(ccc->getName().c_str());
    if ( ptr ) {
        if ( ccc->getType() != (*ptr)->getType() ) {
            throw smsc::util::Exception("CountManager: counter '%s' already registered with different type %s != %s",
                                        ccc->getName().c_str(), (*ptr)->getTypeName(), ccc->getTypeName());
        }
        return CounterPtrAny(*ptr);
    }
    wasRegistered = true;
    static const char* syspfx = "sys.";
    if ( 0 == strncmp(syspfx,ccc->getName().c_str(),strlen(syspfx)) ) {
        snmpCounterList_.insert(std::make_pair(ccc->getName(),cnt.get()));
    }
    hash_.Insert(ccc->getName().c_str(),cnt.release());
    return CounterPtrAny(ccc);
}


void HashCountManager::scheduleDisposal( Counter& c )
{
    // const counttime_type dt = c.getDisposeTime();
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
                    // it is ready to be destroyed
                    hash_.Delete( (*i)->getName().c_str() );
                    releaseAndDestroy(*i);
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


void HashCountManager::releaseAndDestroy( Counter* cnt )
{
    if ( !cnt ) return;
    std::map< std::string, Counter* >::iterator i =
        snmpCounterList_.find( cnt->getName() );
    if ( i == snmpCounterList_.end() ) return;
    snmpCounterList_.erase(i);
    destroy(cnt);
}


}
}
}
