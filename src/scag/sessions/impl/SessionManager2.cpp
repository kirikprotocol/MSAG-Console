#include "SessionManager2.h"
#include "scag/util/UnlockMutexGuard.h"
#include "scag/re/base/RuleEngine2.h"
#include "scag/lcm/base/LongCallManager2.h"

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

static bool  bSessionManagerInited = false;
static Mutex initSessionManagerLock;

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


#if 0
void SessionManagerImpl::AddRestoredSession(Session * session)
{
    MutexGuard mt(inUseMonitor);

    CSessionKey sessionKey = session->getSessionKey();

    //smsc_log_debug(logger,"SessionManager: Restoring session from store");

    session->expirePendingOperation();

    if(session->hasPending())
    {
        uint16_t lastUSR = getLastUSR(sessionKey.abonentAddr);
        lastUSR++;

        sessionKey.USR = sessionKey.USR > lastUSR ? sessionKey.USR : lastUSR;
        UMRHash.Insert(sessionKey.abonentAddr, sessionKey.USR);

        session->setSessionKey(sessionKey);

        int key;
        Operation* value = 0;

        COperationsHash::Iterator iter = session->OperationsHash.First();

        smsc_log_debug(logger,"SessionManager: Session USR='%d', Address='%s' has %d operations",
                       sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), session->OperationsHash.Count());

        for (;iter.Next(key, value);)
        {
            smsc_log_debug(logger,"SessionManager: Session USR='%d', Address='%s' operation has finished (TYPE=%d)",
                           sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), value->type);

            delete value;
            session->OperationsHash.Delete(key);
        }

        session->m_pCurrentOperation = 0;
        session->currentOperationId = 0;

        reorderExpireQueue(session);
        store_.storeSessionIndex(session);
        if(store_.updateSession(session))
        {
            smsc_log_debug(logger,"SessionManager: session updated.  USR='%d', Address='%s' Pending: %d InUse: %d",
                       sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), session->getPendingAmount(), SessionHash.Count());
            sessionCount++;
        }
    }
    else
    {
         store_.deleteSessionByIndex(session);
        //store_.deleteSession(sessionKey);
    }
    /*smsc_log_debug(logger,"SessionManager: Session restored from store with USR='%d', Address='%s', pending: %d-%d",
       accessData->SessionKey.USR,accessData->SessionKey.abonentAddr.toString().c_str(),
       session->PendingOperationList.size(),session->PrePendingOperationList.size());
       */
}
#endif // if 0


/*
void SessionManager::Init( unsigned theNodeNumber,
                           const scag2::config::SessionManagerConfig& config,
                           SCAGCommandQueue&           cmdqueue )
{
    if (!bSessionManagerInited)
    {
        MutexGuard guard(initSessionManagerLock);
        if (!bSessionManagerInited) {
            SessionManagerImpl& sm = SingleSM::Instance();
            sm.nodeNumber = theNodeNumber;
            sm.cmdqueue = &cmdqueue;
            sm.init(config);
            sm.Start();
            bSessionManagerInited = true;
        }
    }
}
 */

/*
SessionManager& SessionManager::Instance()
{
    if (!bSessionManagerInited)
    {
        MutexGuard guard(initSessionManagerLock);
        if (!bSessionManagerInited)
            throw std::runtime_error("SessionManager not inited!");
    }
    return SingleSM::Instance();
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

    /*
    CSessionSetIterator it;

    for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it)
    {
    delete (*it);
    }

    CSessionKey key;
    SessionPtr* value;

    SessionHash.First();
    for (CSessionHash::Iterator it = SessionHash.getIterator(); it.Next(key, value);)
    {
        SessionPtr session = store_.getSession(key);
        if(session.Get())
            session->abort();
        else
            smsc_log_debug(logger,"SessionManager: cannot find session in store - USR='%d', Address='%s'",
                  key.USR,key.abonentAddr.toString().c_str());
    }
     */

    smsc_log_debug(log_,"SessionManager released");
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

    if ( ! store_.get() )
        store_.reset( new SessionStoreImpl( *this,
                                            *this,
                                            new SessionAllocator() ) );

    store_->init( nodeNumber_,
                  *cmdqueue_,
                  cfg.dir,
                  cfg.name,
                  cfg.indexgrowth,
                  cfg.pagesize,
                  cfg.prealloc );

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


