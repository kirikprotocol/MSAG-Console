
#include <inttypes.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/buffers/XHash.hpp>

#include <scag/util/singleton/Singleton.h>
#include <list>

#include <unistd.h>
#include <time.h>
#include <logger/Logger.h>

#include "SessionStore.h"
#include "SessionManager.h"
#include "scag/exc/SCAGExceptions.h"
#include "scag/util/sms/HashUtil.h"
#include "scag/re/CommandBrige.h"


namespace scag { namespace sessions 
{
    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;
    using namespace scag::exceptions;
    using namespace smsc::core::buffers;
    using namespace scag::util::sms;
    using namespace re;
    using scag::config::SessionManagerConfig;

    using smsc::logger::Logger;

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
        Logger *        logger;

        int16_t         m_nLastUSR;
        Mutex           stopLock;
        Event           awakeEvent, exitEvent;
        bool            bStarted;

        SessionStore    store;
        SessionManagerConfig config;
        
        void Stop();
        bool isStarted();
        int  processExpire();
        int16_t getNewUSR() {return ++m_nLastUSR; }
    public:

        void init(const SessionManagerConfig& config);

        SessionManagerImpl() : bStarted(false) , m_nLastUSR(0),logger(0) {};
        virtual ~SessionManagerImpl() { Stop(); }
        
        // SessionManager interface
        virtual Session* getSession(const CSessionKey& sessionKey);
        virtual void releaseSession(Session* session);
        virtual void closeSession(const Session* session);

        virtual int Execute();
        virtual void Start();

        virtual void startTimer(CSessionKey key,time_t deadLine);
        virtual Session * newSession(CSessionKey& sessionKey);

    };


//const time_t SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL = 60;

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

    if (!logger)
      logger = Logger::getInstance("scag.re.SessionManager");

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
        awakeEvent.Wait(secs*1000);
    }
    smsc_log_info(logger,"SessionManager::stop executing");
    exitEvent.Signal();
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
        store.deleteSession(it->SessionKey);
    }
}


Session* SessionManagerImpl::getSession(const CSessionKey& sessionKey)
{

    Session*    session = 0;
    //CSessionKey sessionKey; 

    //sessionKey.abonentAddr = CommandBrige::getAbonentAddr(command);
    //sessionKey.USR = CommandBrige::getUMR(command);

    MutexGuard guard(inUseMonitor);

    CSLIterator it;

    if (!SessionHash.Exists(sessionKey)) return 0; 

    it = SessionHash.Get(sessionKey);

    while (it->bOpened)
    {
        inUseMonitor.wait();
        if (!SessionHash.Exists(sessionKey)) return 0;
    }                                      

    it->bOpened = true;
    return store.getSession(sessionKey);
}


Session * SessionManagerImpl::newSession(CSessionKey& sessionKey)
{
    Session * session;
    CSessionAccessData accessData;

    MutexGuard guard(inUseMonitor);

    sessionKey.USR = getNewUSR();

    store.newSession(sessionKey);
    session = store.getSession(sessionKey);
    session->setOwner(this);

    time_t time = session->getWakeUpTime();

    accessData.bOpened = true;
    accessData.nextWakeTime = time;

    CSLIterator it;
    for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it) 
    {
        if (time < it->nextWakeTime)
        {
            it->bOpened = true;
            CSLIterator _it = SessionExpirePool.insert(it,accessData);
            SessionHash.Insert(sessionKey,_it);
            return session;
        }
    }
    it = SessionExpirePool.insert(SessionExpirePool.end(),accessData);
    SessionHash.Insert(sessionKey,it);

    return session;
}



void SessionManagerImpl::releaseSession(Session* session)
{
    if (!session) return;
    CSessionKey sessionKey = session->getSessionKey();

    MutexGuard guard(inUseMonitor);
    if (session->isChanged()) store.updateSession(session);
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
    store.deleteSession(sessionKey);

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
/*
    while (it->bOpened) 
    {
        inUseMonitor.wait();
        if (!SessionHash.Exists(key)) return;
        it = SessionHash.Get(key);
    }
*/
    it->nextWakeTime = deadLine;
}

}}


