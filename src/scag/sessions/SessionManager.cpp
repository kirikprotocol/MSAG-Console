#include <inttypes.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/buffers/XHash.hpp>

#include <scag/util/singleton/Singleton.h>
#include <set>

#include <unistd.h>
#include <time.h>
#include <logger/Logger.h>

#include "SessionStore.h"
#include "SessionManager.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/util/sms/HashUtil.h"
#include "scag/re/CommandBrige.h"
#include "scag/config/ConfigListener.h"
#include "scag/transport/http/Managers.h"
#include "scag/transport/http/HttpCommand.h"
#include "scag/transport/smpp/SmppManager.h"
#include "scag/lcm/LongCallManager.h"
#include "scag/re/RuleEngine.h"

namespace scag { namespace sessions
{
    const time_t SessionManager::DEFAULT_EXPIRE_INTERVAL = 60;

    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;
    using namespace scag::exceptions;
    using namespace smsc::core::buffers;
    using namespace scag::util::sms;
    using namespace scag::lcm;
    using namespace re;
    using namespace scag::config;
    using scag::config::SessionManagerConfig;

    using smsc::logger::Logger;


    class SessionManagerImpl : public Thread, public ConfigListener, public SessionManager, public LongCallInitiator
    {
        struct CSessionAccessData
        {
            CSessionKey SessionKey;
            time_t nextWakeTime;
            CSessionAccessData(time_t nwt, const CSessionKey& sk) : nextWakeTime(nwt), SessionKey(sk) {}
        };

        struct FWakeTimeCompare
        {
            bool operator () (const CSessionAccessData* x,const CSessionAccessData* y) const
            {
                return (x->nextWakeTime < y->nextWakeTime);
            }
        };

        typedef std::multiset<CSessionAccessData*,FWakeTimeCompare> CSessionSet;
        typedef std::multiset<CSessionAccessData*>::iterator CSessionSetIterator;
        typedef XHash<CSessionKey,CSessionSetIterator,CSessionKey> CSessionExpireHash;
        typedef XHash<CSessionKey,SessionPtr,CSessionKey> CSessionHash;

        typedef XHash<Address,int,XAddrHashFunc> CUMRHash;

        EventMonitor    inUseMonitor;
        CSessionSet     SessionExpirePool;
        CSessionHash    SessionHash;
        CSessionExpireHash SessionExpireHash;
        CUMRHash        UMRHash;
        Logger *        logger;
        time_t          expireSchedule;

        Mutex           stopLock;
        Event           awakeEvent, exitEvent;
        bool            bStarted;
        uint32_t        sessionCount;

        CachedSessionStore    store;
        SessionManagerConfig config;

        buf::CyclicQueue<SessionPtr> deleteQueue;

        void Stop();
        bool isStarted();
        int  processExpire();
        void deleteSession(SessionPtr& session);
        bool processDeleteSession(SessionPtr& session);
      bool deleteQueuePop(SessionPtr& s);
        void deleteQueuePush(SessionPtr& s, bool expired);

        uint16_t getNewUSR(Address& address);
        uint16_t getLastUSR(Address& address);

        void reorderExpireQueue(Session* session);
    public:
        void configChanged();

        void AddRestoredSession(Session * session);

        void init(const SessionManagerConfig& config);

        SessionManagerImpl() : bStarted(false) , logger(0), sessionCount(0), ConfigListener(SESSIONMAN_CFG) {};
        virtual ~SessionManagerImpl();

        // SessionManager interface
        virtual bool getSession(const CSessionKey& key, SessionPtr& session, SCAGCommand& cmd);
        virtual void releaseSession(SessionPtr session);
        virtual void getSessionsCount(uint32_t& sessionsCount, uint32_t& sessionsLockedCount);

        virtual int Execute();
        virtual void Start();

        virtual SessionPtr newSession(CSessionKey& sessionKey);

        virtual void continueExecution(LongCallContext* context, bool dropped);
    };

// ################## Singleton related issues ##################

static bool  bSessionManagerInited = false;
static Mutex initSessionManagerLock;

inline unsigned GetLongevity(SessionManager*) { return 6; } // ? Move upper ?
typedef SingletonHolder<SessionManagerImpl> SingleSM;

void SessionManagerCallback(void * sm,Session * session)
{
    if (sm == 0) return;
    if (session == 0) return;

    SessionManagerImpl * smImpl = (SessionManagerImpl *)sm;

    smImpl->AddRestoredSession(session);
}


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
        store.storeSessionIndex(session);
        if(store.updateSession(session))
        {
            smsc_log_debug(logger,"SessionManager: session updated.  USR='%d', Address='%s' Pending: %d InUse: %d",
                       sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), session->getPendingAmount(), SessionHash.Count());
            sessionCount++;
        }
    }
    else
    {
         store.deleteSessionByIndex(session);
        //store.deleteSession(sessionKey);
    }
    /*smsc_log_debug(logger,"SessionManager: Session restored from store with USR='%d', Address='%s', pending: %d-%d",
       accessData->SessionKey.USR,accessData->SessionKey.abonentAddr.toString().c_str(),
       session->PendingOperationList.size(),session->PrePendingOperationList.size());
       */
}