/*
bool SessionManagerImpl::deleteQueuePop( Session* s )
{
  MutexGuard m(inUseMonitor);
    if(!deleteQueue.Count()) return false;
    deleteQueue.Pop(s);
    const CSessionKey& sessionKey = s->getSessionKey();
    smsc_log_debug(logger,"SessionManager: deleteQueuePop USR='%d', Address='%s' InUse: %d, DeleteQueueLen: %d",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), SessionHash.Count(), deleteQueue.Count());
  return true;
}

void SessionManagerImpl::deleteQueuePush(SessionPtr& s, bool expired)
{
    s->setExpired(expired);
    s->deleteScheduled = true;
    const CSessionKey& sessionKey = s->getSessionKey();
    smsc_log_debug(logger,"SessionManager: deleteQueuePush USR='%d', Address='%s' InUse: %d, DeleteQueueLen: %d",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), SessionHash.Count(), deleteQueue.Count());
    deleteQueue.Push(s);
}
 */


int SessionManagerImpl::Execute()
{
    smsc_log_info( log_, "SessionManager::start executing" );

    const int deftmo = 1000;
    MutexGuard mg(expireMonitor_);
    bool alldone = true;
    while( true ) {

        /*
        SessionPtr s;
        while(deleteQueuePop(s))
        {
            if(processDeleteSession(s))
            {
                MutexGuard mt(inUseMonitor);
                deleteSession(s);
            }
        }
         */
        int curtmo = deftmo;
        if ( expireSet_.size() > 0 ) {
            int next = (expireSet_.begin()->expiration - time(0))*1000; // in ms
            if ( curtmo > next ) curtmo = next;
        }
        if ( curtmo > 0 && isStarted() ) expireMonitor_.wait( curtmo );

        const time_t now = time(0);
        std::vector< SessionKey > curset;
        {
            ExpireSet::iterator i;
            if ( ! isStarted() ) {
                if ( expireSet_.size() == 0 && alldone ) break;
                curset.reserve( expireSet_.size() );
                i = expireSet_.end();
                smsc_log_debug(log_, "taking the whole expire set, sz=%u", expireSet_.size() );
            } else {
                ExpireData d( now, SessionKey() );
                i = expireSet_.lower_bound(d);
            }
            for ( ExpireSet::iterator j = expireSet_.begin(); j != i ; ++j ) {
                curset.push_back( j->key );
            }
            expireSet_.erase( expireSet_.begin(), i );
        }

        {
            UnlockMutexGuard ug( expireMonitor_ );
            alldone = store_->expireSessions( curset );

            /*
            MutexGuard cmg(cacheLock_);
            dostopping = stopping_;

            if ( curset.size() > 0 ) smsc_log_debug(log_, "%u sessions expired", curset.size() );

            for ( ExpireSet::iterator i = curset.begin();
                      i != curset.end();
                      ++i ) {

                    const SessionKey& key = i->key;
                    MemStorage::stored_type* v = cache_->get( key );
                    if ( !v ) {
                        // smsc_log_warn(log_,"key=%s to be expired is not found, sz=%u", key.toString().c_str(), curset.size() );
                        continue;
                    }

                    Session* session = cache_->store2val(*v);
                    // smsc_log_debug(log_, "expired key=%s session=%p", key.toString().c_str(), session );
                    const time_t newexpiration = session->expirationTime();

                    if ( session->currentCommand() ) {
                        smsc_log_debug(log_,"key=%s session=%p is not free, cmd=%p, skipped",
                                       key.toString().c_str(), session, session->currentCommand() );
                        newset.push_back( ExpireData(newexpiration,key) );
                        continue;
                    }

                    // check expiration time again, as it may be prolonged while we were waiting
                    if ( ! stopping_ ) {
                        if ( curtime < newexpiration ) {
                            smsc_log_debug(log_,"key=%s session=%p is not expired yet",
                                           key.toString().c_str(), session );
                            newset.push_back( ExpireData(newexpiration,key) );
                            continue;
                        }
                    }

                    smsc_log_debug(log_, "finalizing key=%s session=%p", key.toString().c_str(), session );
                    fin_->finalize( *session );
                    delete cache_->release( key );

                }
             */

        } // lock expireMonitor_

        /*
        if ( newset.size() > 0 ) smsc_log_debug( log_, "%u sessions has not expired yet", newset.size() );
        for ( std::vector< ExpireData >::const_iterator i = newset.begin();
              i != newset.end();
              ++i ) {
            expireSet_.insert(*i);
        }
         */

    } // while expireSet_ is not empty

    smsc_log_info( log_, "SessionManager::stop executing" );
    return 0;
}

