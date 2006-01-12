
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
            bool hasPending;

            CSessionAccessData() : nextWakeTime(0), bOpened(false) {}
        };

        class XSessionHashFunc {
        public:
            static uint32_t CalcHash(const CSessionKey& key)
            {
                return XAddrHashFunc::CalcHash(key.abonentAddr);
            }
        };


        struct FAccessDataLess 
        {
            bool operator () (const CSessionAccessData* x,const CSessionAccessData* y) const
            {
                return (x->nextWakeTime < y->nextWakeTime);
            }
        };

        typedef std::set<CSessionAccessData*,FAccessDataLess> CSessionSet;
        typedef std::set<CSessionAccessData*>::iterator CSessionSetIterator;
        typedef XHash<CSessionKey,CSessionSetIterator,XSessionHashFunc> CSessionHash;

        typedef XHash<Address,int,XAddrHashFunc> CUMRHash;

        EventMonitor    inUseMonitor;
        CSessionSet     SessionExpirePool;
        CSessionHash    SessionHash;
        CUMRHash        UMRHash;
        Logger *        logger;

//        int16_t         m_nLastUSR;
        Mutex           stopLock;
        Event           awakeEvent, exitEvent;
        bool            bStarted;

        SessionStore    store;
        SessionManagerConfig config;
        
        void Stop();
        bool isStarted();
        int  processExpire();
        int16_t getNewUSR(Address& address);

    public:
        void AddRestoredSession(Session * session);

        void init(const SessionManagerConfig& config);

        SessionManagerImpl() : bStarted(false) , logger(0) {};
        virtual ~SessionManagerImpl();
        
        // SessionManager interface
        virtual SessionPtr getSession(const CSessionKey& sessionKey);
        virtual void releaseSession(SessionPtr session);
        virtual void closeSession(SessionPtr session);

        virtual int Execute();
        virtual void Start();

        virtual void startTimer(CSessionKey key,time_t deadLine);
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

}

SessionManagerImpl::~SessionManagerImpl() 
{ 
    Stop(); 

    CSessionSetIterator it;

    for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it)
    {
        delete (*it);
    }

/*    CSessionKey key;
    CSessionSetIterator * value;
    
    SessionHash.First();
    for (CSessionHash::Iterator it = SessionHash.getIterator(); it.Next(key, value);)
    {
        store.deleteSession(key);
    }     */
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
    store.init(config.dir,SessionManagerCallback,this);

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
        //smsc_log_debug(logger,"SessionManager::----------- ping %d",secs);
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
        if (SessionExpirePool.empty()) return 1;//SessionManagerConfig::DEFAULT_EXPIRE_INTERVAL;

        CSessionSetIterator it;
        for (it = SessionExpirePool.begin();it!=SessionExpirePool.end();++it)
        {
            if ((!(*it)->bOpened)&&((*it)->hasPending)) break;
        }

          time_t now;
        time(&now);
        int iPeriod;

        if (it == SessionExpirePool.end())
        {
            it = SessionExpirePool.begin();
            iPeriod = ((*it)->nextWakeTime - now);
            if (iPeriod <= 0) return 1;
            else return iPeriod;
        }

        iPeriod = (*it)->nextWakeTime - now;
        if (iPeriod > 0) return iPeriod;

        //expire pending operations
        SessionPtr session = store.getSession((*it)->SessionKey);

        while ((iPeriod <= 0)&&(session->hasPending()))
        {
            session->expirePendingOperation();
            (*it)->hasPending = session->hasPending();
            (*it)->nextWakeTime = session->getWakeUpTime();

            iPeriod = (*it)->nextWakeTime - now;
        }

        // Session expired
        //smsc_log_debug(logger,"SessionManager: ********* %d ******** %d **** %d",session->hasOperations(),iPeriod,session->hasPending());

        if (!session->hasOperations()) 
        {
            SessionHash.Delete((*it)->SessionKey);
            SessionExpirePool.erase(it);
            store.deleteSession((*it)->SessionKey);
            delete (*it);
        } 
        else if (iPeriod > 0) return iPeriod;

    }
}


