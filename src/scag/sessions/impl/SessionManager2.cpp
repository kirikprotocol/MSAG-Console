#include "SessionManager2.h"
#include "scag/util/UnlockMutexGuard.h"
#include "scag/re/base/RuleEngine2.h"
#include "scag/lcm/base/LongCallManager2.h"

namespace {

using namespace scag2::sessions;

time_t maxtime_() {
    time_t zero = time_t(0);

    unsigned long long x(-1);
    for ( size_t i = 0; i < 100; ++i ) {
        time_t res = time_t(x);
        if ( res > zero ) return res;
        x = x >> 1;
    }
    ::abort();
    return zero;
}

const time_t maxtime = maxtime_();

}


namespace scag2 {

using namespace config;

namespace sessions {

// const time_t SessionManagerImpl::DEFAULT_EXPIRE_INTERVAL = 60;

using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
// using namespace scag::util::singleton;
using namespace scag::exceptions;
using namespace smsc::core::buffers;
// using namespace scag::util::sms;
using namespace lcm;
using namespace re;

using scag2::config::SessionManagerConfig;
using scag::util::UnlockMutexGuard;

using smsc::logger::Logger;

struct SessionManagerImpl::lessAccessTime 
{
    bool operator() ( ExpireList::const_iterator a,
                      ExpireList::const_iterator b ) const
    {
        return a->access < b->access;
        // return ( a->second < b->second ) || ( a->second == b->second && a->first < b->first );
    }
};


SessionManagerImpl::SessionManagerImpl() :
ConfigListener(SESSIONMAN_CFG),
nodeNumber_(-1),
flushLimitSize_(100000000),
flushLimitTime_(100000000),
activeSessions_(0),
cmdqueue_(0),
inputList_(0),
log_(0),
started_(false),
oldestsession_(time(0))
{
    log_ = smsc::logger::Logger::getInstance("sess.mgr");
}


SessionManagerImpl::~SessionManagerImpl()
{
    Stop();
    smsc_log_info(log_,"SessionManager is released");
}


void SessionManagerImpl::init( const scag2::config::SessionManagerConfig& cfg,
                               unsigned nodeNumber,
                               SCAGCommandQueue& cmdqueue ) // possible throws exceptions
{
    nodeNumber_ = nodeNumber;
    flushLimitSize_ = cfg.flushlimitsize;
    flushLimitTime_ = cfg.flushlimittime;
    if ( ! cfg.diskio ) {
        flushLimitSize_ = 100000000;
        flushLimitTime_ = 100000000;
    }
    cmdqueue_ = &cmdqueue;
    config_ = cfg;
    // expireSchedule = time(NULL) + DEFAULT_EXPIRE_INTERVAL;
    if ( cfg.expireInterval >= 1000 ) {
        Session::setDefaultLiveTime( cfg.expireInterval / 1000 );
    }
    if ( cfg.ussdExpireInterval >= 1000 ) {
        Session::setUssdLiveTime( cfg.ussdExpireInterval / 1000 );
    }
    if ( cfg.ussdReplaceTimeout >= 1000 ) {
        Session::setUssdReplaceTimeout( cfg.ussdReplaceTimeout / 1000 );
    }

    // if (!log_) log_ = Logger::getInstance("sess.mgr");

    if ( ! allocator_.get() )
        allocator_.reset( new SessionAllocator() );

    if ( ! store_.get() )
        store_.reset( new CompositeSessionStore( log_,
                                                 *this,
                                                 *this,
                                                 allocator_.get() ) );

    store_->init( nodeNumber_,
                  *cmdqueue_,
                  cfg.dirs,
                  cfg.indexgrowth,
                  cfg.pagesize,
                  cfg.prealloc,
                  cfg.initUploadCount,
                  cfg.initUploadInterval,
                  cfg.diskio );

    smsc_log_debug(log_,"SessionManager::initialized");
}


ActiveSession SessionManagerImpl::fetchSession( const SessionKey&           key,
                                                std::auto_ptr<SCAGCommand>& cmd,
                                                bool                        create )
{
    if ( !isStarted() || !store_.get() || !cmd.get() ) return ActiveSession();
    return store_->fetchSession( key, cmd, create );
}


void SessionManagerImpl::getSessionsCount(uint32_t& sessionsCount,
                                          uint32_t& sessionsLoadedCount,
                                          uint32_t& sessionsLockedCount)
{
    if ( store_.get() ) store_->getSessionsCount( sessionsCount, 
                                                  sessionsLoadedCount,
                                                  sessionsLockedCount );
}


void SessionManagerImpl::configChanged()
{
    // MutexGuard mt(inUseMonitor);
    Stop();
    assert( cmdqueue_ );
    init( ConfigManager::Instance().getSessionManConfig(),
          nodeNumber_,
          *cmdqueue_ );
    Start();
}


bool SessionManagerImpl::isStarted()
{
    //MutexGuard guard(stopLock);
    return started_;
}


void SessionManagerImpl::Start()
{
    MutexGuard guard(stopLock_);
    if (!started_)
    {
        started_ = true;
        Thread::Start();
        if ( store_.get() ) store_->start();
    }
}


void SessionManagerImpl::Stop()
{
    MutexGuard guard(stopLock_);

    if (started_)
    {
        MutexGuard mg(expireMonitor_);

        // waiting for all sessions to be unlocked
        if ( store_.get() ) {
            store_->stop();

            smsc_log_debug(log_,"store is signalled to stop");

            unsigned sessionLockedCount = 0;
            for ( int passes = 0; ; ) {

                unsigned sessionCount, sessionLoadedCount, newSessionLockedCount;
                store_->getSessionsCount( sessionCount, sessionLoadedCount,
                                          newSessionLockedCount );
                smsc_log_debug(log_,"session count pass=%u tot/ldd/lck=%u/%u/%u",
                               passes,
                               sessionCount,sessionLoadedCount,newSessionLockedCount);
                if ( newSessionLockedCount == 0 ) break;
                if ( sessionLockedCount == newSessionLockedCount )
                    ++passes;
                sessionLockedCount = newSessionLockedCount;

                expireMonitor_.wait(100);
                if ( passes >= 10 ) {
                    smsc_log_error( log_, "logic error in stop: long lived session(s) or dead lock? Final number of sessions=%u", sessionLockedCount );
                    ::abort();
                }
            }

            // all sessions have been returned to store
            // notify expire thread
            smsc_log_debug( log_, "all sessions are in expire pool" );
            started_ = false;
            expireMonitor_.notify();
        }
        started_ = false;
        delete inputList_;
        inputList_ = 0;

    } // if started
    this->WaitFor();
    smsc_log_info(log_,"SessionManager::stop");
}


int SessionManagerImpl::Execute()
{
    smsc_log_info( log_, "SessionManager:start executing" );

    int curtmo = 0;
    const int deftmo = 1; // in seconds
    // MutexGuard mg(expireMonitor_);
    bool alldone = true;
    bool started = true;
    typedef std::vector< std::pair<SessionKey,time_t> > SessVec;
    SessVec curset;
    while( true ) {

        // smsc_log_debug(log_,"sessman rolling");
        ExpireList* list = 0;
        {
            MutexGuard mg(expireMonitor_);
            if ( curtmo > 0 ) {
                expireMonitor_.wait(curtmo*1000);
                curtmo = 0;
            }

            if ( inputList_ && ! inputList_->empty() ) {
                list = inputList_;
                inputList_ = 0;
            }
            started = isStarted();
        }

        // process input
        unsigned increment = 0;
        if ( list ) {

            // expireList_ is a std::list< Expire >
            // expireMap_  is a std::multimap< time_t, ExpireList::iterator >
            // expireHash_ is XHash< KeyPtr, ExpireList::iterator, KeyPtr >
            unsigned count = 0;
            time_t now = time(0);
            for ( ; ! list->empty(); list->pop_front() ) {

                ++increment;
                Expire& e = list->front();
                ExpireList::iterator i;
                ExpireList::iterator* ptr = expireHash_.GetPtr(KeyPtr(e));
                if ( e.access != maxtime ) {
                    if ( e.access < oldestsession_ ) oldestsession_ = e.access;
                    ++activeSessions_;
                    alldone = false;
                }
                if ( ptr ) {
                    // found
                    i = *ptr;
                    ExpireMap::iterator ei = findExpire(i->expire,e.key);
                    if ( i->access != maxtime ) --activeSessions_;
                    if ( e.access > i->access || i->access == maxtime ) {
                        i->access = e.access;
                    }
                    if ( i->expire == e.expire ) continue;
                    i->expire = e.expire;
                    expireMap_.erase( ei );
                } else {
                    // not found
                    expireList_.push_front( e );
                    i = expireList_.begin();
                    expireHash_.Insert( KeyPtr(*i), i );
                }
                if ( e.expire - now > 1000000 ) {
                    smsc_log_warn( log_, "key=%s has too great expiration time: %u",
                                   e.key.toString().c_str(), unsigned(e.expire-now) );
                }
                expireMap_.insert( std::make_pair(i->expire,i) );

                if ( ++count % 10 == 0 ) Thread::Yield();
            }
            delete list;
        }

        time_t now = time(0);
        if ( ! curset.empty() ) {
            const time_t expireTimeRet = now+5;
            smsc_log_debug(log_,"%u sessions has returned from previous expiration",unsigned(curset.size()));
            ExpireMap::iterator mi = expireMap_.lower_bound(expireTimeRet);
            for ( SessVec::const_iterator i = curset.begin();
                  i != curset.end();
                  ++i ) {
                ExpireList::iterator* ptr = expireHash_.GetPtr(KeyPtr(&(i->first)));
                if ( !ptr ) {
                    // if not found
                    smsc_log_debug(log_,"adding returned key=%s",i->first.toString().c_str());
                    if (i->second != maxtime) {
                        if ( i->second < oldestsession_ ) oldestsession_ = i->second;
                        ++activeSessions_;
                        alldone = false;
                    }
                    expireList_.push_front( Expire(expireTimeRet,i->second,i->first) );
                    ExpireList::iterator it = expireList_.begin();
                    expireHash_.Insert( KeyPtr(*it), it );
                    expireMap_.insert(mi,std::make_pair(expireTimeRet,it));
                }
            }
            curset.clear();
        }

        curtmo = deftmo;
        int next = -666;
        if ( ! expireMap_.empty() ) {
            next = int(expireMap_.begin()->first - now);
            if ( curtmo > next ) curtmo = next;
        }
        // oldwait is a number of sec to wait for oldest session flush time
        // NOTE: we also add 5 seconds to bunch old session processing.
        int oldwait = int(oldestsession_+flushLimitTime_+5 - now);
        if ( curtmo > oldwait ) curtmo = oldwait;
        if ( curtmo > int(flushLimitTime_) ) curtmo = int(flushLimitTime_);
        if ( activeSessions_ > flushLimitSize_ ) curtmo = 0;
        if ( ! started && activeSessions_ == 0 ) {
            if ( alldone ) break;
            curtmo = 1;
        }

        if ( curtmo > 0 ) continue;

        // smsc_log_debug( log_, "act/tot=%u/%u", activeSessions_, expireMap_.size() );

        if ( started ) {
            ExpireMap::iterator i = expireMap_.upper_bound(now);
            /*
            ExpireMap::iterator i;
            if ( ! started ) {
                curset.reserve( expireMap_.size() );
                i = expireMap_.end();
                smsc_log_debug(log_, "taking the whole expire set, sz=%u", expireMap_.size() );
            } else {
                i = inext;
            }
             */
            // smsc_log_debug( log_, "waked for expiration, cnt=%u/%u", 
            // expireMap_.size(), expireHash_.Count() );
            for ( ExpireMap::iterator j = expireMap_.begin(); j != i ; ++j ) {
                // smsc_log_debug( log_, "prepare key=%s for expiration", j->second.toString().c_str() );
                if ( j->second->access != maxtime ) {
                    // is living session
                    --activeSessions_;
                }
                curset.push_back( std::make_pair(j->second->key,j->second->access) );
                expireHash_.Delete( KeyPtr(*j->second) );
                expireList_.erase( j->second );
            }
            expireMap_.erase( expireMap_.begin(), i );
        }

        if ( ! expireMap_.empty() )
            next = int(expireMap_.begin()->first - now);

        SessVec flushset;
        if ( activeSessions_ > 0 && ! started ) {

            // all active sessions should be dumped to disk
            for ( ExpireList::iterator i = expireList_.begin();
                  i != expireList_.end();
                  ++i ) {
                if ( i->access == maxtime ) continue;
                flushset.push_back( std::make_pair(i->key,i->access) );
                --activeSessions_;
                i->access = maxtime;
            }
            oldestsession_ = now;

        } else if ( activeSessions_ > flushLimitSize_ || oldwait <= 0 ) {

            // too many living sessions
            typedef std::vector< ExpireList::iterator > OldAccess_type;
            OldAccess_type oldaccess;
            // oldaccess.reserve( expireMap_.size()/2 );
            for ( ExpireList::iterator i = expireList_.begin();
                  i != expireList_.end();
                  ++i ) {

                if ( i->access == maxtime ) continue;
                if ( i->expire < i->access + 5 ) continue;  // too close expiration time
                oldaccess.push_back( i );

            }
            std::sort( oldaccess.begin(),
                       oldaccess.end(),
                       lessAccessTime() );
            const size_t finalsize = size_t(flushLimitSize_ * 0.9);
            size_t extracount = oldaccess.size() > finalsize ? oldaccess.size() - finalsize : 0;
            if ( extracount > 0 ) 
                flushset.reserve( extracount );
            else
                flushset.reserve( size_t(oldaccess.size() * 0.1) );
            const time_t flushtime = time_t(now - flushLimitTime_);
            oldestsession_ = maxtime;
            for ( OldAccess_type::iterator i = oldaccess.begin(); i != oldaccess.end(); ++i ) {

                // smsc_log_debug( log_, "oldaccess: %s %i",
                // (*i)->first.toString().c_str(), int((*i)->second - now) );
                // if ( (*i)->second == maxtime ) continue;
                    
                if ( extracount > 0 ) {
                    --extracount;
                } else if ( (*i)->access <= flushtime ) {
                } else {
                    // a new oldest session
                    oldestsession_ = (*i)->access;
                    break;
                }
                    
                flushset.push_back( std::make_pair( (*i)->key, (*i)->access ) );
                --activeSessions_;
                (*i)->access = maxtime;

            }
            if ( oldestsession_ == maxtime ) oldestsession_ = now;
            
        }

        if ( !curset.empty() || !flushset.empty() ) {

            Thread::Yield();

            unsigned storeTotal;
            unsigned storeLoaded;
            unsigned storeLocked;
            store_->getSessionsCount(storeTotal,storeLoaded,storeLocked);

            // const unsigned tot = unsigned(expireMap_.size());
            // const unsigned act = activeSessions_;
            smsc_log_info( log_, "expire/flush=%u/%u act/tot=%u/%u tot/load/lock=%u/%u/%u inc=%u tmAE=%d/%d limTS=%u/%u run=%u",
                           unsigned(curset.size()),
                           unsigned(flushset.size()),
                           activeSessions_,
                           unsigned(expireMap_.size()),
                           storeTotal,
                           storeLoaded,
                           storeLocked,
                           increment,
                           oldwait,
                           next,
                           flushLimitTime_,
                           flushLimitSize_,
                           started ? 1 : 0 );
            alldone = store_->expireSessions( curset, flushset );

            /*
             if ( log_->isDebugEnabled() ) {
             uint32_t sc;
             uint32_t slc;
             getSessionsCount( sc, slc );
             smsc_log_debug( log_, "after expire/flush sessions tot/lck=%u/%u",
             sc, slc );
             }
             */

        }

    } // while expireMap_ is not empty

    smsc_log_info( log_, "SessionManager::stop executing" );
    return 0;
}


void SessionManagerImpl::continueExecution( LongCallContextBase* lcmCtx, bool dropped )
{
    /// long call in session_destroy
    Session* session = reinterpret_cast<Session*>( lcmCtx->stateMachineContext );
    lcmCtx->continueExec = ! dropped;

    // special finalization command
    assert( session && session->currentCommand() == 1 );

    smsc_log_debug( log_, "continueExec(session=%p/%s,drop=%d)",
                    session, session->sessionKey().toString().c_str(), dropped ? 1 : 0 );
    // scag_plog_debug(pl, log_);
    // session->print(pl);

    if ( dropped ) {
        // finalize immediately
        store_->sessionFinalized( *session );
    } else {
        scheduleExpire( session->expirationTime(), session->lastAccessTime(), session->sessionKey() );
    }
}


void SessionManagerImpl::scheduleExpire( time_t expirationTime,
                                         time_t lastaccessTime,
                                         const SessionKey& key )
{
    // unsigned mapsz;
    // unsigned actsz;
    // time_t now = time(0);
    // int prevtime;

    time_t now = time(0);
    {
        MutexGuard mg(expireMonitor_);
        if ( ! inputList_ ) inputList_ = new ExpireList;
        inputList_->push_back( Expire(expirationTime, lastaccessTime, key ) );
    
        // if ( lastaccessTime < oldestsession_ ) oldestsession_ = lastaccessTime;

    /*
        time_t* ptr = expireHash_.GetPtr(key);
        mapsz = unsigned(expireMap_.size());
        actsz = activeSessions_;

        if ( actsz == 0 ) oldestsession_ = lastaccessTime;

        if ( ptr ) {

            prevtime = int(*ptr - now);

            // find the element in the map and remove it
            ExpireMap::iterator ei = findExpire(*ptr,key);
            assert( ei != expireMap_.end() );
            if ( ei->second.second != maxtime ) {
                if ( ei->first == expirationTime ) break; // already the same time
                --activeSessions_;
            }
            expireMap_.erase( ei );
            *ptr = expirationTime;
        } else {
            expireHash_.Insert(key, expirationTime);
            prevtime = -1;
            ++mapsz;
        }
        actsz = ++activeSessions_;
        expireMap_.insert
            ( std::make_pair( expirationTime, std::make_pair
                              ( key, lastaccessTime ) ) );


     */

        if ( expirationTime < now || !isStarted() ||
             // activeSessions_ > flushLimitSize_ ||
             // unsigned(now - oldestsession_) > flushLimitTime_ ||
             inputList_->size() > 50 ) expireMonitor_.notify();

    }
    // smsc_log_debug( log_, "scheduleExpire(etime=%d,key=%s): time=%d => %d, cnt=%u/%u",
    // key.toString().c_str(),
    // prevtime, int( expirationTime - now ),
    // actsz, mapsz );
}


bool SessionManagerImpl::finalize( Session& session )
{
    smsc_log_debug( log_, "session=%p/%s: finalize", &session, session.sessionKey().toString().c_str() );

    try {

        RuleStatus rs;
        re::RuleEngine::Instance().processSession( session, rs );
        LongCallContext& lcmCtx = session.getLongCallContext();
        if ( rs.status == STATUS_LONG_CALL ) {

            lcmCtx.stateMachineContext = &session;
            lcmCtx.initiator = this;
            if ( LongCallManager::Instance().call(&lcmCtx) )
                // successfully called, session is not finalized
                return false;

        } else if ( rs.status == STATUS_OK ) {

            lcmCtx.runPostProcessActions();

        }

    } catch ( std::exception& exc ) {
        smsc_log_error( log_, "session=%p/%s: finalize: %s",
                        &session, session.sessionKey().toString().c_str(), exc.what() );
    } catch (...) {
        smsc_log_error( log_, "session=%p/%s: finalize: unknown error",
                        &session, session.sessionKey().toString().c_str() );
    }
    return true;
}


SessionManagerImpl::ExpireMap::iterator
    SessionManagerImpl::findExpire( time_t expire, const SessionKey& key )
{
    ExpireMap::iterator i = expireMap_.lower_bound(expire);
    for ( ; ; ++i ) {
        if ( i == expireMap_.end() || i->first != expire ) {
            smsc_log_error( log_, "Logic error in scheduleExpire(key=%s): %s",
                            key.toString().c_str(),
                            i == expireMap_.end() ? "at end" : "time mismatch" );
            i = expireMap_.end();
            break;
        }
        if ( i->second->key == key ) break;
    }
    return i;
}

}}