/*
bool SessionManagerImpl::processDeleteSession(SessionPtr& session)
{
    RuleStatus rs;
    const CSessionKey& sessionKey = session->getSessionKey();
    smsc_log_debug(log_,"SessionManager: processingSessionDestroy USR='%d', Address='%s' InUse: %d, DeleteQueueLen: %d",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), SessionHash.Count(), deleteQueue.Count());
    try{
        scag::re::RuleEngine::Instance().processSession(*session, rs);
        LongCallContext& lcmCtx = session->getLongCallContext();
        if(rs.status == STATUS_LONG_CALL)
        {
            lcmCtx.stateMachineContext = new SessionPtr(session);
            lcmCtx.initiator = this;
            if(!LongCallManager::Instance().call(&lcmCtx))
                delete static_cast<SessionPtr*>(lcmCtx.stateMachineContext);
            else
                return false;
        }
        else if(rs.status == STATUS_OK)
            lcmCtx.runPostProcessActions();
    }
    catch(SCAGException& exc)
    {
        smsc_log_error(logger, "deleteSession: %s", exc.what());
    }
    catch(...)
    {
        smsc_log_error(logger, "deleteSession: unknown error");
    }
    smsc_log_debug(logger,"SessionManager: processedSessionDestroy USR='%d', Address='%s' InUse: %d, DeleteQueueLen: %d",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), SessionHash.Count(), deleteQueue.Count());
    return true;
}

void SessionManagerImpl::deleteSession(SessionPtr& session)
{
    const CSessionKey& sessionKey = session->getSessionKey();
    CSessionSetIterator* itPtr = SessionExpireHash.GetPtr(sessionKey);
    if(itPtr)
    {
        SessionExpirePool.erase(*itPtr);
        SessionExpireHash.Delete(sessionKey);
    }
    SessionHash.Delete(sessionKey);
    store_.deleteSession(sessionKey);
    sessionCount--;
    smsc_log_debug(logger,"SessionManager: session closed USR='%d', Address='%s' InUse: %d, DeleteQueueLen: %d",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), SessionHash.Count(), deleteQueue.Count());
}

void SessionManagerImpl::reorderExpireQueue(Session* session)
{
    const CSessionKey& sessionKey = session->getSessionKey();
    CSessionSetIterator* itPtr = SessionExpireHash.GetPtr(sessionKey);

    if(itPtr)
    {
        CSessionAccessData* ad = *(*itPtr);

        SessionExpirePool.erase(*itPtr);
        SessionExpireHash.Delete(sessionKey);

        if(session->hasPending())
        {
            ad->nextWakeTime = session->getWakeUpTime();
            SessionExpireHash.Insert(sessionKey, SessionExpirePool.insert(ad));
        }
        else
            delete ad;
    }
    else if(session->hasPending())
    {
        CSessionAccessData* ad = new CSessionAccessData(session->getWakeUpTime(), sessionKey);
        SessionExpireHash.Insert(sessionKey, SessionExpirePool.insert(ad));
    }
}

int SessionManagerImpl::processExpire()
{
    MutexGuard guard(inUseMonitor);

    smsc_log_debug(logger,"SessionManager: process expire InUse: %d, ExpirePoolSize: %d, ExpireHashSize: %d", SessionHash.Count(), SessionExpirePool.size(), SessionExpireHash.Count());

    if(SessionExpirePool.empty()) return DEFAULT_EXPIRE_INTERVAL;

    time_t now = time(NULL);
    CSessionSetIterator it;

    bool changed = false;
    it = SessionExpirePool.begin();

    smsc_log_debug(logger,"SessionManager: process expire, headtime: %d, curtime=%d", (*it)->nextWakeTime, time(NULL));
    while(it != SessionExpirePool.end() && (*it)->nextWakeTime <= now)
    {
        CSessionAccessData *accessData = *it++;
        SessionPtr* s = SessionHash.GetPtr(accessData->SessionKey);
        if(!s)
        {
            SessionPtr session = store_.getSession(accessData->SessionKey);
            if(session.Get())
            {
                if(session->hasPending())
                {
                    smsc_log_debug(logger,"SessionManager: try to expire session USR='%d', Address='%s', has p: %d op: %d",
                               accessData->SessionKey.USR, accessData->SessionKey.abonentAddr.toString().c_str(),
                               session->hasPending(),session->hasOperations());

                    if(session->expirePendingOperation())
                        reorderExpireQueue(session.Get());
                }
                if(!session->hasOperations())
                {
                    SessionHash.Insert(session->getSessionKey(), session);
                    deleteQueuePush(session, true);
                }
                else
                    store_.updateSession(session.Get());
            }
            else
            {
                smsc_log_debug(logger,"SessionManager: Session USR='%d', Address='%s' cannot be found in store",
                               accessData->SessionKey.USR, accessData->SessionKey.abonentAddr.toString().c_str());
                SessionExpirePool.erase(accessData);
                SessionExpireHash.Delete(accessData->SessionKey);
            }
            changed = true;
        }
    }

    int iPeriod = DEFAULT_EXPIRE_INTERVAL;
    if(!SessionExpirePool.empty() && changed)
        iPeriod = (*SessionExpirePool.begin())->nextWakeTime - now;
    return iPeriod;
}
 */