SessionPtr SessionManagerImpl::getSession(const CSessionKey& sessionKey)
{

    SessionPtr session;

    MutexGuard guard(inUseMonitor);

    CSessionSetIterator it;

    if (!SessionHash.Exists(sessionKey)) return session; 

    it = SessionHash.Get(sessionKey);

    while ((*it)->bOpened)
    {
        inUseMonitor.wait();
        if (!SessionHash.Exists(sessionKey)) return session;
    }                                      

    (*it)->bOpened = true;
    return store.getSession(sessionKey);
}


SessionPtr SessionManagerImpl::newSession(CSessionKey& sessionKey)
{
    SessionPtr session;
    CSessionAccessData * accessData = 0;

    MutexGuard guard(inUseMonitor);

    sessionKey.USR = getNewUSR(sessionKey.abonentAddr);

    session = store.newSession(sessionKey);
    session->setOwner(this);

    time_t time = session->getWakeUpTime();

    accessData = new CSessionAccessData();

    accessData->bOpened = true;
    accessData->nextWakeTime = time;
    accessData->hasPending = session->hasPending();

    CSessionSetIterator it;
    std::pair<CSessionSetIterator, bool> pr;

    pr = SessionExpirePool.insert(accessData);
    SessionHash.Insert(sessionKey,pr.first);
    return session;
}



void SessionManagerImpl::releaseSession(SessionPtr session)
{
    if (!session.Get()) return;
    CSessionKey sessionKey = session->getSessionKey();

    MutexGuard guard(inUseMonitor);
    if (session->isChanged()) store.updateSession(session);
    
    CSessionSetIterator it;

    if (!SessionHash.Exists(sessionKey)) throw SCAGException("SessionManager: Fatal error 0");
    it = SessionHash.Get(sessionKey);

    if (!session->hasOperations()) 
    {
        SessionHash.Delete(sessionKey);
        SessionExpirePool.erase(it);
        delete (*it);

        store.deleteSession(sessionKey);
    }
    else
    {
        (*it)->bOpened = false;
        (*it)->hasPending = session->hasPending();
    }



    inUseMonitor.notifyAll();
    smsc_log_debug(logger,"SessionManager: session released");
}


void SessionManagerImpl::closeSession(SessionPtr session)
{
    if (!session.Get()) return;
    CSessionKey sessionKey = session->getSessionKey();

    MutexGuard guard(inUseMonitor);
    store.deleteSession(sessionKey);

    CSessionSetIterator it;

    if (!SessionHash.Exists(sessionKey)) throw SCAGException("SessionManager: Fatal error 1");
    it = SessionHash.Get(sessionKey);

    SessionExpirePool.erase(it);
    SessionHash.Delete(sessionKey);
    delete (*it);

    smsc_log_debug(logger,"SessionManager: session closed");
    inUseMonitor.notifyAll();
}

void SessionManagerImpl::startTimer(CSessionKey key,time_t deadLine)
{
    MutexGuard guard(inUseMonitor);


    if (!SessionHash.Exists(key)) throw SCAGException("SessionManager: Fatal error 2");

    CSessionSetIterator it;
    it = SessionHash.Get(key);
/*
    while (it->bOpened) 
    {
        inUseMonitor.wait();
        if (!SessionHash.Exists(key)) return;
        it = SessionHash.Get(key);
    }
*/
    (*it)->nextWakeTime = deadLine;
}


int16_t SessionManagerImpl::getNewUSR(Address& address)
{
    //TODO: Develop expire mehanism

    int16_t result = 0;
    if (UMRHash.Exists(address)) 
    {
        result = UMRHash.Get(address)++;
        UMRHash[address] = result;
        return result;
    }

    UMRHash.Insert(address,result);
    return result;
}


}}


