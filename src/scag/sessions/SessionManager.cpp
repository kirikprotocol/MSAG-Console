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
    using scag::config::SessionManagerConfig;

    using smsc::logger::Logger;


    class SessionManagerImpl : public SessionManager, public Thread, public ConfigListener
    {
        struct CSessionAccessData
        {
            CSessionKey SessionKey;
            time_t nextWakeTime;
            bool bOpened;
            bool hasPending;
            bool hasOperations;

            CSessionAccessData() : nextWakeTime(0), bOpened(false), hasPending(false), hasOperations(false) {}
        };

        class XSessionHashFunc{
        public:
            static uint32_t CalcHash(const CSessionKey& key)
            {
                uint32_t retval = (key.abonentAddr.type+key.USR)^key.abonentAddr.plan;
                int i;
                for(i=0;i<key.abonentAddr.length;i++)
                {
                    retval=retval*10+(key.abonentAddr.value[i]-'0');
                }
                return retval;
            }
        };


        struct FAccessDataCompare 
        {
            bool operator () (const CSessionAccessData* x,const CSessionAccessData* y) const
            {
                return (x->nextWakeTime < y->nextWakeTime);
            }
        };

        typedef std::multiset<CSessionAccessData*,FAccessDataCompare> CSessionSet;
        typedef std::multiset<CSessionAccessData*>::iterator CSessionSetIterator;
        typedef XHash<CSessionKey,CSessionSetIterator,XSessionHashFunc> CSessionHash;

        typedef XHash<Address,int,XAddrHashFunc> CUMRHash;

        EventMonitor    inUseMonitor;
        CSessionSet     SessionExpirePool;
        CSessionHash    SessionHash;
        CUMRHash        UMRHash;
        Logger *        logger;

//        std::list<CSessionKey> NeedToUpdateSessionList;

//        int16_t         m_nLastUSR;
        Mutex           stopLock;
        Event           awakeEvent, exitEvent;
        bool            bStarted;

        SessionStore    store;
        SessionManagerConfig config;

        void Stop();
        bool isStarted();
        int  processExpire();
        CSessionSetIterator DeleteSession(CSessionSetIterator it);

        
        int16_t getNewUSR(Address& address);
        int16_t getLastUSR(Address& address);

    public:
        void configChanged();
        
        void AddRestoredSession(Session * session);

        void init(const SessionManagerConfig& config);

        SessionManagerImpl() : bStarted(false) , logger(0), ConfigListener(SESSIONMAN_CFG) {};
        virtual ~SessionManagerImpl();
        
        // SessionManager interface
        virtual SessionPtr getSession(const CSessionKey& sessionKey);
        virtual void releaseSession(SessionPtr session);
        virtual void closeSession(SessionPtr session);
        virtual uint32_t getSessionsCount();

        virtual int Execute();
        virtual void Start();

        virtual SessionPtr newSession(CSessionKey& sessionKey);

    };


//const time_t SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL = 60;

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

    CSessionKey sessionKey;
    sessionKey = session->getSessionKey();

    //smsc_log_debug(logger,"SessionManager: Restoring session from store");

    int16_t lastUSR = getLastUSR(sessionKey.abonentAddr);
    lastUSR++;

    int16_t maxUSR;

    if (sessionKey.USR > lastUSR) maxUSR = sessionKey.USR;
    else maxUSR = lastUSR;

    sessionKey.USR = maxUSR;
    UMRHash.Insert(sessionKey.abonentAddr,maxUSR);

    session->setSessionKey(sessionKey);
    
    time_t time = session->getWakeUpTime();

    CSessionAccessData * accessData = new CSessionAccessData();

    accessData->bOpened = false;
    accessData->nextWakeTime = time;
    accessData->hasPending = session->hasPending();
    accessData->SessionKey = sessionKey;
    accessData->hasOperations = session->hasOperations();


    CSessionSetIterator it = SessionExpirePool.insert(accessData);

