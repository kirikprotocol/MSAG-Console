#ifndef __SMSC_SYSTEM_DPFTRACKER_HPP__
#define __SMSC_SYSTEM_DPFTRACKER_HPP__

#include "sms/sms.h"
#include "core/threads/ThreadedTask.hpp"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "logger/Logger.h"
#include "core/buffers/File.hpp"
#include <set>

namespace smsc{
namespace system{

namespace sync=smsc::core::synchronization;
namespace buf=smsc::core::buffers;

using smsc::sms::Address;

class DpfTracker:public smsc::core::threads::Thread{
public:

  DpfTracker()
  {
    changesCount=0;
    maxChangesCount=10000;
    lastCompact=time(NULL);
    compactTime=300;
    isStopping=false;
  }

  void Init(const char* argStoreLocation,int to1179,int to1044);
  void registerSetDpf(const Address& abonent,const Address &smeAddr,int errCode,int smeIdx);
  void hlrAlert(const Address& abonent);

  int Execute();
  void stop()
  {
    cthread.stop();
    cthread.WaitFor();
    sync::MutexGuard mg(mon);
    isStopping=true;
    mon.notify();
  }

protected:
  int timeOut1179;
  int timeOut1044;
  std::string storeLocation;

  struct ReqRecord{
    time_t expiration;
    uint32_t smeIdx;
    smsc::sms::Address addr;

    static void WriteAddr(File& f,const Address& addr)
    {
      f.WriteByte(addr.type);
      f.WriteByte(addr.plan);
      f.WriteByte(addr.length);
      f.Write(addr.value,addr.length);
    }

    static void ReadAddr(File& f,Address& addr)
    {
      addr.type=f.ReadByte();
      addr.plan=f.ReadByte();
      addr.length=f.ReadByte();
      f.Read(addr.value,addr.length);
      addr.value[addr.length]=0;
    }

    void Write(File& f)const
    {
      f.WriteNetInt64(expiration);
      f.WriteNetInt32(smeIdx);
      WriteAddr(f,addr);
    }
    void Read(File& f)
    {
      expiration=f.ReadNetInt64();
      smeIdx=f.ReadNetInt32();
      ReadAddr(f,addr);
    }
    bool operator<(const ReqRecord& rhs)const
    {
      return expiration<rhs.expiration;
    }
  };

  struct Record;
  struct RecordPtrAbonentComparator{
    inline bool operator()(const Record* lhs,const Record* rhs)const;
    inline bool operator()(uint64_t abonent,const Record* rhs)const;
    inline bool operator()(const Record* lhs,uint64_t abonent)const;
  };

  struct RecordPtrExpirationComparator{
    inline bool operator()(const Record* lhs,const Record* rhs)const;
  };

  typedef std::set<Record*,RecordPtrAbonentComparator> AbonentsSet;
  typedef std::multiset<Record*,RecordPtrExpirationComparator> ExpirationsSet;

  struct Record{
    uint64_t abonent;
    typedef std::multiset<ReqRecord> ReqSet;
    ReqSet requests;
    time_t expiration;
    ExpirationsSet::iterator expIter;

    void Write(File& f)const
    {
      f.WriteNetInt64(abonent);
      f.WriteNetInt16((uint16_t)requests.size());
      ReqSet::iterator end=requests.end();
      for(ReqSet::iterator it=requests.begin();it!=end;it++)
      {
        it->Write(f);
      }
    }
    void Read(File& f)
    {
      abonent=f.ReadNetInt64();
      uint16_t reqCnt=f.ReadNetInt16();
      ReqRecord req;
      requests.clear();
      for(int i=0;i<reqCnt;i++)
      {
        req.Read(f);
        requests.insert(req);
      }
      expiration=requests.begin()->expiration;
    }
  };

