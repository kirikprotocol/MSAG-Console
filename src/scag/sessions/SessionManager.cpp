
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

namespace scag { namespace sessions 
{
    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;
    using namespace smsc::core::buffers;

    class SessionManagerImpl : public SessionManager, public Thread
    {
        struct CSessionAccessData
        {
            CSessionKey SessionKey;
            time_t lastAccess;
            bool bOpened;

            CSessionAccessData() : lastAccess(0), bOpened(false) {}
        };

        class XHashFunc{
        public:
          template <class T> static inline unsigned int CalcHash(T key)
          {
            return key.USR;
          }
        };


        typedef std::list<CSessionAccessData> CSessionList;
        typedef std::list<CSessionAccessData>::iterator CSLIterator;
        typedef XHash<CSessionKey,CSLIterator,XHashFunc> CSessionHash;

        Mutex           Lock;
        CSessionList    SessionExpirePool;
        CSessionHash    SessionHash;
        
        Mutex           stopLock;
        Event           awakeEvent, exitEvent;
        bool            bStarted;

        SessionStore    store;
        SessionManagerConfig config;
        
        void Stop();
        bool isStarted()
        int  processExpire();

    public:

        void init(const SessionManagerConfig& config);

        SessionManagerImpl() : bStarted(false) {};
        virtual ~SessionManagerImpl() { Stop(); }
        
        // SessionManager interface
        virtual Session* getSession(const SCAGCommand& command);
        virtual void releaseSession(const Session* session);
        virtual void closeSession(const Session* session);

        virtual int Execute();
        virtual void Start();
    };


const time_t SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL = 3600;

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
    MutexGuard guard(stopLock);
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
}
int SessionManagerImpl::Execute()
{
    while (isStarted())
    {
        int secs = processExpire();
        awakeEvent.Wait(secs*1000);
    }
    exitEvent.Signal();
    return 0;
}

/*
int SessionManagerImpl::processExpire()
{
    MutexGuard guard(Lock);
    
    while (1) 
    {
        if (SessionExpirePool.empty()) return m_uiExpirationTime;

        CSLIterator it;
        for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it)
            if (!it->bOpened) break;

        if (it == SessionExpirePool.end()) return m_uiExpirationTime;

        time_t now;
        time(&now);
        
        int iPeriod = it->lastAccess + m_uiExpirationTime - now;
        if (iPeriod > 0) return iPeriod;

        // Session expired
        SessionHash.Delete(it->SessionKey);
        SessionExpirePool.erase(it);
        // TODO: Remove from storage
    }
}
*/

time_t SessionManagerImpl::processExpire()
{
    if (SessionExpirePool.empty()) return config.expireInterval;

    time_t timeToNextExpire = 0; // TODO: get current session's expiration time ?

    CSLIterator it;
    for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it)
    {
        // TODO: calculate next session's expiration time
        if (it->isInUse()) continue;
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

Session* SessionManagerImpl::getSession(const SCAGCommand& command)
{
    Session*    session = 0;
    CSessionKey sessionKey; // TODO: obtain from SCAGCommand somehow

    MutexGuard guard(inUseMonitor);
    while (inUse.Exists(sessionKey)) inUseMonitor.Wait();
    
    session = store.getSession(sessionKey);
    if (session) {
        // TODO: add session to expire pool & sessionsHash
        inUse.Insert(sessionKey, session);
    }

    return session; // TODO: possible SessionGuard
}
void SessionManagerImpl::releaseSession(const Session* session)
{
    if (!session) return;
    CSessionKey sessionKey = session->getSessionKey();

    MutexGuard guard(inUseMonitor);
    if (session->isChanged) store.updateSesion(session);
    inUse.Delete(sessionKey);
    inUseMonitor.NotifyAll();
}
void SessionManagerImpl::closeSession(const Session* session)
{
    if (!session) return;
    CSessionKey sessionKey = session->getSessionKey();

    MutexGuard guard(inUseMonitor);
    // TODO: remove session from expire pool & sessionsHash
    store.deleteSesion(sessionKey);
    inUse.Delete(sessionKey);
    inUseMonitor.NotifyAll();
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