/*    if (!pr.second) 
    {
        delete accessData;

        smsc_log_debug(logger,"SessionManager: Error - session cannot be inserted");
        return;
    }*/
    SessionHash.Insert(sessionKey,it);

    smsc_log_debug(logger,"SessionManager: Session restored from store with USR='%d', Address='%s', pending: %d-%d",
		   accessData->SessionKey.USR,accessData->SessionKey.abonentAddr.toString().c_str(),
		   session->PendingOperationList.size(),session->PrePendingOperationList.size());
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
    CSessionSetIterator * value;
    
    SessionHash.First();
    for (CSessionHash::Iterator it = SessionHash.getIterator(); it.Next(key, value);)
    {
        SessionPtr session = store.getSession(key);
        if (session.Get()) session->abort(); 
        else smsc_log_debug(logger,"SessionManager: cannot find session in store - USR='%d', Address='%s'",
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

// ################ TODO: Actual SessionManager Implementation follows ################ 

void SessionManagerImpl::init(const SessionManagerConfig& _config) // possible throws exceptions
{
    this->config = _config;

    if (!logger)
      logger = Logger::getInstance("scag.SessionManager");

    store.init(config.dir,SessionManagerCallback,this);

    
    CSessionSetIterator it;
    for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it)
    {
        CSessionAccessData * accessData = (*it);

        SessionPtr session = store.getSession(accessData->SessionKey);

        int key;
        Operation * value = 0;
        
        COperationsHash::Iterator iter = session->OperationsHash.First();

        smsc_log_debug(logger,"SessionManager: Session USR='%d', Address='%s' has %d operations", 
                       (*it)->SessionKey.USR, (*it)->SessionKey.abonentAddr.toString().c_str(), session->OperationsHash.Count());

        for (;iter.Next(key, value);)
        {              
            smsc_log_debug(logger,"SessionManager: Session USR='%d', Address='%s' operation has finished (TYPE=%d)", 
                           (*it)->SessionKey.USR, (*it)->SessionKey.abonentAddr.toString().c_str(), value->type);

            if (session->m_pCurrentOperation == value) 
            {
                session->m_pCurrentOperation = 0;
                session->currentOperationId = 0;
            }

            delete value;
            session->bChanged = true;

            session->OperationsHash.Delete(key);
        }    

        if (session->bChanged) 
        {
            accessData->hasOperations = session->hasOperations();
            store.updateSession(session);
        }

        smsc_log_debug(logger,"SessionManager:: USR='%d', Address='%s', has pending='%d'",
                       (*it)->SessionKey.USR,(*it)->SessionKey.abonentAddr.toString().c_str(),(*it)->hasPending);
    }

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

//��������� �����. ������������ ������ ��� ��������.
SessionManagerImpl::CSessionSetIterator SessionManagerImpl::DeleteSession(CSessionSetIterator it)
{
    CSessionSetIterator res;

    SessionHash.Delete((*it)->SessionKey);
    CSessionAccessData * accessData = (*it);

    res = it;
    res++;


    SessionExpirePool.erase(it);
    store.deleteSession(accessData->SessionKey);

    smsc_log_debug(logger,"SessionManager: session expired USR='%d', Address='%s'", 
                   accessData->SessionKey.USR, accessData->SessionKey.abonentAddr.toString().c_str());
    delete accessData;

    return res;
}



int SessionManagerImpl::processExpire()
{
    //smsc_log_debug(logger,"SessionManager: process expire");

    MutexGuard guard(inUseMonitor);
    
    while (1) 
    {
        //smsc_log_debug(logger,"SessionManager: processing expire");

        if (SessionExpirePool.empty()) return DEFAULT_EXPIRE_INTERVAL;

        CSessionSetIterator it;

        for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();)
        {
            if ((!(*it)->bOpened)&&((*it)->hasPending)) break;
            if ((!(*it)->bOpened)&&(!(*it)->hasPending)&&(!(*it)->hasOperations)) 
            {
                it = DeleteSession(it);
            } 
            else ++it;
        }

        time_t now;
        time(&now);
        int iPeriod;

        //smsc_log_debug(logger,"SessionManager: process expire - check session");


        if (it == SessionExpirePool.end())
        {
            if (!SessionExpirePool.empty()) 
            {
                it = SessionExpirePool.begin();
                iPeriod = ((*it)->nextWakeTime - now);
                if (iPeriod <= 0) return DEFAULT_EXPIRE_INTERVAL;
                else return iPeriod;
            }
            else 
                return DEFAULT_EXPIRE_INTERVAL;
        }

        iPeriod = (*it)->nextWakeTime - now;

        smsc_log_debug(logger,"SessionManager: session USR= '%d', Address='%s' has period: %d",
                       (*it)->SessionKey.USR,(*it)->SessionKey.abonentAddr.toString().c_str(),iPeriod);

        if (iPeriod > 0) return iPeriod;

        SessionPtr sessionPtr(0);
        sessionPtr = store.getSession((*it)->SessionKey);
        Session * session = sessionPtr.Get();
        CSessionAccessData * data = (*it);

        while ((iPeriod <= 0)&&(data->hasPending))
        {
            if (!session) 
            {
                smsc_log_debug(logger,"SessionManager: Session USR='%d', Address='%s' cannot be found in store",
                               (*it)->SessionKey.USR,(*it)->SessionKey.abonentAddr.toString().c_str());

                SessionHash.Delete((*it)->SessionKey);
                SessionExpirePool.erase(it);
                delete (*it);
                return 0;
            }

            smsc_log_debug(logger,"SessionManager: try to expire session USR='%d', Address='%s', has p: %d op: %d",
                           (*it)->SessionKey.USR,(*it)->SessionKey.abonentAddr.toString().c_str(),
                           session->hasPending(),session->hasOperations());

            session->expirePendingOperation();

            data->hasPending = session->hasPending();
            data->hasOperations = session->hasOperations();

            time_t wakeTime = session->getWakeUpTime();

            if (wakeTime == 0) data->nextWakeTime = now; 
            else data->nextWakeTime = wakeTime;

            iPeriod = data->nextWakeTime - now;
        }

        SessionExpirePool.erase(it);
        it = SessionExpirePool.insert(data);
        SessionHash.Delete(data->SessionKey);
        SessionHash.Insert(data->SessionKey,it);


        // Session expired
        if (!(*it)->hasOperations) 
        {
            DeleteSession(it);
            return 0;
        } 
        else
        {
            store.updateSession(sessionPtr);
            if (iPeriod >= 0) return iPeriod;
        }

    }  
    return 10;
}


SessionPtr SessionManagerImpl::getSession(const CSessionKey& sessionKey)
{
    smsc_log_debug(logger,"SessionManager: try to get session USR='%d', Address='%s'",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str());

    SessionPtr session(0);

    MutexGuard guard(inUseMonitor);

    smsc_log_debug(logger,"SessionManager: wait session USR='%d', Address='%s'",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str());

    CSessionSetIterator * itPtr = SessionHash.GetPtr(sessionKey);
    if (!itPtr) return session; 
    
    CSessionSetIterator it = (*itPtr);
    while ((*it)->bOpened)
    {
        inUseMonitor.wait();
        itPtr = SessionHash.GetPtr(sessionKey);
        smsc_log_debug(logger, "SessionManager: inUse monitor exited for  session USR='%d', Address='%s' (result: %s session, %s)",
                       sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), (itPtr) ? "got":"no",
		       (itPtr && (*(*itPtr))->bOpened) ? "locked":"free");
	if (!itPtr) return session; //session not found
        it = (*itPtr);
    }                                      

    (*it)->bOpened = true;

    session = store.getSession(sessionKey);

    smsc_log_debug(logger,"SessionManager: got session USR='%d', Address='%s' (pending count=%d)", 
                   session->getUSR(), session->getSessionKey().abonentAddr.toString().c_str(), 
		   session->PendingOperationList.size());
    
    return session;
}