  enum ChangeType{
    ctRegisterSme,
    ctRemoveAbonent,
    ctRemoveSme
  };
  struct Change{
    ChangeType ct;
    uint64_t abonent;
    time_t expiration;
    uint32_t smeIdx;
    Address addr;
    Change(uint64_t argAbonent,time_t argExpiration,uint32_t argSmeIdx,const Address& argAddr):ct(ctRegisterSme),
      abonent(argAbonent),expiration(argExpiration),smeIdx(argSmeIdx),addr(argAddr)
    {
    }
    Change(uint64_t argAbonent):ct(ctRemoveAbonent),abonent(argAbonent)
    {
    }
    Change(uint64_t argAbonent,uint32_t argSme):ct(ctRemoveSme),abonent(argAbonent),smeIdx(argSme)
    {
    }
    ReqRecord asReqRecord()const
    {
      ReqRecord req;
      req.expiration=expiration;
      req.smeIdx=smeIdx;
      req.addr=addr;
      return req;
    }
    void Write(File& f)const
    {
      f.WriteByte(ct);
      if(ct==ctRegisterSme)
      {
        f.WriteNetInt64(abonent);
        f.WriteNetInt64(expiration);
        f.WriteNetInt32(smeIdx);
        ReqRecord::WriteAddr(f,addr);
      }else if(ct==ctRemoveAbonent)
      {
        f.WriteNetInt64(abonent);
      }else if(ct==ctRemoveSme)
      {
        f.WriteNetInt64(abonent);
        f.WriteNetInt32(smeIdx);
      }
    }
    void Read(File& f)
    {
      ct=(ChangeType)f.ReadByte();
      if(ct==ctRegisterSme)
      {
        abonent=f.ReadNetInt64();
        expiration=f.ReadNetInt64();
        smeIdx=f.ReadNetInt32();
        ReqRecord::ReadAddr(f,addr);
      }else if(ct==ctRemoveAbonent)
      {
        abonent=f.ReadNetInt64();
      }else if(ct==ctRemoveSme)
      {
        abonent=f.ReadNetInt64();
        smeIdx=f.ReadNetInt32();
      }
    }
  };

  void SaveChange(const Change& c);
  bool StartCompacting();

  void ApplyChanges(const std::string& fileName);

  class CompactingThread:public smsc::core::threads::Thread{
  public:
    CompactingThread():compacting(false),needCompacting(false),isStopping(false)
    {
    }
    void init(const std::string& argStoreLocation);
    int Execute();
    void stop();
    void startCompacting();
    bool isCompacting()
    {
      MutexGuard mg(mon);
      return compacting || needCompacting;
    }
  protected:
    sync::EventMonitor mon;
    bool compacting;
    bool needCompacting;
    bool isStopping;
    std::string storeLocation;

    struct ReqRequestSmeIdComparator{
      bool operator()(const ReqRecord& a,const ReqRecord& b)const
      {
        return a.smeIdx<b.smeIdx;
      }
    };

    struct SmallRecord{
      uint64_t abonent;
      typedef std::set<ReqRecord,ReqRequestSmeIdComparator> ReqSet;
      mutable ReqSet requests;

      void Write(File& f)const
      {
        f.WriteNetInt64(abonent);
        f.WriteNetInt16((uint16_t)requests.size());
        ReqSet::iterator end=requests.end();
        for(ReqSet::iterator it=requests.begin();it!=end;it++)
        {
          it->Write(f);
        }
      }

      void Read(File& f)
      {
        abonent=f.ReadNetInt64();
        uint16_t reqCnt=f.ReadNetInt16();
        ReqRecord req;
        requests.clear();
        for(int i=0;i<reqCnt;i++)
        {
          req.Read(f);
          requests.insert(req);
        }
      }
    };
    struct SmallRecordComparator{
      bool operator()(const SmallRecord& a,const SmallRecord& b)const
      {
        return a.abonent<b.abonent;
      }
      bool operator()(const SmallRecord& a,uint64_t b)const
      {
        return a.abonent<b;
      }
      bool operator()(uint64_t a,const SmallRecord& b)const
      {
        return a<b.abonent;
      }
    };
    typedef std::set<SmallRecord,SmallRecordComparator> RecSet;
  };

  void sendAlertNotify(uint64_t abonent,const smsc::sms::Address& smeAddr,int smeIdx,int status);

  AbonentsSet abonents;
  ExpirationsSet expirations;
  sync::EventMonitor mon;
  bool isStopping;
  CompactingThread cthread;

  File changesStore;
  int changesCount;
  int maxChangesCount;
  time_t lastCompact;
  time_t compactTime;

  smsc::logger::Logger* log;

};

inline bool DpfTracker::RecordPtrAbonentComparator::operator()(const DpfTracker::Record* lhs,const DpfTracker::Record* rhs)const
{
  return lhs->abonent<rhs->abonent;
}
inline bool DpfTracker::RecordPtrAbonentComparator::operator()(uint64_t abonent,const DpfTracker::Record* rhs)const
{
  return abonent<rhs->abonent;
}
inline bool DpfTracker::RecordPtrAbonentComparator::operator()(const DpfTracker::Record* lhs,uint64_t abonent)const
{
  return lhs->abonent<abonent;
}

inline bool DpfTracker::RecordPtrExpirationComparator::operator()(const DpfTracker::Record* lhs,const DpfTracker::Record* rhs)const
{
  return lhs->expiration<rhs->expiration ||
         (lhs->expiration==rhs->expiration && lhs<rhs);
}


}
}


#endif

