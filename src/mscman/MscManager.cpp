
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/Array.hpp>
#include <core/threads/Thread.hpp>

#include <db/DataSource.h>

#include "MscManager.h"

namespace smsc { namespace mscman
{
    using std::string;

    using namespace smsc::db;
    using namespace core::threads;
    using namespace core::buffers;
    using namespace core::synchronization;
    
    const char* MSCMAN_NO_INSTANCE    = "MscManager wasn't instantiated";
    const char* MSCMAN_INSTANCE_EXIST = "MscManager already instantiated";
    const char* MSCMAN_NOT_STARTED    = "MscManager wasn't started";

    class MscManagerImpl : public MscManager, public Thread
    {
    protected:

        bool                bStarted, bNeedExit;
        Mutex               hashLock;
        Event               updateEvent, exitedEvent;
        int                 sleepInterval;
        
        Hash<MscInfo *>     mscs;

    public:

        MscManagerImpl(DataSource& ds, Manager& config)
            throw(ConfigException, InitException);
        virtual ~MscManagerImpl();

        void Start();
        void Stop();
        virtual int Execute();

        /* MscStatus implementation */
        virtual void report(const char* msc, bool status);
        virtual bool check(const char* msc);

        /* MscAdmin implementation */
        virtual void registrate(const char* msc);
        virtual void unregister(const char* msc);
        virtual void block(const char* msc);
        virtual void clear(const char* msc);
        virtual Array<MscInfo> list();
    };

Mutex MscManager::startupLock;
MscManager* MscManager::instance = 0;
log4cpp::Category& MscManager::log = 
    Logger::getCategory("smsc.mscman.MscManager");

MscManager::MscManager(DataSource& _ds, Manager& config)
    throw(ConfigException)
        : MscStatus(), MscAdmin(), ds(_ds)
{
    // TODO: Loadup parameters from config !
    automaticRegistration = true;
    failureLimit = 100;
}
MscManager::~MscManager()
{
    // Do nothing ?
}

void MscManager::startup(DataSource& ds, Manager& config)
    throw(ConfigException, InitException)
{
    MutexGuard guard(startupLock);
    if (instance) throw InitException(MSCMAN_INSTANCE_EXIST);
    instance = new MscManagerImpl(ds, config);
    ((MscManagerImpl *)instance)->Start();
}
void MscManager::shutdown()
{
    MutexGuard guard(startupLock);
    if (instance) 
    {
        ((MscManagerImpl *)instance)->Stop();   
        delete instance;
        instance = 0;
    }
}
MscManager& MscManager::getInstance() 
    throw(InitException) 
{
    MutexGuard guard(startupLock);
    if (!instance) throw InitException(MSCMAN_NO_INSTANCE);
    return (*instance); 
}


/* ---------------------- MscManager implementation ---------------------- */ 
MscManagerImpl::MscManagerImpl(DataSource& _ds, Manager& config)
    throw(ConfigException, InitException)
        : MscManager(_ds, config), Thread(), 
            bStarted(false), bNeedExit(false) 
{
}
MscManagerImpl::~MscManagerImpl() 
{
}
void MscManagerImpl::Start() 
{
    if (!bStarted)
    {
        bNeedExit = false;
        Thread::Start();
        bStarted = true;
    }
}
void MscManagerImpl::Stop()
{
    if (bStarted)
    {
        bNeedExit = true;
        updateEvent.Signal();
        exitedEvent.Wait();
    }
}
int MscManagerImpl::Execute()
{
    do 
    {
        updateEvent.Wait(sleepInterval); // ??? sleepInterval
        // TODO: Implement DbUpdate process here !!!
    } 
    while (!bNeedExit);

    bStarted = false;
    exitedEvent.Signal();
    return 0;
}

/* ------------------------ MscStatus implementation ------------------------ */
void MscManagerImpl::report(const char* msc, bool status) 
{
    // TODO: implement it !!!

    MutexGuard  guard(hashLock);

    MscInfo* info = mscs.Get(msc);
    if (!info) {
        info = new MscInfo(msc);
        mscs.Insert(msc, info);
    }
    
    if (automaticRegistration == false) info->manualLock = true;
    else if (status) info->automaticLock = false;
    else info->automaticLock = (++(info->failureCount) >= failureLimit);
    
    updateEvent.Signal();
}
bool MscManagerImpl::check(const char* msc) 
{ 
    // TODO: implement it !!!

    MutexGuard  guard(hashLock);

    if (!mscs.Exists(msc)) return true;
    MscInfo* info = mscs.Get(msc);
    return !(info->manualLock || info->automaticLock);
}   

/* ------------------------ MscAdmin implementation ------------------------ */
void MscManagerImpl::registrate(const char* msc) 
{
    // TODO: implement it !!!

    MutexGuard  guard(hashLock);

    if (!mscs.Exists(msc)) {
        MscInfo* info = new MscInfo(msc, false, !automaticRegistration, 0);
        mscs.Insert(msc, info);
        updateEvent.Signal();
    }
}
void MscManagerImpl::unregister(const char* msc)
{
    // TODO: implement it !!!

    if (mscs.Exists(msc)) {

    }
}
void MscManagerImpl::block(const char* msc)
{
    // TODO: implement it !!!
}
void MscManagerImpl::clear(const char* msc) 
{
    // TODO: implement it !!!
}
Array<MscInfo> MscManagerImpl::list() 
{
    MutexGuard  guard(hashLock);

    Array<MscInfo> list; 
    
    char* key; MscInfo* info = 0;
    while (mscs.Next(key, info)) 
        if (info) list.Push(*info);
    
    return list;
}


}}

