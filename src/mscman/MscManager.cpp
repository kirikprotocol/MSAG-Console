#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/Array.hpp>
#include <core/threads/Thread.hpp>

#include "core/buffers/FixedRecordFile.hpp"

#include "MscManager.h"

#include "cluster/Interconnect.h"

namespace smsc {
namespace mscman {
    using std::string;

    using namespace core::threads;
    using namespace core::buffers;
    using namespace core::synchronization;

    const char* MSCMAN_NO_INSTANCE    = "MscManager wasn't instantiated";
    const char* MSCMAN_INSTANCE_EXIST = "MscManager already instantiated";
    const char* MSCMAN_NOT_STARTED    = "MscManager wasn't started";

    typedef enum
    {
      MSCOP_UNKNOWN,
      MSCOP_INSERT,
      MSCOP_DELETE,
      MSCOP_BLOCK,
      MSCOP_REPORT,
      MSCOP_CLEAR
    } MscInfoOp;

    static const char* opNames[]=
    {
      "Unknown",
      "Insert",
      "Delete",
      "Block",
      "Report",
      "Clear"
    };

    static const char sig[]="SMSCMSCM";
    static uint32_t ver=0x00010000;

    struct MscInfoChange
    {

        MscInfoOp   op;
        char        mscNum[22];
        int         data;

        MscInfoChange(): op(MSCOP_UNKNOWN)
        {
          memset(mscNum,0,sizeof(mscNum));
        }

        MscInfoChange(MscInfoOp argOp,const char* argMsc,int argData=0)
            : op(argOp),data(argData)
        {
          if(argMsc)
          {
            strncpy(mscNum,argMsc,sizeof(mscNum));
            mscNum[sizeof(mscNum)-1]=0;
          }
        }
        MscInfoChange(const MscInfoChange& that)
        {
          op=that.op;
          strcpy(mscNum,that.mscNum);
          data=that.data;
        }
    };

    class MscManagerImpl : public MscManager, public Thread
    {
    protected:

        EventMonitor mon;
        bool         isStopping;

        Hash<MscInfo*>          mscs;
        Array<MscInfoChange>    changes;
        Mutex                   changesLock;

        FixedRecordFile<MscInfo> storeFile;

        void createFile(const char* fileName);

        void init(Manager& config)
            throw(ConfigException, InitException);

        void insertToFile(MscInfo& mscinfo);
        void updateFile(MscInfo& mscinfo);
        void removeFromFile(MscInfo& mscinfo);

    public:

        MscManagerImpl(Manager& config)
            throw(ConfigException, InitException);
        virtual ~MscManagerImpl();

        void Stop();
        virtual int Execute();

        void processChange(const MscInfoChange& change);


        /* MscStatus implementation */
        virtual void report(const char* msc, bool status);
        virtual MscState check(const char* msc);