/*
    MutexGuard guard(inUseMonitor);

    smsc_log_debug(logger,"SessionManager: try to get session USR='%d', Address='%s' InUse: %d",
                   key.USR, key.abonentAddr.toString().c_str(), SessionHash.Count());

    SessionPtr* s = SessionHash.GetPtr(key);

    if(!s)
    {
        session = store_.getSession(key);

        if(!session.Get())
        {
            smsc_log_warn(logger, "SessionManager: Cannot find session USR='%d', Address='%s' to get",
                          key.USR, key.abonentAddr.toString().c_str());
            return true;
        }

        SessionHash.Insert(key, session);

        smsc_log_debug(logger,"SessionManager: got session USR='%d', Address='%s' (pending count=%d) InUse: %d",
            session->getUSR(), key.abonentAddr.toString().c_str(), session->PendingOperationList.size(), SessionHash.Count());

        session->m_CanOpenSubmitOperation = false;
        session->getLongCallContext().clear();
        return true;
    }

    if((*s)->deleteScheduled)
    {
        smsc_log_debug(logger, "SessionManager: Session scheduled for deletion (cannot get) USR='%d', Address='%s'",
                   key.USR, key.abonentAddr.toString().c_str());
        session = NULL;
        return true;
    }

    (*s)->m_CanOpenSubmitOperation = false;

    cmd.setSession(*s);

    (*s)->pushCommand(cmd.getType() == SMPP ? new SmppCommand((SmppCommand&)cmd) : &cmd);

    smsc_log_debug(logger, "SessionManager: Session locked USR='%d', Address='%s', command pushed to session queue, transport %d. Commands count: %d",
                   key.USR, key.abonentAddr.toString().c_str(), cmd.getType(), (*s)->commandsCount());
    return false;
}

SessionPtr SessionManagerImpl::newSession(CSessionKey& key)
{
    MutexGuard guard(inUseMonitor);

    SessionPtr session(0);

    smsc_log_debug(logger,"SessionManager: creating new session for '%s' InUse: %d", key.abonentAddr.toString().c_str(), SessionHash.Count());

    key.USR = getNewUSR(key.abonentAddr);
    session = store_.newSession(key);
    session->setSessionKey(key);
    session->m_CanOpenSubmitOperation = true;
    sessionCount++;

    SessionHash.Insert(key, session);

    smsc_log_debug(logger,"SessionManager: created new session USR='%d', Address='%s' InUse: %d",
                   session->getUSR(), key.abonentAddr.toString().c_str(), SessionHash.Count());

    return session;
}

void SessionManagerImpl::releaseSession(SessionPtr session)
{
    if (!session.Get()) return;
    MutexGuard guard(inUseMonitor);
    const CSessionKey& key = session->getSessionKey();

    SessionPtr* s = SessionHash.GetPtr(key);
    if (!s)
    {
        smsc_log_warn(logger,"SessionManager: cannot find session USR='%d', Address='%s' to release",
                   session->getUSR(), key.abonentAddr.toString().c_str());
        return;
    }

    std::list<PendingOperation>::iterator itPending;
    if(session->PrePendingOperationList.size() > 0)
    {
        for (itPending = session->PrePendingOperationList.begin(); itPending!=session->PrePendingOperationList.end(); ++itPending)
            session->DoAddPendingOperation(*itPending);

        session->PrePendingOperationList.clear();
    }

    if(!session->commandsEmpty())
    {
        SCAGCommand* cmd = session->popCommand();
        smsc_log_debug(logger,"SessionManager: release: push command to state machine USR='%d', Address='%s' transport=%d Commands Count: %d",
                       key.USR, key.abonentAddr.toString().c_str(), cmd->getType(), session->commandsCount());
        if(cmd->getType() == HTTP)
            scag::transport::http::HttpManager::Instance().process(((scag::transport::http::HttpCommand*)cmd)->getContext());
        else if(cmd->getType() == SMPP)
        {
            scag::transport::smpp::SmppManager::Instance().pushCommand(*(SmppCommand*)cmd);
            delete (SmppCommand*)cmd;
        }
        return;
    }

    if(!session->hasOperations())
    {
        deleteQueuePush(session, false);
        awakeEvent.Signal();
        return;
    }

    reorderExpireQueue(session.Get());

    store_.updateSession(session.Get());
    SessionHash.Delete(key);

    awakeEvent.Signal();

    smsc_log_debug(logger,"SessionManager: session released USR='%d', Address='%s' (pending count=%d) InUse: %d",
                   key.USR, key.abonentAddr.toString().c_str(), session->PendingOperationList.size(), SessionHash.Count());
}

uint16_t SessionManagerImpl::getLastUSR(Address& address)
{
    int* resultPtr = UMRHash.GetPtr(address);
    return resultPtr ? *resultPtr : 0;
}

uint16_t SessionManagerImpl::getNewUSR(Address& address)
{
    uint16_t result = 1;

    int * resultPtr = UMRHash.GetPtr(address);
    if (resultPtr) result = ++(*resultPtr);
    else UMRHash.Insert(address,result);

    return result;
}
 */

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
        scheduleExpire( session->expirationTime(), session->sessionKey() );
    }
}


void SessionManagerImpl::scheduleExpire( time_t expirationTime,
                                         const SessionKey& key )
{
    MutexGuard mg(expireMonitor_);
    smsc_log_debug( log_, "place key=%s to expire queue", key.toString().c_str() );
    expireSet_.insert( ExpireData(expirationTime,key) );
    if ( expirationTime < time(0) || !isStarted() ) expireMonitor_.notify();
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

}}
