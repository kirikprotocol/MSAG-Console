#ifndef __SMSC_CORE_BUFFERS_JSTORE_HPP__
#define __SMSC_CORE_BUFFERS_JSTORE_HPP__

#include "core/buffers/File.hpp"
#include "core/buffers/PODHash.hpp"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/Exception.hpp"
#include "logger/Logger.h"

namespace smsc{
namespace core{
namespace buffers{

namespace synch=smsc::core::synchronization;

template <class K,class V,class HF>
class JStore:public smsc::core::threads::Thread{
public:
    JStore() : log_(smsc::logger::Logger::getInstance("jstore"))
  {
    rollTime=1800;
    maxChanges=10000;
    running=false;
    waitRolling=false;
    changesCount=0;
    sig="JSTORE";
    jsig="STOREJ";
      smsc_log_debug(log_,"ctor");
  }
  
    virtual ~JStore() {
        smsc_log_debug(log_,"dtor");
        Stop();
        WaitFor();
        smsc_log_debug(log_,"dtor finished");
    }

  void Insert(K k,const V& v)
  {
    synch::MutexGuard mg(mon);
    hash.Insert(k,v);
    Change c(k,v);
    c.Write(jfile);
    jfile.Flush();
    changesCount++;
    if(changesCount>maxChanges)
    {
      mon.notify();
    }
  }
  
  bool Lookup(K k,V& v)
  {
    synch::MutexGuard mg(mon);
    return hash.Lookup(k,v);
  }
  
  void Delete(K k)
  {
    synch::MutexGuard mg(mon);
    hash.Delete(k);
    Change c(k);
    c.Write(jfile);
    jfile.Flush();
    changesCount++;
    if(changesCount>maxChanges)
    {
      mon.notify();
    }
  }
  
  void Init(const char* argFileName,int argRollTime,size_t argMaxChanges)
  {
      smsc_log_debug(log_,"init '%s' rollTime=%d maxChg=%u",argFileName,argRollTime,unsigned(argMaxChanges));
    fileName=argFileName;
    rollTime=argRollTime;
    maxChanges=argMaxChanges;
    if(File::Exists((fileName+".rol").c_str()))
    {
      if(File::Exists(fileName.c_str()))
      {
        File::Unlink(fileName.c_str());
      }
      File::Rename((fileName+".rol").c_str(),fileName.c_str());
    }
    
    if(File::Exists(fileName.c_str()))
    {
      File storeFile;
      OpenRWFile(storeFile,fileName,sig,true);
      uint64_t cnt=storeFile.ReadNetInt64();
      K k;
      V v;
      for(uint64_t i=0;i<cnt;i++)
      {
        ReadRecord(storeFile,k,v);
        hash.Insert(k,v);
      }
    }
    std::string jfn=fileName+".j";
    std::string jrolfn=fileName+".jrol";
    
    if(File::Exists(jrolfn.c_str()) && File::Exists(jfn.c_str()))
    {
      File src,dst;
      OpenRWFile(src,jfn.c_str(),jsig,true);
      OpenRWFile(dst,jrolfn.c_str(),jsig,false);
      dst.SeekEnd(0);
      File::offset_type pos=src.Pos(),sz=src.Size();
      Change c;
      while(pos<sz)
      {
        pos+=c.Read(src);
        c.Write(dst);
      }
      src.Close();
      dst.Flush();
      File::Unlink(jfn.c_str());
      dst.Rename(jfn.c_str());
    }
    if(File::Exists(jfn.c_str()))
    {
      OpenRWFile(jfile,jfn.c_str(),jsig,false);
      try{
        Change c;
        File::offset_type sz=jfile.Size();
        File::offset_type pos=jfile.Pos();
        while(pos<sz)
        {
          pos+=c.Read(jfile);
          changesCount++;
          if(c.op==Change::opInsert)
          {
            hash.Insert(c.k,c.v);
          }else 
          if(c.op==Change::opDelete)
          {
            hash.Delete(c.k);
          }else
          {
            //ops?
          }
        }
      }catch(std::exception& e)
      {
        //eof?
      }
    }else
    {
      OpenWFile(jfile,fileName+".j",jsig);
    }
    {
      synch::MutexGuard mg(mon);
      Start();
      while(!running)mon.wait();
    }
      smsc_log_debug(log_,"inited '%s'",fileName.c_str());
  }
  
  void forceRoll()
  {
    synch::MutexGuard mg(mon);
    changesCount=maxChanges+1;
    mon.notify();
  }
  
