
#include <inttypes.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/buffers/XHash.hpp>

#include <scag/util/singleton/Singleton.h>
#include <list>

#include <unistd.h>
#include <time.h>

#include "Session.h"
#include "SessionManager.h"
#include "scag/exc/SCAGExceptions.h"


namespace scag { namespace sessions 
{
    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;
    using namespace smsc::core::buffers;
    using namespace scag::exceptions;

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
              uint32_t retval=key.abonentAddr.type^key.abonentAddr.plan;
              for(int i=0;i<key.abonentAddr.length;i++)
              {
                  retval=retval*10+(key.abonentAddr.value[i]-'0');
              }
              return retval;
          }
        };


        typedef std::list<CSessionAccessData> CSessionList;
        typedef std::list<CSessionAccessData>::iterator CSLIterator;
        typedef XHash<CSessionKey,CSLIterator,XHashFunc> CSessionHash;

        Mutex Lock;
        CSessionList SessionExpirePool;
        CSessionHash SessionHash;
        unsigned int m_uiWaitSeconds;
        unsigned int m_uiExpirationTime;
        bool bStopFlag;

        void Stop();
        int ProcessExpire();

    public:

        void init(const std::string& dir);

        SessionManagerImpl() : bStopFlag(false), m_uiWaitSeconds(0), m_uiExpirationTime(3600) {};
        virtual ~SessionManagerImpl() { Stop(); }
        
        // SessionManager interface
        virtual Session* getSession(const SCAGCommand& command);
        virtual void releaseSession(const Session* session);
        virtual void closeSession(const Session* session);

        virtual int Execute();
        virtual void Start();
    };


    // ################## Singleton related issues ##################

static bool  bSessionManagerInited = false;
static Mutex initSessionManagerLock;

// ? Move upper ? 
// ? Use SessionManagerImpl ?
inline unsigned GetLongevity(SessionManager*) { return 6; } 
typedef SingletonHolder<SessionManagerImpl> SingleSM;

void SessionManager::Init(const std::string& dir)
{
    if (!bSessionManagerInited)
    {
        MutexGuard guard(initSessionManagerLock);
        if (!bSessionManagerInited) {
            SessionManagerImpl& sm = SingleSM::Instance();
            sm.init(dir); sm.Start();
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

void SessionManagerImpl::init(const std::string& dir) // possible throws exceptions
{

}
void SessionManagerImpl::Start()
{
    bStopFlag = false;
    Thread::Start();
    // TODO: implement thread start (flag, mutex & etc)
}
void SessionManagerImpl::Stop()
{
    bStopFlag = true;
}


int SessionManagerImpl::ProcessExpire()
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



int SessionManagerImpl::Execute()
{
    for (;!bStopFlag;)
    {
        timespec tt;
        tt.tv_sec = ProcessExpire();
        nanosleep(&tt,0);
    }

    return 0;
}

Session* SessionManagerImpl::getSession(const SCAGCommand& command)
{
    CSessionKey SessionKey; 
    
    SessionKey.abonentAddr = command.getAbonentAddr();
    SessionKey.UMR = command.getUMR();

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
        
    //TODO : GetSessionFromStorage(SessionKey);
    Session * session;
    if (!session) 
    {
        Lock.Unlock();
        throw SCAGException("SessionManager: Invalid session returned from sorage");
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
}


}}


