
#include <inttypes.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/buffers/XHash.hpp>

#include <scag/util/singleton/Singleton.h>
#include <list>

#include <unistd.h>
#include <time.h>

#include "SessionStore.h"
#include "SessionManager.h"
#include "Session.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/util/sms/HashUtil.h"

#include <iostream>

namespace scag { namespace sessions 
{
    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;
    using namespace scag::exceptions;
    using namespace smsc::core::buffers;
    using namespace scag::util::sms;

    class SessionManagerImpl : public SessionManager, public Thread, public SessionOwner
    {
        struct CSessionAccessData
        {
            CSessionKey SessionKey;
            time_t nextWakeTime;
            bool bOpened;

            CSessionAccessData() : nextWakeTime(0), bOpened(false) {}
        };


        class XSessionHashFunc {
        public:
            static uint32_t CalcHash(const CSessionKey& key)
            {
                return XAddrHashFunc::CalcHash(key.abonentAddr);
            }
        };

        typedef std::list<CSessionAccessData> CSessionList;
        typedef std::list<CSessionAccessData>::iterator CSLIterator;
        typedef XHash<CSessionKey,CSLIterator,XSessionHashFunc> CSessionHash;

        EventMonitor    inUseMonitor;
        CSessionList    SessionExpirePool;
        CSessionHash    SessionHash;
        
        Mutex           stopLock;
        Event           awakeEvent, exitEvent;
        bool            bStarted;

        SessionStore    store;
        SessionManagerConfig config;
        
        void Stop();
        bool isStarted();
        int  processExpire();

    public:

        void init(const SessionManagerConfig& config);

        SessionManagerImpl() : bStarted(false) {};
        virtual ~SessionManagerImpl() { Stop(); }
        
        // SessionManager interface
        virtual Session* getSession(const SCAGCommand& command);
        virtual void releaseSession(Session* session);
        virtual void closeSession(const Session* session);

        virtual int Execute();
        virtual void Start();

        virtual void startTimer(CSessionKey key,time_t deadLine);
        Session * NewSession(CSessionKey sessionKey);

    };


const time_t SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL = 60;

// ################## Singleton related issues ##################

static bool  bSessionManagerInited = false;
static Mutex initSessionManagerLock;

inline unsigned GetLongevity(SessionManager*) { return 6; } // ? Move upper ? 
typedef SingletonHolder<SessionManagerImpl> SingleSM;

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
    store.init(config.dir);
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
    std::cout<<"SessionManager::stop" << std::endl;
}
int SessionManagerImpl::Execute()
{
    std::cout<<"SessionManager::start executing" << std::endl;

    while (isStarted())
    {
        int secs = processExpire();
        awakeEvent.Wait(secs*1000);
    }
    exitEvent.Signal();
    std::cout<<"SessionManager::stop executing" << std::endl;
    return 0;
}


int SessionManagerImpl::processExpire()
{
    MutexGuard guard(inUseMonitor);

    while (1) 
    {
        if (SessionExpirePool.empty()) return SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;

        CSLIterator it;
        for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it)
            if (!it->bOpened) break;

        time_t now;
        time(&now);
        int iPeriod;

        if (it == SessionExpirePool.end())
        {
            it == SessionExpirePool.begin();
            iPeriod = (it->nextWakeTime - now);
            if (iPeriod <= 0) return 1;
            else return iPeriod;
        }

        iPeriod = it->nextWakeTime - now;
        if (iPeriod > 0) return iPeriod;

        // Session expired
        SessionHash.Delete(it->SessionKey);
        SessionExpirePool.erase(it);
        store.deleteSesion(it->SessionKey);
    }
}


/*
time_t SessionManagerImpl::processExpire()
{
    if (SessionExpirePool.empty()) return config.expireInterval;

    time_t timeToNextExpire = 0; // TODO: get current session's expiration time ?

    CSLIterator it;
    for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it)
    {
        // TODO: calculate next session's expiration time
        
        if (it->isInUse()) 
            {
            continue;
        }
        CSessionKey sessionKey = it->getSessionKey();

        MutexGuard guard(inUseMonitor);
        if (inUse.Exists(sessionKey)) {
            session->Expire();
            store.deleteSesion(sessionKey);
            inUseMonitor.NotifyAll();
        }
    }
    if (it == SessionExpirePool.end()) return config.expireInterval;
    
    return timeToNextExpire;    
}

  */

Session * SessionManagerImpl::NewSession(CSessionKey sessionKey)
{
    Session * session = 0;
    store.newSesion(sessionKey);

    session = store.getSession(sessionKey);

    time_t time = session->getWakeUpTime();

    CSessionAccessData accessData;
    accessData.nextWakeTime = session->getWakeUpTime();
    if (session->getWakeUpTime() == 0) return 0;

    CSLIterator it;
    for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it) 
    {
        if (time < it->nextWakeTime)
        {
            SessionExpirePool.insert(it,accessData);
            SessionHash.Insert(sessionKey,it);
            return session;
        }
    }
    SessionExpirePool.insert(it,accessData);
    it = SessionExpirePool.end();
    --it;
    SessionHash.Insert(sessionKey,it);

    return session;
}

