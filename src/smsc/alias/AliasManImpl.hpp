#ifndef __SMSC_ALIAS_ALIASMANIMPL_HPP__
#define __SMSC_ALIAS_ALIASMANIMPL_HPP__

#include "AliasMan.hpp"
#include "core/buffers/XTree.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/FixedRecordFile.hpp"
#include "sms/sms_util.h"
#include "logger/Logger.h"

namespace smsc{
namespace alias{

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;

class AliasManImpl:public AliasManager
{
public:
  AliasManImpl(const char* storeFile):store("ALIASMAN",0x00010000)
  {
    allowFileModification=false;
    fileName=storeFile;
    log=smsc::logger::Logger::getInstance("aliasman");
  }
  virtual ~AliasManImpl()
  {
    clear();
  }
  void enableControllerMode()
  {
    allowFileModification=true;
  }
  void addAlias(const AliasInfo& ai)
  {
    if(ai.addr.length==0 || ai.alias.length==0)throw smsc::util::Exception("AliasMan::addAlias - attempt to add empty alias or addr");
    if(ai.addr.value[ai.addr.length-1]=='?' || ai.alias.value[ai.alias.length-1]=='?')
    {
      int i=1;
      while(i>=ai.addr.length && ai.addr.value[ai.addr.length-i]=='?')i++;
      int j=1;
      while(j>=ai.alias.length && ai.alias.value[ai.alias.length-j]=='?')j++;
      if(i!=j)throw smsc::util::Exception("Mask part of alias and address doesn't match:'%s' <-> '%s'",ai.addr.toString().c_str(),ai.alias.toString().c_str());
    }
    smsc_log_info(log,"addAlias:%s->%s",ai.addr.toString().c_str(),ai.alias.toString().c_str());
    sync::MutexGuard mg(mtx);
    Record* tmp;
    char bufAddr[32];
    char bufAlias[32];
    ai.addr.toString(bufAddr,sizeof(bufAddr));
    bool duplAddr2Alias=false;
    bool duplAlias2Addr=false;
    if(addr2alias.Find(bufAddr,tmp))
    {
      if(ai.addr==tmp->addr)
      {
        duplAddr2Alias=true;
      }
    }

    ai.alias.toString(bufAlias,sizeof(bufAlias));
    if(alias2addr.Find(bufAlias,tmp))
    {
      if(ai.alias==tmp->alias)
      {
        duplAlias2Addr=true;
      }
    }
    if(duplAddr2Alias && !duplAlias2Addr)
    {
      smsc_log_warn(log,"duplicate addr<->alias:%s<->%s",bufAddr,bufAlias);
      throw smsc::util::Exception("Duplicate alias(addr2alias) found:%s->%s",bufAlias,bufAddr);
    }else if(duplAlias2Addr && !duplAddr2Alias)
    {
      smsc_log_warn(log,"duplicate alias<->addr:%s<->%s",bufAlias,bufAddr);
      throw smsc::util::Exception("Duplicate alias(alias2addr) found:%s->%s",bufAlias,bufAddr);
    }else if(duplAddr2Alias && duplAlias2Addr)
    {
      smsc_log_warn(log,"duplicate2 alias<->addr:%s<->%s",bufAlias,bufAddr);
      throw smsc::util::Exception("Duplicate alias(alias2addr & addr2alias) found:%s->%s",bufAlias,bufAddr);
    }

    Record *recptr=new Record(ai.addr,ai.alias,ai.hide);
    alias2addr.Insert(bufAlias,recptr);
    if(ai.hide)
    {
      addr2alias.Insert(bufAddr,recptr);
    }
    if(allowFileModification)
    {
      recptr->offset=store.Append(*recptr);
    }
  }

  void deleteAlias(const smsc::sms::Address& alias)
  {
    sync::MutexGuard mg(mtx);
    Record** recptrptr=alias2addr.Extract(alias.toString().c_str());
    if(!recptrptr)
    {
      smsc_log_info(log,"deleteAlias:%s - failed",alias.toString().c_str());
      return;
    }
    smsc_log_info(log,"deleteAlias:%s - ok",alias.toString().c_str());
    Record* recptr=*recptrptr;
    addr2alias.Delete(recptr->addr.toString().c_str());
    if(allowFileModification)
    {
      store.Delete(recptr->offset);
    }

    delete recptr;
    delete recptrptr;
  }

  bool AliasToAddress(const smsc::sms::Address& alias,smsc::sms::Address& addr)
  {
    sync::MutexGuard mg(mtx);
    Record* recptr;
    if(alias2addr.Find(alias.toString().c_str(),recptr))
    {
      addr=MakeAddrFromAlias(alias,recptr);
      return true;
    }
    return false;
  }
  bool AddressToAlias(const smsc::sms::Address& addr,smsc::sms::Address& alias)
  {
    sync::MutexGuard mg(mtx);
    Record* recptr;
    char buf[32];
    addr.toString(buf,sizeof(buf));
    if(addr2alias.Find(buf,recptr))
    {
      alias=recptr->alias;
      return true;
    }
    return false;
  }
  void clear()
  {
    sync::MutexGuard mg(mtx);
    Deleter d;
    alias2addr.ForEach(d);
    alias2addr.Clear();
    addr2alias.Clear();
  }

  void Load()
  {
    smsc_log_info(log,"Loading aliases from file:%s",fileName.c_str());
    store.Open(fileName.c_str());
    Record r;
    File::offset_type off;
    int cnt=0;
    while(off=store.Read(r))
    {
      Record *recptr=new Record(r.addr,r.alias,r.hide);
      char buf[32];
      if(r.hide)
      {
        r.addr.toString(buf,sizeof(buf));
        addr2alias.Insert(buf,recptr);
      }
      r.alias.toString(buf,sizeof(buf));
      recptr->offset=off;
      alias2addr.Insert(buf,recptr);
      cnt++;
    }
    smsc_log_info(log,"%d aliases loaded",cnt);
  }
protected:

  struct Record{
    smsc::sms::Address addr;
    smsc::sms::Address alias;
    bool hide;
    buf::File::offset_type offset;
    Record():hide(false),offset(0){}
    Record(const smsc::sms::Address& argAddr,const smsc::sms::Address& argAlias,bool argHide):addr(argAddr),alias(argAlias),hide(argHide),offset(0){}

    template <class F>
    void Read(F& f)
    {
      ReadAddress(f,addr);
      ReadAddress(f,alias);
      hide=f.ReadByte();
    }
    template <class F>
    void Write(F& f)const
    {
      WriteAddress(f,addr);
      WriteAddress(f,alias);
      f.WriteByte(hide);
    }
    static size_t Size()
    {
      return smsc::sms::AddressSize()*2+1;
    }
  };

  struct Deleter
  {
    void operator()(Record* ptr)
    {
      delete ptr;
    }
  };

  smsc::sms::Address MakeAddrFromAlias(const smsc::sms::Address& alias,Record* recptr)
  {
    smsc::sms::Address addr=recptr->addr;
    int i=recptr->alias.length-1;
    int j=addr.length-1;
    while(i>=0 && j>=0 && recptr->alias.value[i]=='?')
    {
      addr.value[j]=alias.value[i];
      i--;j--;
    }
    return addr;
  }

  XTree<Record*,HeapAllocator,false> addr2alias;
  XTree<Record*,HeapAllocator,false> alias2addr;
  buf::FixedRecordFile<Record> store;
  sync::Mutex mtx;
  std::string fileName;
  smsc::logger::Logger* log;
  bool allowFileModification;
};

}//alias
}//smsc

#endif