SessionManagerImpl::~SessionManagerImpl()
{
    Stop();

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
        SessionPtr session = store.getSession(key);
        if(session.Get())
            session->abort();
        else
            smsc_log_debug(logger,"SessionManager: cannot find session in store - USR='%d', Address='%s'",
                  key.USR,key.abonentAddr.toString().c_str());
    }

    smsc_log_debug(logger,"SessionManager released");
}


void SessionManager::Init(const SessionManagerConfig& config)
{
    if (!bSessionManagerInited)
    {
        MutexGuard guard(initSessionManagerLock);
        if (!bSessionManagerInited) {
            SessionManagerImpl& sm = SingleSM::Instance();
            sm.init(config); sm.Start();
            bSessionManagerInited = true;
        }
    }
}
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

void SessionManagerImpl::init(const SessionManagerConfig& _config) // possible throws exceptions
{
    this->config = _config;
    expireSchedule = time(NULL) + DEFAULT_EXPIRE_INTERVAL;

    if (!logger)
      logger = Logger::getInstance("sess.man");

    store.init(config.dir,SessionManagerCallback,this);

    smsc_log_debug(logger,"SessionManager::initialized");
}

void SessionManagerImpl::configChanged()
{
    MutexGuard mt(inUseMonitor);

    Stop();
    init(ConfigManager::Instance().getSessionManConfig());
    Start();
}

bool SessionManagerImpl::isStarted()
{
    //MutexGuard guard(stopLock);
    return bStarted;
}
void SessionManagerImpl::Start()
{
    MutexGuard guard(stopLock);
    if (!bStarted)
    {
        bStarted = true;
        Thread::Start();
    }
}
void SessionManagerImpl::Stop()
{
    MutexGuard guard(stopLock);

    if (bStarted)
    {
        bStarted = false;
        awakeEvent.Signal();
        exitEvent.Wait();
    }
    smsc_log_info(logger,"SessionManager::stop");
}

bool SessionManagerImpl::deleteQueuePop(SessionPtr& s)
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

int SessionManagerImpl::Execute()
{
    smsc_log_info(logger,"SessionManager::start executing");

    while(isStarted())
    {
        SessionPtr s;
        while(deleteQueuePop(s))
        {
            if(processDeleteSession(s))
            {
                MutexGuard mt(inUseMonitor);
                deleteSession(s);
            }
        }
        time_t now = time(NULL);
        if(expireSchedule < now)
            expireSchedule = now + processExpire();
//        smsc_log_debug(logger,"SessionManager::----------- ping %d",secs);
        awakeEvent.Wait(expireSchedule - now);
    }
    smsc_log_info(logger,"SessionManager::stop executing");
    exitEvent.Signal();
    return 0;
}

bool SessionManagerImpl::processDeleteSession(SessionPtr& session)
{
    RuleStatus rs;
    const CSessionKey& sessionKey = session->getSessionKey();
    smsc_log_debug(logger,"SessionManager: processingSessionDestroy USR='%d', Address='%s' InUse: %d, DeleteQueueLen: %d",
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
    store.deleteSession(sessionKey);
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
            SessionPtr session = store.getSession(accessData->SessionKey);
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
                    store.updateSession(session.Get());
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


bool SessionManagerImpl::getSession(const CSessionKey& key, SessionPtr& session, SCAGCommand& cmd)
{
    MutexGuard guard(inUseMonitor);

    smsc_log_debug(logger,"SessionManager: try to get session USR='%d', Address='%s' InUse: %d",
                   key.USR, key.abonentAddr.toString().c_str(), SessionHash.Count());

    SessionPtr* s = SessionHash.GetPtr(key);

    if(!s)
    {
        session = store.getSession(key);

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
    session = store.newSession(key);
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

    store.updateSession(session.Get());
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

void SessionManagerImpl::getSessionsCount(uint32_t& sessionsCount, uint32_t& sessionsLockedCount)
{
//    MutexGuard mt(inUseMonitor);
    sessionsCount = sessionCount;
    sessionsLockedCount = SessionHash.Count();
}

void SessionManagerImpl::continueExecution(LongCallContext* lcmCtx, bool dropped)
{
    SessionPtr* s = (SessionPtr*)lcmCtx->stateMachineContext;
    lcmCtx->continueExec = true;

    if(!dropped)
    {
        MutexGuard mg(inUseMonitor);
        deleteQueue.Push(*s);
        awakeEvent.Signal();
    }
    delete s;
}

}}