Session* SessionManagerImpl::getSession(const SCAGCommand& command)
{
    Session*    session = 0;
    CSessionKey sessionKey; 

    sessionKey.abonentAddr = command.getAbonentAddr();
    sessionKey.USR = command.getUMR();

    MutexGuard guard(inUseMonitor);

    CSLIterator it;

    if (!SessionHash.Exists(sessionKey)) return NewSession(sessionKey);

    it = SessionHash.Get(sessionKey);
    while (it->bOpened)
    {
        inUseMonitor.wait();
        if (!SessionHash.Exists(sessionKey)) return NewSession(sessionKey);
        it = SessionHash.Get(sessionKey);
    }                                      
               
    return store.getSession(sessionKey);
}


void SessionManagerImpl::releaseSession(Session* session)
{
    if (!session) return;
    CSessionKey sessionKey = session->getSessionKey();

    MutexGuard guard(inUseMonitor);
    if (session->isChanged()) store.updateSesion(session);
    session->releaseOperation();

    CSLIterator it;

    if (!SessionHash.Exists(sessionKey)) throw SCAGException("SessionManager: Fatal error 0");
    it = SessionHash.Get(sessionKey);
    it->bOpened = false;

    inUseMonitor.notifyAll();
}


void SessionManagerImpl::closeSession(const Session* session)
{
    if (!session) return;
    CSessionKey sessionKey = session->getSessionKey();

    MutexGuard guard(inUseMonitor);
    store.deleteSesion(sessionKey);

    CSLIterator it;

    if (!SessionHash.Exists(sessionKey)) throw SCAGException("SessionManager: Fatal error 1");
    it = SessionHash.Get(sessionKey);

    SessionExpirePool.erase(it);
    SessionHash.Delete(sessionKey);

    inUseMonitor.notifyAll();
}

void SessionManagerImpl::startTimer(CSessionKey key,time_t deadLine)
{
    MutexGuard guard(inUseMonitor);

    if (!SessionHash.Exists(key)) throw SCAGException("SessionManager: Fatal error 2");

    CSLIterator it;
    it = SessionHash.Get(key);
    while (it->bOpened) 
    {
        inUseMonitor.wait();
        if (!SessionHash.Exists(key)) return;
        it = SessionHash.Get(key);
    }
    if (deadLine == 0) 
    {
        time_t now;
        time(&now);
        deadLine = SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL + now;
    }
    it->nextWakeTime = deadLine;
}

/*
Session* SessionManagerImpl::getSession(const SCAGCommand& command)
{
    CSessionKey SessionKey; 
    
    SessionKey.abonentAddr = command.getAbonentAddr();
    SessionKey.USR = command.getUMR(); // TODO: MMS & WAP commands has no UMR field !!!

    CSessionAccessData data;
    CSLIterator it;
    bool bSessionExists;

    do
    {
        Lock.Lock();

        data.bOpened = false;
        bSessionExists = false;

        if (SessionHash.Exists(SessionKey)) 
        {
            it = SessionHash.Get(SessionKey);
            data = *it;
            bSessionExists = true;
        }

        if (!data.bOpened) break;

        Lock.Unlock();
        timespec tt;
        tt.tv_sec = 0;
        tt.tv_nsec = 1000;
        nanosleep(&tt,0);
    }
    while (data.bOpened); 
        

    Session * session = GetSessionFromStorage(SessionKey);
    if (!session) 
    {
        Lock.Unlock();
        throw Exception("SessionManager: Invalid session returned from sorage");
    }

    if (bSessionExists == true) 
    {
        data = *it;
        data.bOpened = true;
        (*it) = data;
    } 
    else
    {
        time(&data.lastAccess);
        data.SessionKey = SessionKey;
        data.bOpened = true;

        SessionExpirePool.push_back(data);
        it = SessionExpirePool.end();
        SessionHash.Insert(SessionKey,--it);
    }

    Lock.Unlock();
    return session;
}

void SessionManagerImpl::releaseSession(const Session* session)
{
    MutexGuard guard(Lock);
    if (!session) return;

    if (SessionHash.Exists(session->getSessionKey()))
    {

        CSLIterator it = SessionHash.Get(session->getSessionKey());

        CSessionAccessData data = *it;

        time(&data.lastAccess);
        data.bOpened = false;

        //TODO: change session lastAccessTime
        SessionExpirePool.erase(it);
        SessionExpirePool.push_back(data);

        it = SessionExpirePool.end();
        SessionHash.Insert(session->getSessionKey(),--it);
    }
}

void SessionManagerImpl::closeSession(const Session* session)
{
    MutexGuard guard(Lock);
    if (!session) return;

    if (SessionHash.Exists(session->getSessionKey()))
    {
        CSLIterator it = SessionHash.Get(session->getSessionKey());
        SessionExpirePool.erase(it);
        SessionHash.Delete(session->getSessionKey());
        //TODO: close session
    }
}*/


}}


