
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

    typedef enum { INSERT=1, UPDATE=2, DELETE=3, UNKNOWN=4 } MscInfoOp;
    struct MscInfoChange : public MscInfo
    {
        MscInfoOp   op;

        MscInfoChange()
            : MscInfo(""), op(UNKNOWN) {};
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
    private:
        
        static const char* selectSql;
        static const char* insertSql;
        static const char* updateSql;
        static const char* deleteSql;

    protected:

        Mutex               hashLock;
        bool                bStarted, bNeedExit;
        Event               flushEvent, exitedEvent;

        Connection*         connection;
        
        Hash<MscInfo*>          mscs;
        Array<MscInfoChange>    changes;
        Mutex                   changesLock;
        
        void addChange(const MscInfoChange& change, bool signal=true);
        void processChange(const MscInfoChange& change);

        void init(Manager& config)
            throw(ConfigException, InitException);

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
    init(config);
}
MscManagerImpl::~MscManagerImpl() 
{
    MutexGuard  guard(hashLock);

    // Clean msc info hash here.
    mscs.First();
    char* key; MscInfo* info = 0;
    while (mscs.Next(key, info))
        if (info) delete info;

    if (connection) ds.freeConnection(connection);
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

        MscInfoChange change;
        int result = 1;
        while (result)
        {
            {
                MutexGuard guard(changesLock);
                result = changes.Shift(change);
            }
            processChange(change);
        }
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

const char* MscManagerImpl::selectSql =
"SELECT MSC, M_LOCK, A_LOCK, F_COUNT FROM MSC_LOCK";
const char* MscManagerImpl::insertSql =
"INSERT INTO MSC_LOCK (MSC, M_LOCK, A_LOCK, F_COUNT) "
"VALUES (:MSC, :M_LOCK, :A_LOCK, :F_COUNT)";
const char* MscManagerImpl::updateSql =
"UPDATE MSC_LOCK SET M_LOCK=:M_LOCK, A_LOCK=:A_LOCK, F_COUNT=:F_COUNT "
"WHERE MSC=:MSC";
const char* MscManagerImpl::deleteSql =
"DELETE FROM MSC_LOCK WHERE MSC=:MSC";

void MscManagerImpl::init(Manager& config)
    throw(ConfigException, InitException)
{
    // TODO: Load up extra params from config if needed

    connection = ds.getConnection();
    if (!connection || !connection->isAvailable()) 
    {
        InitException exc("Get connection to DB failed");
        log.error(exc.what());
        throw exc;
    }

    // TODO: Load up msc info here !!!
    Statement* statement = 0;
    ResultSet* rs = 0;
    try
    {
        statement = connection->createStatement(selectSql);
        if (!statement)
            throw InitException("Create statement failed");
        ResultSet* rs = statement->executeQuery();
        if (!rs)
            throw InitException("Get result set failed");

        while (rs->fetchNext())
        {
            const char* mscNum = rs->getString(1);
            const char* mLockStr = rs->getString(2);
            const char* aLockStr = rs->getString(3);
            int fc = rs->getInt32(4);
            bool mLock = (mLockStr && mLockStr[0]=='Y' && mLockStr[1]=='\0');
            bool aLock = (aLockStr && aLockStr[0]=='Y' && aLockStr[1]=='\0');
            MscInfo* info = new MscInfo(mscNum, mLock, aLock, fc);
            
            MutexGuard  guard(hashLock);
            mscs.Insert(mscNum, info);
        }

        if (rs) delete rs;
        if (statement) delete statement;
    }
    catch (Exception& exc)
    {
        if (rs) delete rs;
        if (statement) delete statement;
        log.error(exc.what());
        throw InitException(exc.what());
    }
}

void MscManagerImpl::processChange(const MscInfoChange& change)
{
    __require__(connection);

    __trace2__("Processing change on DB Op=%d "
               "Msc:%s, mLock=%d, aLock=%d fc=%d", change.op, 
               change.mscNum.c_str(), change.manualLock, 
               change.automaticLock, change.failureCount);
    
    Statement* statement = 0; 
    try
    {
        if (!connection->isAvailable()) connection->connect();
        
        switch (change.op)
        {
        case INSERT:
            statement = connection->createStatement(insertSql);
            statement->setString(1, change.mscNum.c_str());
            statement->setString(2, change.manualLock ? "Y":"N");
            statement->setString(3, change.automaticLock ? "Y":"N");
            statement->setInt32 (4, change.failureCount);
            break;
        case UPDATE:
            statement = connection->createStatement(updateSql);
            statement->setString(1, change.manualLock ? "Y":"N");
            statement->setString(2, change.automaticLock ? "Y":"N");
            statement->setInt32 (3, change.failureCount);
            statement->setString(4, change.mscNum.c_str());
            break;
        case DELETE:
            statement = connection->createStatement(deleteSql);
            statement->setString(1, change.mscNum.c_str());
            break;
        default:
            throw Exception("Operation unknown", change.op);
        }

        if (statement) {
            statement->executeUpdate();
            connection->commit();
            delete statement;
        }
    }
    catch (Exception& exc)
    {
        log.error("Process change failed. %s", exc.what());
        if (statement) delete statement;
        connection->rollback();
    }
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

