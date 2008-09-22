#include "SessionManager2.h"
#include "scag/util/UnlockMutexGuard.h"
#include "scag/re/base/RuleEngine2.h"
#include "scag/lcm/base/LongCallManager2.h"

namespace {

using namespace scag2::sessions;

struct lessAccessTime
{
    bool operator() ( const std::pair<SessionKey,time_t>* a,
                      const std::pair<SessionKey,time_t>* b ) const
    {
        return ( a->second < b->second ) || ( a->second == b->second && a->first < b->first );
    }
};

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

// ################## Singleton related issues ##################

// static bool  bSessionManagerInited = false;
// static Mutex initSessionManagerLock;

// inline unsigned GetLongevity(SessionManager*) { return 6; } // ? Move upper ?
// typedef SingletonHolder<SessionManagerImpl> SingleSM;

/*
void SessionManagerCallback(void * sm,Session * session)
{
    if (sm == 0) return;
    if (session == 0) return;

    SessionManagerImpl * smImpl = (SessionManagerImpl *)sm;

    smImpl->AddRestoredSession(session);
}
 */


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
    log_ = smsc::logger::Logger::getInstance("sess.man");
}


SessionManagerImpl::~SessionManagerImpl()
{
    Stop();
    smsc_log_debug(log_,"SessionManager released");
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

    if (!log_) log_ = Logger::getInstance("sess.man");

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


void SessionManagerImpl::getSessionsCount(uint32_t& sessionsCount, uint32_t& sessionsLockedCount)
{
    if ( store_.get() ) store_->getSessionsCount( sessionsCount, sessionsLockedCount );
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

            unsigned sessionLockedCount = 0;
            for ( int passes = 0; ; ) {

                unsigned sessionCount;
                unsigned newSessionLockedCount;
                store_->getSessionsCount( sessionCount, newSessionLockedCount );
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
            smsc_log_info( log_, "all sessions are in expire pool" );
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

    const int deftmo = 1000;
    // MutexGuard mg(expireMonitor_);
    bool alldone = true;
    bool started = true;
    time_t oldtime;
    while( true ) {

        ExpireList* list = 0;
        {
            MutexGuard mg(expireMonitor_);
            if ( inputList_ && ! inputList_->empty() ) {
                list = inputList_;
                inputList_ = 0;
            }
            started = isStarted();
            oldtime = oldestsession_;
        }

        // process input
        if ( list ) {

            // expireList_ is a std::list< Expire >
            // expireMap_  is a std::multimap< time_t, ExpireList::iterator >
            // expireHash_ is XHash< KeyPtr, ExpireList::iterator, KeyPtr >

            for ( ; ! list->empty(); list->pop_front() ) {

                Expire& e = list->front();
                ExpireList::iterator i;
                ExpireList::iterator* ptr = expireHash_.GetPtr(KeyPtr(e));
                if ( ptr ) {
                    // found
                    i = ei->second;
                    ExpireMap::iterator ei = findExpire(*ptr,e.key);
                    if ( i->access == maxtime && e.access != maxtime ) ++activeSessions_;
                    i->access = e.access;
                    if ( i->expire == e.expire ) continue;
                    expireMap_.erase( ei );
                } else {
                    // not found
                    ++activeSessions_;
                    expireList_.push_front( e );
                    i = expireList_.begin();
                    expireHash_.Insert( KeyPtr(*i), i );
                }
                expireMap_.insert( i->expire, i );

            }
            delete list;
        }

        int curtmo = deftmo;
        if ( ! expireMap_.empty() ) {
            int next = int((expireMap_.begin()->first - time(0))*1000); // in ms
            if ( curtmo > next ) curtmo = next;
        }
        time_t now = time(0);
        // oldwait is a number of ms to wait for oldest session flush time
        // NOTE: we also add 5 seconds to bunch old session processing.
        int oldwait = int(oldttime+flushLimitTime_+5 - now) * 1000; // in ms
        if ( curtmo > oldwait ) curtmo = oldwait;

        if ( started ) {
            if ( curtmo > 0 && activeSessions_ <= flushLimitSize_ ) {
                expireMonitor_.wait(curtmo);
                continue;
            }
            // now = time(0);
            // oldwait = int(oldestsession_+flushLimitTime_+5 - now) * 1000;
        } else if ( expireMap_.empty() ) {
            continue;
            // expireMonitor_.wait(500);
            // continue;
        }

        std::vector< SessionKey > curset;
        {
            ExpireMap::iterator i;
            if ( alldone ) break;
            // FIXME

                    expireMonitor_.wait( 500 );
                    continue;
                }
                curset.reserve( expireMap_.size() );
                i = expireMap_.end();
                smsc_log_debug(log_, "taking the whole expire set, sz=%u", expireMap_.size() );
            } else {
                i = expireMap_.upper_bound( now );
            }
            // smsc_log_debug( log_, "waked for expiration, cnt=%u/%u", 
            // expireMap_.size(), expireHash_.Count() );
            for ( ExpireMap::iterator j = expireMap_.begin(); j != i ; ++j ) {
                // smsc_log_debug( log_, "prepare key=%s for expiration", j->second.toString().c_str() );
                if ( j->second.second != ::maxtime ) --activeSessions_;
                curset.push_back( j->second.first );
                expireHash_.Delete( j->second.first );
            }
            expireMap_.erase( expireMap_.begin(), i );
        }

        // const size_t expiredCount = curset.size();

        std::vector< std::pair<SessionKey,time_t> > flushset;
        if ( activeSessions_ > flushLimitSize_ || oldwait < 0 ) {

            // too many living sessions
            typedef std::vector< std::pair<SessionKey,time_t>* > OldAccess_type;
            OldAccess_type oldaccess;
            oldaccess.reserve( expireMap_.size() );
            for ( ExpireMap::iterator i = expireMap_.begin();
                  i != expireMap_.end();
                  ++i ) {
                oldaccess.push_back( &(i->second) );
            }
            std::sort( oldaccess.begin(),
                       oldaccess.end(),
                       ::lessAccessTime() );
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
                if ( (*i)->second == maxtime ) continue;
                    
                if ( extracount > 0 ) {
                    --extracount;
                } else if ( (*i)->second <= flushtime ) {
                } else {
                    // a new oldest session
                    oldestsession_ = (*i)->second;
                    break;
                }
                    
                --activeSessions_;
                flushset.push_back(**i);
                (*i)->second = maxtime;

            }
            if ( oldestsession_ == maxtime ) oldestsession_ = now;
            
        }

        if ( !curset.empty() || !flushset.empty() ) {

            const unsigned tot = expireMap_.size();
            const unsigned act = activeSessions_;

            UnlockMutexGuard ug( expireMonitor_ );
            smsc_log_debug( log_, "%u/%u to be expired/flushed, %u/%u left active/total",
                            unsigned(curset.size()),
                            unsigned(flushset.size()),
                            act, tot );
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

        } // lock expireMonitor_

    } // while expireMap_ is not empty

    smsc_log_info( log_, "SessionManager::stop executing" );
    return 0;
}


void SessionManagerImpl::continueExecution( LongCallContext* lcmCtx, bool dropped )
{
    /// long call in session_destroy
    Session* session = reinterpret_cast<Session*>( lcmCtx->stateMachineContext );
    lcmCtx->continueExec = true;

    // special finalization command
    assert( session && session->currentCommand() == 1 );

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
    
        if ( lastaccessTime < oldestsession_ ) oldestsession_ = lastaccessTime;

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
             unsigned(now - oldestsession_) > flushLimitTime_ ||
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
        if ( i->second.first == key ) break;
    }
    return i;
}

}}