  void setWaitRolling(bool val)
  {
    waitRolling=val;
  }
  
  int Execute()
  {
      smsc_log_debug(log_,"Execute(%s)",fileName.c_str());
    {
      synch::MutexGuard mg(mon);
      running=true;
      mon.notify();
    }
    while(running)
    {
      try{
      {
        synch::MutexGuard mg(mon);
        if(changesCount<maxChanges)
        {
          mon.wait(rollTime*1000);
          continue;
        }
        if(!running)break;
        smsc_log_debug(log_,"changesCount=%u",unsigned(changesCount));
        rollHash=hash;
        jfile.Rename((fileName+".jrol").c_str());
        jfile.Close();
        OpenWFile(jfile,fileName+".j",jsig);
        if(File::Exists(fileName.c_str()))
        {
          File::Rename(fileName.c_str(),(fileName+".rol").c_str());
        }
        changesCount=0;
      }
      File storeFile;
      OpenWFile(storeFile,fileName.c_str(),sig);
      storeFile.WriteNetInt64(rollHash.getCount());
      typename StoreHash::Iterator it=rollHash.getIterator();
      K k;
      V v;
      while(it.Next(k,v))
      {
        WriteRecord(storeFile,k,v);
        if(!waitRolling && !running)return 0;
      }
      storeFile.Flush();
      rollHash.Clear();
      if(File::Exists((fileName+".rol").c_str()))
      {
        File::Unlink((fileName+".rol").c_str());
      }
      File::Unlink((fileName+".jrol").c_str());
      }catch(std::exception& e)
      {
        fprintf(stderr,"Exception in jstore:%s\n",e.what());
      }
    }
      smsc_log_debug(log_,"Execute(%s) finished",fileName.c_str());
    return 0;
  }
  
  void Stop()
  {
      smsc_log_debug(log_,"stop");
    synch::MutexGuard mg(mon);
    running=false;
    mon.notify();
  }

  void First()
  {
    iter=hash.getIterator();
  }

  bool Next(K& k,V& v)
  {
    return iter.Next(k,v);
  }
  
protected:
  struct Change{
    enum ChangeOperation{
      opNone,
      opInsert,
      opDelete
    };
    uint8_t op;
    K k;
    V v;
    Change():op(opNone){}
    Change(K argK,const V& argV):op(opInsert),k(argK),v(argV){}
    Change(K argK):op(opDelete),k(argK){}
    File::offset_type Read(File& f)
    {
      File::offset_type sz=1;
      op=f.ReadByte();
      if(op==opInsert)
      {
        sz+=ReadRecord(f,k,v);
      }else if(op==opDelete)
      {
        sz+=ReadKey(f,k);
      }else
      {
        //ops?
      }
      return sz;
    }
    File::offset_type Write(File& f)
    {
      File::offset_type sz=1;
      f.WriteByte(op);
      if(op==opInsert)
      {
        sz+=WriteRecord(f,k,v);
      }else if(op==opDelete)
      {
        sz+=WriteKey(f,k);
      }else
      {
        //ops??
      }
      return sz;
    }
  };
  
  void OpenRWFile(File& f,const std::string& fn,const char* sig,bool readonly)
  {
    smsc_log_debug(log_,"opening '%s'%s",fn.c_str(),readonly ? " readonly" : "");
    if(readonly)
    {
      f.ROpen(fn.c_str());
    }else
    { 
      f.RWOpen(fn.c_str());
    }
    char fileSig[7]={0,};
    f.Read(fileSig,6);
    if(strcmp(fileSig,sig))
    {
      throw smsc::util::Exception("JStore: Invalid file signature:'%s'. Expected:'%s'",fileSig,sig);
    }
  }
  
    void OpenWFile(File& f,const std::string& fn,const char* sig)
    {
        smsc_log_debug(log_,"opening '%s' writeonly",fn.c_str());
        f.WOpen(fn.c_str());
        f.Write(sig,6);
        f.Flush();
    }
  
    smsc::logger::Logger* log_;
  typedef PODHash<K,V,HF> StoreHash;
  StoreHash hash,rollHash;
  typename StoreHash::Iterator iter;
  File jfile;
  smsc::core::synchronization::EventMonitor mon;
  std::string fileName;
  int rollTime;
  size_t maxChanges;
  size_t changesCount;
  const char* sig;
  const char* jsig;
  bool running;
  bool waitRolling;
};

}//buffers
}//core
}//smsc


#endif