        /* MscAdmin implementation */
        virtual void registrate(const char* msc);
        virtual void unregister(const char* msc);
        virtual void block(const char* msc);
        virtual void clear(const char* msc);
        virtual Array<MscInfo> list();
    };

Mutex MscManager::startupLock;
MscManager* MscManager::instance = 0;
smsc::logger::Logger *MscManager::log = 0;

MscManager::MscManager(Manager& config)
    throw(ConfigException)
        : MscStatus(), MscAdmin()
{
    automaticRegistration = config.getBool("MscManager.automaticRegistration");
    failureLimit = config.getInt("MscManager.failureLimit");
    try{
      singleAttemptTimeout = config.getInt("MscManager.singleAttemptTimeout");
    }catch(...)
    {
      singleAttemptTimeout=3600;
      __warning2__("MscManager.singleAttemptTimeout not found in config, using default:%d",singleAttemptTimeout);
    }
}
MscManager::~MscManager()
{
    // Do nothing ?
}

void MscManager::startup(Manager& config)
    throw(ConfigException, InitException)
{
    MutexGuard guard(startupLock);
    if (!log) log = Logger::getInstance("smsc.msc");
    if (instance) throw InitException(MSCMAN_INSTANCE_EXIST);
    instance = new MscManagerImpl(config);
    ((MscManagerImpl *)instance)->Start();
}
void MscManager::shutdown()
{
    MutexGuard guard(startupLock);
    if (instance)
    {
        ((MscManagerImpl *)instance)->Stop();
        ((MscManagerImpl *)instance)->WaitFor();
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
MscManagerImpl::MscManagerImpl(Manager& config)
    throw(ConfigException, InitException)
        : MscManager(config), Thread(),
            isStopping(false),storeFile(sig,ver)
{
    init(config);
}
MscManagerImpl::~MscManagerImpl()
{
    MutexGuard  guard(mon);

    // Clean msc info hash here.
    mscs.First();
    char* key; MscInfo* info = 0;
    while (mscs.Next(key, info))
        if (info) delete info;

}
void MscManagerImpl::Stop()
{
  MutexGuard mg(mon);
  isStopping=true;
  mon.notify();
}
int MscManagerImpl::Execute()
{
  MutexGuard mg(mon);
  while(!isStopping)
  {
    while(!isStopping && changes.Count()==0)
    {
      mon.wait(5000);
    }
    if(isStopping)break;
    MscInfoChange change;
    changes.Pop(change);
    processChange(change);
  }
  return 0;
}

static void rollRename(std::string fileName,int index)
{
  char buf[32];
  sprintf(buf,".%d",index);
  if(File::Exists((fileName+buf).c_str()))
  {
    if(index<9)
    {
      rollRename(fileName,index+1);
    }else
    {
      File::Unlink((fileName+buf).c_str());
    }
  }
  if(index==0)
  {
    File::Rename(fileName.c_str(),(fileName+buf).c_str());
  }else
  {
    char buf2[32];
    sprintf(buf2,".%d",index-1);
    File::Rename((fileName+buf2).c_str(),(fileName+buf).c_str());
  }
}
static void bakFile(std::string fileName)
{
  std::string bakFilename=fileName+".bak";
  if(File::Exists(bakFilename.c_str()))
  {
    rollRename(bakFilename,0);
  }
  File::Rename(fileName.c_str(),bakFilename.c_str());
}

void MscManagerImpl::createFile(const char* fileName)
{
  storeFile.Open(fileName);
}


void MscManagerImpl::init(Manager& config)
    throw(ConfigException, InitException)
{
    // Loadup extra params from config if needed
    /*
    connection = ds.getConnection();
    if (!connection)
    {
        InitException exc("Get connection to DB failed");
        smsc_log_error(log, exc.what());
        throw exc;
    }

    // Loadup msc info here
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
            if (!mscNum || !mscNum[0]) continue;
            const char* mLockStr = rs->isNull(2) ? 0:rs->getString(2);
            const char* aLockStr = rs->isNull(3) ? 0:rs->getString(3);
            int fc = rs->isNull(4) ? 0:rs->getInt32(4);
            bool mLock = (mLockStr && mLockStr[0]=='Y');
            bool aLock = (aLockStr && aLockStr[0]=='Y');
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
        smsc_log_error(log, exc.what());
        throw InitException(exc.what());
    }
    */
try {
  string fileName=config.getString("MscManager.storeFile");
  try{
    storeFile.Open(fileName.c_str());
  }catch(std::exception& e)
  {
    smsc_log_error(log,"Failed to open msc store file '%s':'%s'",fileName.c_str(),e.what());
    bakFile(fileName);
    storeFile.Open(fileName.c_str());
  }
  MscInfo info;
  MutexGuard  guard(mon);
  while((info.offset=storeFile.Read(info)))
  {
    mscs.Insert(info.mscNum, new MscInfo(info));
  }
} catch (std::exception& e) {
    smsc_log_error(log,"Exception occured :'%s'",e.what());
    throw InitException("%s", e.what());
} catch (...) {
    smsc_log_error(log,"Unexpected exception during MSCMgr init");
    throw InitException("Unexpected exception");
}
}

void MscManagerImpl::processChange(const MscInfoChange& change)
{
  MscInfo** infoptr=mscs.GetPtr(change.mscNum);
  info2(log,"ChangeOp=%s, changeMsc=%s, %s",opNames[change.op],change.mscNum,infoptr?"Exists":"Doesn't exists");
  try{
    switch (change.op)
    {
      case MSCOP_INSERT:
      {
        if(!infoptr)
        {
          MscInfo* info = new MscInfo(change.mscNum, false, false, 0);
          mscs.Insert(info->mscNum,info);
          insertToFile(*info);
        }
      }break;
      case MSCOP_CLEAR:
      {
        if (infoptr)
        {
          MscInfo& info = **infoptr;
          if (info.manualLock || info.automaticLock)
          {
            info.manualLock = false;
            info.automaticLock = false;
            info.failureCount = 0;
            updateFile(info);
          }
        }
      }break;
      case MSCOP_DELETE:
      {
        if(infoptr)
        {
          removeFromFile(**infoptr);
          using namespace smsc::cluster;
          mscs.Delete(change.mscNum);
        }
      }break;
      case MSCOP_BLOCK:
      {
        if (infoptr)
        {
          MscInfo& info = **infoptr;
          if (!info.manualLock)
          {
            info.manualLock = true;
            updateFile(info);
          }
        }
      }break;
      case MSCOP_REPORT:
      {
        bool wasInsert=false;
        if(!infoptr)
        {
          if(!automaticRegistration)
          {
            //do not create record at all
            break;
          }
          MscInfo* tmp=new MscInfo(change.mscNum);
          //if(!automaticRegistration)tmp->manualLock=true;
          infoptr=mscs.SetItem(change.mscNum,tmp);
          wasInsert=true;
        }

        MscInfo& info=**infoptr;
        if(change.data)
        {
          if(!info.automaticLock && info.failureCount==0 && !wasInsert)
          {
            break;
          }
          info.automaticLock=false;
          info.failureCount=0;
        }else
        {
          if(automaticRegistration)info.failureCount++;
          if(!info.automaticLock && info.failureCount>=failureLimit)
          {
            info.blockTime=time(NULL);
            info.automaticLock=true;
          }
        }

        if(wasInsert)
        {
          insertToFile(info);
        }else
        {
          updateFile(info);
        }
      }break;
    }
  }
  catch (Exception& exc)
  {
    smsc_log_error(log, "Process change failed. %s", exc.what());
  }
}

void MscManagerImpl::insertToFile(MscInfo& mscinfo)
{
  mscinfo.offset=storeFile.Append(mscinfo);
  info2(log,"Insert into file %s, offset=%llx",mscinfo.mscNum,mscinfo.offset);
}

void MscManagerImpl::updateFile(MscInfo& mscinfo)
{
  info2(log,"Update file %s, offset=%llx",mscinfo.mscNum,mscinfo.offset);
  storeFile.Write(mscinfo.offset,mscinfo);
}

void MscManagerImpl::removeFromFile(MscInfo& mscinfo)
{
  info2(log,"Delete from file %s, offset=%llx",mscinfo.mscNum,mscinfo.offset);
  storeFile.Delete(mscinfo.offset);
}


/* ------------------------ MscStatus implementation ------------------------ */

void MscManagerImpl::report(const char* msc, bool status)
{
  if (!msc || !msc[0]) return;
  MutexGuard  guard(mon);
  changes.Push(MscInfoChange(MSCOP_REPORT,msc,status?1:0));
}

MscState MscManagerImpl::check(const char* msc)
{
  if (!msc || !msc[0]) return mscLocked;
  MutexGuard  guard(mon);

  MscInfo** infoptr=mscs.GetPtr(msc);
  if (!infoptr) return mscUnlocked;
  MscInfo& info = **infoptr;
  if(info.manualLock)return mscLocked;
  if(info.automaticLock)
  {
    time_t now=time(NULL);
    if(now-info.blockTime>singleAttemptTimeout)
    {
      info.blockTime=now;
      return mscUnlockedOnce;
    }
  }
  return info.automaticLock?mscLocked:mscUnlocked;
}

/* ------------------------ MscAdmin implementation ------------------------ */

void MscManagerImpl::registrate(const char* msc)
{
  if (!msc || !msc[0]) return;
  MutexGuard  guard(mon);
  changes.Push(MscInfoChange(MSCOP_INSERT,msc,0));
  mon.notify();
}

void MscManagerImpl::unregister(const char* msc)
{
  if (!msc || !msc[0]) return;
  MutexGuard  guard(mon);
  changes.Push(MscInfoChange(MSCOP_DELETE,msc));
  mon.notify();
}

void MscManagerImpl::block(const char* msc)
{
  if (!msc || !msc[0]) return;
  MutexGuard  guard(mon);
  changes.Push(MscInfoChange(MSCOP_BLOCK,msc));
  mon.notify();
}

void MscManagerImpl::clear(const char* msc)
{
  if (!msc || !msc[0]) return;
  MutexGuard  guard(mon);
  changes.Push(MscInfoChange(MSCOP_CLEAR,msc));
  mon.notify();
}

Array<MscInfo> MscManagerImpl::list()
{
  Array<MscInfo> list;

  MutexGuard  guard(mon);

  mscs.First();
  char* key=0;
  MscInfo* info = 0;
  while (mscs.Next(key, info))
  {
    if (info)
    {
      list.Push(*info);
    }
  }

  return list;
}


}}
