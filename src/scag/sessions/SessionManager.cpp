
#include <inttypes.h>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/synchronization/EventMonitor.hpp>

#include <scag/util/singleton/Singleton.h>

#include "SessionManager.h"

namespace scag { namespace sessions 
{
    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using namespace scag::util::singleton;

    class SessionManagerImpl : public SessionManager, public Thread
    {
    private:

        // TODO: add SessionStore here

        void Stop();
        
    public:

        void init(const std::string& dir);

        SessionManagerImpl() : SessionManager(), Thread() {};
        virtual ~SessionManagerImpl() { Stop(); }
        
        // SessionManager interface
        virtual Session* getSession(SCAGCommand* command);
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

SessionManager::SessionManager(const SessionManager& sm)
{
    throw std::runtime_error("SessionManager couldn't be copied");
}
SessionManager& SessionManager::operator=(const SessionManager& sm)
{
    throw std::runtime_error("SessionManager couldn't be copied");
}


// ################ TODO: Actual SessionManager Implementation follows ################ 

void SessionManagerImpl::init(const std::string& dir) // possible throws exceptions
{

}
void SessionManagerImpl::Start()
{
    // TODO: implement thread start (flag, mutex & etc)
    Thread::Start();
}
void SessionManagerImpl::Stop()
{
    // TODO: implement stop of expiring mechanism (used on program exit)
}
int SessionManagerImpl::Execute()
{
    // TODO: implement expiring mechanism
    return 0;
}


}}


