#include "SessionManager2.h"
#include "scag/util/UnlockMutexGuard.h"
#include "scag/re/base/RuleEngine2.h"
#include "scag/lcm/base/LongCallManager2.h"

namespace {

using namespace scag2::sessions;

struct lessAccessTime
{
    bool operator() ( const std::pair<time_t, SessionKey>& a,
                      const std::pair<time_t, SessionKey>& b ) const
    {
        return ( a.first < b.first ) || ( a.first == b.first && a.second < b.second );
    }
};

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
cmdqueue_(0),
log_(0),
started_(false)
{
    log_ = smsc::logger::Logger::getInstance("sess.man");
}


SessionManagerImpl::~SessionManagerImpl()
{
    Stop();
    smsc_log_debug(log_,"SessionManager released");
}


size_t SessionManagerImpl::flushSizeLimit() const
{
    return 1000;
}


void SessionManagerImpl::init( const scag2::config::SessionManagerConfig& cfg,
                               unsigned nodeNumber,
                               SCAGCommandQueue& cmdqueue ) // possible throws exceptions
{
    nodeNumber_ = nodeNumber;
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
                  cfg.name,
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

    } // if started
    this->WaitFor();
    smsc_log_info(log_,"SessionManager::stop");
}


int SessionManagerImpl::Execute()
{
    smsc_log_info( log_, "SessionManager:start executing" );

    const int deftmo = 1000;
    MutexGuard mg(expireMonitor_);
    bool alldone = true;
    while( true ) {

        int curtmo = deftmo;
        if ( ! expireMap_.empty() ) {
            int next = int((expireMap_.begin()->first - time(0))*1000); // in ms
            if ( curtmo > next ) curtmo = next;
        }
        if ( isStarted() && curtmo > 0 &&
             expireMap_.size() < flushSizeLimit() ) expireMonitor_.wait( curtmo );

        const time_t now = time(0);
        std::vector< SessionKey > curset;
        {
            ExpireMap::iterator i;
            if ( ! isStarted() ) {
                if ( expireMap_.size() == 0 && alldone ) break;
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
                curset.push_back( j->second );
                expireHash_.Delete( j->second );
            }
            expireMap_.erase( expireMap_.begin(), i );
        }

        // const size_t expiredCount = curset.size();

        std::vector< SessionKey > flushset;
        if ( expireMap_.size() > flushSizeLimit() ) {
            // too many living sessions
            typedef std::vector< std::pair<time_t, SessionKey > > OldAccess_type;
            OldAccess_type oldaccess;
            oldaccess.reserve( expireMap_.size() );
            std::copy( expireMap_.begin(), expireMap_.end(),
                       std::back_inserter(oldaccess) );
            std::sort( oldaccess.begin(), oldaccess.end(),
                       ::lessAccessTime() );
            size_t extracount = oldaccess.size() - size_t(flushSizeLimit()*0.9);
            flushset.reserve( extracount );
            for ( OldAccess_type::const_iterator i = oldaccess.begin();
                  extracount > 0;
                  --extracount ) {

                ++i;
                flushset.push_back( i->second );
                eraseExpire( i->first, i->second );
                expireHash_.Delete( i->second );

            }
            
        }

        if ( !curset.empty() || !flushset.empty() ) {

            UnlockMutexGuard ug( expireMonitor_ );
            alldone = store_->expireSessions( curset, flushset );

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
        scheduleExpire( session->expirationTime(), *session );
    }
}


void SessionManagerImpl::scheduleExpire( time_t expirationTime,
                                         Session& session )
{
    unsigned mapsz;
    unsigned hashsz;
    time_t now = time(0);
    int prevtime;
    const SessionKey& key( session.sessionKey() );
    do {

        MutexGuard mg(expireMonitor_);
        time_t* ptr = expireHash_.GetPtr(key);
        mapsz = unsigned(expireMap_.size());
        hashsz = expireHash_.Count();

        session.setLastAccessTime( now );

        if ( ptr ) {

            prevtime = int(*ptr - now);
            if ( *ptr == expirationTime ) break; // time has not changed

            // find the element in the map and remove it
            eraseExpire(*ptr,key);
            *ptr = expirationTime;
        } else {
            expireHash_.Insert(key, expirationTime);
            prevtime = -1;
            ++mapsz;
            ++hashsz;
        }
        expireMap_.insert( std::pair< time_t, SessionKey >(expirationTime,key) );

        if ( expirationTime < now || !isStarted() ||
             expireMap_.size() > flushSizeLimit() ) expireMonitor_.notify();

    } while ( false );

    smsc_log_debug( log_, "scheduleExpire(key=%s): time=%d => %d, cnt=%u/%u",
                    key.toString().c_str(),
                    prevtime, int( expirationTime - now ),
                    mapsz, hashsz );
}


bool SessionManagerImpl::finalize( Session& session )
{
    smsc_log_debug( log_, "finalize: session=%p, key=%s", &session, session.sessionKey().toString().c_str() );

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

    } catch ( SCAGException& exc ) {
        smsc_log_error( log_, "finalize: %s", exc.what() );
    } catch (...) {
        smsc_log_error( log_, "finalize: unknown error" );
    }
    return true;
}


void SessionManagerImpl::eraseExpire( time_t expire, const SessionKey& key )
{
    for ( ExpireMap::iterator i = expireMap_.lower_bound(expire);
          ;
          ++i ) {
        if ( i == expireMap_.end() || i->first != expire ) {
            smsc_log_error( log_, "Logic error in scheduleExpire: %s",
                            i == expireMap_.end() ? "at end" : "time mismatch" );
            ::abort();
        }
        if ( i->second == key ) {
            expireMap_.erase( i );
            break;
        }
    }
}

}}
