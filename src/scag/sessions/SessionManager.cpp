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

namespace scag { namespace sessions
{
    const time_t SessionManager::DEFAULT_EXPIRE_INTERVAL = 60;

    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;
    using namespace scag::exceptions;
    using namespace smsc::core::buffers;
    using namespace scag::util::sms;
    using namespace re;
    using namespace scag::config;
    using scag::config::SessionManagerConfig;

    using smsc::logger::Logger;


    class SessionManagerImpl : public Thread, public ConfigListener, public SessionManager
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

        Mutex           stopLock;
        Event           awakeEvent, exitEvent;
        bool            bStarted;

        CachedSessionStore    store;
        SessionManagerConfig config;

        void Stop();
        bool isStarted();
        int  processExpire();
        void deleteSession(SessionPtr& session);

        uint16_t getNewUSR(Address& address);
        uint16_t getLastUSR(Address& address);

        void reorderExpireQueue(Session* session);
    public:
        void configChanged();

        void AddRestoredSession(Session * session);

        void init(const SessionManagerConfig& config);

        SessionManagerImpl() : bStarted(false) , logger(0), ConfigListener(SESSIONMAN_CFG) {};
        virtual ~SessionManagerImpl();

        // SessionManager interface
        virtual bool getSession(const CSessionKey& key, SessionPtr& session, SCAGCommand& cmd);
        virtual void releaseSession(SessionPtr session);
        virtual uint32_t getSessionsCount();

        virtual int Execute();
        virtual void Start();

        virtual SessionPtr newSession(CSessionKey& sessionKey);
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
        store.updateSession(session);
        smsc_log_debug(logger,"SessionManager: session updated.  USR='%d', Address='%s' Pending: %d InUse: %d",
                       sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), session->getPendingAmount(), SessionHash.Count());

    }
    else
        store.deleteSession(sessionKey);
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
int SessionManagerImpl::Execute()
{
    smsc_log_info(logger,"SessionManager::start executing");

    while (isStarted())
    {
        int secs = processExpire();
//        smsc_log_debug(logger,"SessionManager::----------- ping %d",secs);
        awakeEvent.Wait(secs*1000);
    }
    smsc_log_info(logger,"SessionManager::stop executing");
    exitEvent.Signal();
    return 0;
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
    store.deleteSession(sessionKey);

    smsc_log_debug(logger,"SessionManager: session closed USR='%d', Address='%s' InUse: %d",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), SessionHash.Count());
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

    smsc_log_debug(logger,"SessionManager: process expire InUse: %d, ExpirePoolSize: %d", SessionHash.Count(), SessionExpirePool.size());

    if(SessionExpirePool.empty()) return DEFAULT_EXPIRE_INTERVAL;

    time_t now = time(NULL);
    CSessionSetIterator it;

    bool changed = false;
    it = SessionExpirePool.begin();

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
                    deleteSession(session);
                else
                    store.updateSession(session.Get());
            }
            else
            {
                smsc_log_debug(logger,"SessionManager: Session USR='%d', Address='%s' cannot be found in store",
                               accessData->SessionKey.USR, accessData->SessionKey.abonentAddr.toString().c_str());
                deleteSession(session);
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
        return true;
    }

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

    smsc_log_error(logger,"SessionManager: release session USR='%d', Address='%s'",
           session->getUSR(), key.abonentAddr.toString().c_str());

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
        deleteSession(session);
        SessionHash.Delete(key);
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

uint32_t SessionManagerImpl::getSessionsCount()
{
    MutexGuard mt(inUseMonitor);
    return store.getSessionsCount();
}

}}