SessionPtr SessionManagerImpl::newSession(CSessionKey& sessionKey)
{
    SessionPtr session(0);
    CSessionAccessData * accessData = 0;

    smsc_log_debug(logger,"SessionManager: creating new session for '%s'", 
		   sessionKey.abonentAddr.toString().c_str());

    MutexGuard guard(inUseMonitor);

    smsc_log_debug(logger,"SessionManager: creating new session for '%s', get new USR",
		   sessionKey.abonentAddr.toString().c_str());
    sessionKey.USR = getNewUSR(sessionKey.abonentAddr);

    session = store.newSession(sessionKey);
    session->setSessionKey(sessionKey);
    session->m_CanOpenSubmitOperation = true;

    time_t time = session->getWakeUpTime();

    accessData = new CSessionAccessData();

    accessData->bOpened = true;
    accessData->nextWakeTime = time;
    accessData->hasPending = session->hasPending();
    accessData->SessionKey = sessionKey;
    accessData->hasOperations = session->hasOperations();

    CSessionSetIterator it = SessionExpirePool.insert(accessData);

/*    if (!pr.second) 
    {
        delete accessData;
        store.deleteSession(sessionKey);

        smsc_log_debug(logger,"SessionManager: cannot create a new session - session with such sessionId already exists");
        session = SessionPtr(0);
        return session;
    }*/
    smsc_log_debug(logger,"SessionManager: created new session USR='%d', Address='%s'",
                   session->getUSR(), sessionKey.abonentAddr.toString().c_str());

    SessionHash.Insert(sessionKey,it);
    return session;
}



