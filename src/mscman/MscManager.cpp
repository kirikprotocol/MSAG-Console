
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

    typedef enum { INSERT, UPDATE, DELETE } MscInfoOp;
    struct MscInfoChange : public MscInfo
    {
        MscInfoOp   op;

        MscInfoChange(MscInfoOp _op, const MscInfo& info)
            : MscInfo(info), op(_op) {};
        MscInfoChange(MscInfoOp _op, string msc, bool mLock=false, 
                      bool aLock=false, int fc=0)
            : MscInfo(msc, mLock, aLock, fc), op(_op) {};
        MscInfoChange(const MscInfoChange& info)
            : MscInfo(info), op(info.op) {};
            
        MscInfoChange& operator=(const MscInfoChange& info) {
            op = info.op;
            MscInfo::operator=(info);
            return (*this);
        };
        
    };

    class MscManagerImpl : public MscManager, public Thread
    {
    protected:

        bool                bStarted, bNeedExit;
        Mutex               hashLock;
        Event               flushEvent, exitedEvent;
        
        Hash<MscInfo*>          mscs;
        Array<MscInfoChange>    changes;
        Mutex                   changesLock;
        
        void addChange(const MscInfoChange& change, bool signal=true);

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
        flushEvent.Signal();
        exitedEvent.Wait();
    }
}
int MscManagerImpl::Execute()
{
    do 
    {
        flushEvent.Wait(); // ??? sleepInterval
        // TODO: Implement DbUpdate process here !!!
    } 
    while (!bNeedExit);

    bStarted = false;
    exitedEvent.Signal();
    return 0;
}

void MscManagerImpl::addChange(const MscInfoChange& change, bool signal)
{
    MutexGuard  guard(changesLock);
    changes.Push(change);
    if (signal) flushEvent.Signal();
}

/* ------------------------ MscStatus implementation ------------------------ */

void MscManagerImpl::report(const char* msc, bool status) 
{
    MutexGuard  guard(hashLock);
    
    bool needInsert = false;
    MscInfo* info = mscs.Get(msc);
    if (!info || !mscs.Exists(msc))
    {
        info = new MscInfo(msc);
        mscs.Insert(msc, info);
        needInsert = true;
    }
    
    if (automaticRegistration == false) info->manualLock = true;
    else if (status) info->automaticLock = false;
    else info->automaticLock = (++(info->failureCount) >= failureLimit);

    MscInfoChange change((needInsert) ? INSERT:UPDATE, *info);
    addChange(change);
}
bool MscManagerImpl::check(const char* msc) 
{ 
    MutexGuard  guard(hashLock);

    if (!mscs.Exists(msc)) return true;
    MscInfo* info = mscs.Get(msc);
    return !(info->manualLock || info->automaticLock);
}   

/* ------------------------ MscAdmin implementation ------------------------ */

void MscManagerImpl::registrate(const char* msc) 
{
    MutexGuard  guard(hashLock);

    if (!mscs.Exists(msc)) 
    {
        MscInfo* info = new MscInfo(msc, false, !automaticRegistration, 0);
        mscs.Insert(msc, info);
        MscInfoChange change(INSERT, *info);
        addChange(change);
    }
}
void MscManagerImpl::unregister(const char* msc)
{
    MutexGuard  guard(hashLock);

    if (mscs.Exists(msc)) 
    {
        MscInfo* info = mscs.Get(msc);
        mscs.Delete(msc);
        MscInfoChange change(DELETE, *info);
        if (info) delete info;
        addChange(change);
    }
}
void MscManagerImpl::block(const char* msc)
{
    MutexGuard  guard(hashLock);

    if (mscs.Exists(msc)) 
    {
        MscInfo* info = mscs.Get(msc);
        if (!info->manualLock)
        {
            info->manualLock = true;
            MscInfoChange change(UPDATE, *info);
            addChange(change);
        }
    }
}
void MscManagerImpl::clear(const char* msc) 
{
    MutexGuard  guard(hashLock);

    if (mscs.Exists(msc)) 
    {
        MscInfo* info = mscs.Get(msc);
        if (info->manualLock || info->automaticLock)
        {
            info->manualLock = false;
            info->automaticLock = false;
            MscInfoChange change(UPDATE, *info);
            addChange(change);
        }
    }
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

