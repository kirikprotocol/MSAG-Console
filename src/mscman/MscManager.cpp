#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/EventMonitor.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/Array.hpp>
#include <core/threads/Thread.hpp>

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

    struct MscInfoChange
    {

        MscInfoOp   op;
        char        mscNum[22];
        int         data;
        File::offset_type offset;

        MscInfoChange(): op(MSCOP_UNKNOWN)
        {
          memset(mscNum,0,sizeof(mscNum));
        }

        MscInfoChange(MscInfoOp argOp,const char* argMsc,int argData=0,File::offset_type argOffset=0)
            : op(argOp),data(argData),offset(argOffset)
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
          offset=that.offset;
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
        virtual void report(const char* msc, bool status,File::offset_type offset=0);
        virtual bool check(const char* msc);

        /* MscAdmin implementation */
        virtual void registrate(const char* msc,File::offset_type offset=0);
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
            isStopping(false)
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

  const char sig[]="SMSCMSCM";
  uint32_t ver=0x00010000;
  string fileName=config.getString("MscManager.storeFile");
  if(!File::Exists(fileName.c_str()))
  {
    storeFile.RWCreate(fileName.c_str());
    storeFile.Write(sig,8);
    storeFile.WriteNetInt32(ver);
    storeFile.Flush();
    return;
  }else
  {
    storeFile.RWOpen(fileName.c_str());
  }
  char fileSig[9]={0,};
  uint32_t fileVer;
  storeFile.Read(fileSig,8);
  if(strcmp(sig,fileSig))
  {
    throw InitException("Invalid msc store file(%s!=%s):%s",sig,fileSig,fileName.c_str());
  }
  fileVer=storeFile.ReadNetInt32();
  if(fileVer!=ver)
  {
    throw InitException("Invalid msc store file version:%s",fileName.c_str());
  }
  File::offset_type pos=storeFile.Pos();
  File::offset_type sz=storeFile.Size();
  MutexGuard  guard(mon);
  while(pos<sz)
  {
    bool used=storeFile.ReadByte()==1;
    if(used)
    {
      MscInfo* info=new MscInfo;
      info->Read(storeFile);
      mscs.Insert(info->mscNum, info);
    }else
    {
      holes.push_back(pos);
      storeFile.SeekCur(MscInfo::Size());
    }
    pos+=1+MscInfo::Size();
    if(sz-pos<1+MscInfo::Size())
    {
      holes.push_back(pos);
      break;
    }
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
          MscInfo* info = new MscInfo(change.mscNum, false, !automaticRegistration, 0);
          mscs.Insert(info->mscNum,info);

          if(change.offset!=0)
          {
            info->offset=change.offset;
          }else
          {
            insertToFile(*info);
            using namespace smsc::cluster;
            if(Interconnect::getInstance()->getRole()==MASTER)
            {
              Interconnect::getInstance()->sendCommand(new MscRegistrateCommand(info->mscNum,info->offset));
            }
          }
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
            using namespace smsc::cluster;
            if(Interconnect::getInstance()->getRole()!=SLAVE)
            {
              if(Interconnect::getInstance()->getRole()==MASTER)
              {
                Interconnect::getInstance()->sendCommand(new MscClearCommand(change.mscNum));
              }
              updateFile(info);
            }
          }
        }
      }break;
      case MSCOP_DELETE:
      {
        if(infoptr)
        {
          removeFromFile(**infoptr);
          using namespace smsc::cluster;
          if(Interconnect::getInstance()->getRole()!=SLAVE)
          {
            mscs.Delete(change.mscNum);
            if(Interconnect::getInstance()->getRole()==MASTER)
            {
              Interconnect::getInstance()->sendCommand(new MscUnregisterCommand(change.mscNum));
            }
          }
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
            using namespace smsc::cluster;
            if(Interconnect::getInstance()->getRole()!=SLAVE)
            {
              updateFile(info);
              if(Interconnect::getInstance()->getRole()==MASTER)
              {
                Interconnect::getInstance()->sendCommand(new MscBlockCommand(change.mscNum));
              }
            }
          }
        }
      }break;
      case MSCOP_REPORT:
      {
        bool wasInsert=false;
        if(!infoptr)
        {
          MscInfo* tmp=new MscInfo(change.mscNum);
          if(!automaticRegistration)tmp->manualLock=true;
          infoptr=mscs.SetItem(change.mscNum,tmp);
          wasInsert=true;
        }

        MscInfo& info=**infoptr;
        if(change.data)
        {
          info.automaticLock=false;
        }else
        {
          info.failureCount++;
          info.automaticLock=info.failureCount>=failureLimit;
        }

        using namespace smsc::cluster;
        if(!change.offset)
        {
          if(wasInsert)
          {
            insertToFile(info);
          }else
          {
            updateFile(info);
          }
          if(Interconnect::getInstance()->getRole()==MASTER)
          {
            Interconnect::getInstance()->sendCommand(new MscReportCommand(change.mscNum,change.data,info.offset));
          }

        }else
        {
          info.offset=change.offset;
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
  File::offset_type offset;
  if(holes.empty())
  {
    offset=storeFile.Size();
  }else
  {
    offset=holes.back();
    holes.pop_back();
  }
  mscinfo.offset=offset;
  storeFile.Seek(offset-1);
  storeFile.WriteByte(1);
  mscinfo.Write(storeFile);
  storeFile.Flush();
  info2(log,"Insert into file %s, offset=%llx",mscinfo.mscNum,mscinfo.offset);
}

void MscManagerImpl::updateFile(MscInfo& mscinfo)
{
  info2(log,"Update file %s, offset=%llx",mscinfo.mscNum,mscinfo.offset);
  storeFile.Seek(mscinfo.offset);
  mscinfo.Write(storeFile);
  storeFile.Flush();
}

void MscManagerImpl::removeFromFile(MscInfo& mscinfo)
{
  info2(log,"Delete from file %s, offset=%llx",mscinfo.mscNum,mscinfo.offset);
  holes.push_back(mscinfo.offset);
  storeFile.Seek(mscinfo.offset-1);
  storeFile.WriteByte(0);
  storeFile.Flush();
}


/* ------------------------ MscStatus implementation ------------------------ */

void MscManagerImpl::report(const char* msc, bool status,File::offset_type off)
{
  if (!msc || !msc[0]) return;
  MutexGuard  guard(mon);
  changes.Push(MscInfoChange(MSCOP_REPORT,msc,status?1:0,off));
}

bool MscManagerImpl::check(const char* msc)
{
  if (!msc || !msc[0]) return false;
  MutexGuard  guard(mon);

  MscInfo** infoptr=mscs.GetPtr(msc);
  if (!infoptr) return true;
  MscInfo& info = **infoptr;
  return !(info.manualLock || info.automaticLock);
}

/* ------------------------ MscAdmin implementation ------------------------ */

void MscManagerImpl::registrate(const char* msc,File::offset_type off)
{
  if (!msc || !msc[0]) return;
  MutexGuard  guard(mon);
  changes.Push(MscInfoChange(MSCOP_INSERT,msc,0,off));
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