void SessionManagerImpl::releaseSession(SessionPtr session)
{
    if (!session.Get()) return;
    CSessionKey sessionKey = session->getSessionKey();
    
    smsc_log_debug(logger,"SessionManager: try to release session USR='%d', Address='%s'",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str());

    MutexGuard guard(inUseMonitor);
    //if (session->isChanged()) store.updateSession(session);
    
    CSessionSetIterator * itPtr = SessionHash.GetPtr(sessionKey);

    if (!itPtr) throw 
        SCAGException("SessionManager: Fatal error - cannot find session USR='%d', Address='%s' to release", 
                      sessionKey.USR, sessionKey.abonentAddr.toString().c_str());

    CSessionSetIterator it = (*itPtr);
    CSessionAccessData * accessData = (*it);

    bool changePendingFlag = false;

    std::list<PendingOperation>::iterator itPending;

    if (session->PrePendingOperationList.size() > 0) 
    {
        for (itPending = session->PrePendingOperationList.begin(); itPending!=session->PrePendingOperationList.end(); ++itPending)
        {
            session->DoAddPendingOperation(*itPending);
        }
        accessData->nextWakeTime = session->getWakeUpTime();

        session->PrePendingOperationList.clear();
        session->bChanged = true;
        changePendingFlag = true;
    }

    if (!session->hasOperations()) 
    {
        SessionHash.Delete(sessionKey);
        SessionExpirePool.erase(it);
        delete (*it);

        store.deleteSession(sessionKey);

        inUseMonitor.notifyAll();
        smsc_log_debug(logger,"SessionManager: session closed USR='%d', Address='%s'", 
                       sessionKey.USR, sessionKey.abonentAddr.toString().c_str());
        return;
    }


    accessData->bOpened = false;
    accessData->hasPending = session->hasPending();
    accessData->hasOperations = session->hasOperations();

    if (changePendingFlag) 
    {
        SessionHash.Delete(sessionKey);
        SessionExpirePool.erase(it);

        it = SessionExpirePool.insert(accessData);
        SessionHash.Insert(sessionKey, it);
    }


    //if (session->isChanged())    
    store.updateSession(session);

    awakeEvent.Signal();
    inUseMonitor.notifyAll();

    smsc_log_debug(logger,"SessionManager: session released USR='%d', Address='%s' (pending count=%d)",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str(), session->PendingOperationList.size());
}


void SessionManagerImpl::closeSession(SessionPtr session)
{
    if (!session.Get()) return;
    CSessionKey sessionKey = session->getSessionKey();

    smsc_log_debug(logger,"SessionManager: try to close session USR='%d', Address='%s'",
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str());

    MutexGuard guard(inUseMonitor);

    store.deleteSession(sessionKey);

    CSessionSetIterator * itPtr;

    itPtr = SessionHash.GetPtr(sessionKey);
    if (!itPtr) throw SCAGException("SessionManager: Fatal error 1");
    CSessionSetIterator it = (*itPtr);

    SessionExpirePool.erase(it);
    SessionHash.Delete(sessionKey);
    delete (*it);
    inUseMonitor.notifyAll();
    
    smsc_log_debug(logger,"SessionManager: session closed USR='%d', Address='%s'", 
                   sessionKey.USR, sessionKey.abonentAddr.toString().c_str());
}

int16_t SessionManagerImpl::getLastUSR(Address& address)
{
    int16_t result = 0;

    int * resultPtr = UMRHash.GetPtr(address);
    if (resultPtr) result = (*resultPtr); 
    return result;
}

int16_t SessionManagerImpl::getNewUSR(Address& address)
{
    int16_t result = 1;

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


