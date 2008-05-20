#ifndef SCAG_SESSIONS_STORE
#define SCAG_SESSIONS_STORE

#include <inttypes.h>
#include <string>

#include "Session.h"
#include "core/buffers/RefPtr.hpp"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/PageFile.hpp"

namespace scag{
namespace sessions{


typedef smsc::core::buffers::RefPtr<Session,smsc::core::synchronization::Mutex> SessionPtr;
typedef void (*SessionLoadCallback)(void*,Session*);

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;


class SessionStore
{
public:

  SessionStore() {};
  ~SessionStore() {};

  void init(const std::string& dir,SessionLoadCallback cb,void* data);

  SessionPtr newSession(const CSessionKey& sessionKey);
  SessionPtr getSession(const CSessionKey& sessionKey);
  void deleteSession(const CSessionKey& sessionKey);
  bool updateSession(Session* session);
  void storeSessionIndex(Session* session);
  void deleteSessionByIndex(Session* session);
  uint32_t getSessionsCount() {return dhash.Count();}

protected:
    smsc::logger::Logger* log;

  class DiskSessionKey{
  protected:
  public:
    Address  addr;
    uint16_t umr;
    DiskSessionKey(){}
    DiskSessionKey(const CSessionKey& key):addr(key.abonentAddr),umr(key.USR){}

    bool operator==(const DiskSessionKey& that)const
    {
      return addr==that.addr && umr==that.umr;
    }

    std::string toString()
    {
      std::string str=addr.toString();
      char buf[16];
      sprintf(buf,":%hu",umr);
      str+=buf;
      return str;
    }
    static uint32_t Size()
    {
      return (uint32_t)(1+1+sizeof(AddressValue)+2);
    }
    void Read(File& f)
    {
      addr.type=f.ReadByte();
      addr.plan=f.ReadByte();
      f.Read(addr.value,sizeof(addr.value));
      addr.length=(uint8_t)strlen(addr.value);
      umr=f.ReadNetInt16();
    }
    void Write(File& f)const
    {
      f.WriteByte(addr.type);
      f.WriteByte(addr.plan);
      f.Write(addr.value,sizeof(addr.value));
      f.WriteNetInt16(umr);
    }
    uint32_t HashCode(uint32_t attempt)const
    {
      uint32_t res=(addr.type+addr.plan)*umr;
      res=crc32(res,addr.value,addr.length);
      for(;attempt>0;attempt--)
      {
        res+=umr;
        res=crc32(res,addr.value,addr.length);
      }
      return res;
    }
  };

  struct OffsetValue{
    File::offset_type value;

    OffsetValue():value(0){}
    OffsetValue(File::offset_type argValue):value(argValue){}
    OffsetValue(const OffsetValue& src):value(src.value){}

    static uint32_t Size(){return (uint32_t)sizeof(File::offset_type);}
    void Read(File& f)
    {
      value=f.ReadNetInt64();
    }
    void Write(File& f)const
    {
      f.WriteNetInt64(value);
    }
  };

  buf::PageFile pfile;
  buf::DiskHash<DiskSessionKey,OffsetValue> dhash;
  sync::Mutex mtx;
};

class CachedSessionStore
{
public:

  CachedSessionStore() : cache(NULL) { logger=smsc::logger::Logger::getInstance("csesstore"); };
  ~CachedSessionStore() { if(cache) delete[] cache; };

  void init(const std::string& dir,SessionLoadCallback cb,void* data, uint32_t mcs = 10000);

  SessionPtr getSession(const CSessionKey& sessionKey);

  SessionPtr newSession(const CSessionKey& sessionKey);
  void deleteSession(const CSessionKey& sessionKey);
  bool updateSession(Session* session);
  uint32_t getSessionsCount() {return store.getSessionsCount();}
  void storeSessionIndex(Session* session)
  {
    store.storeSessionIndex(session);
  }
  void deleteSessionByIndex(Session* session)
  {
    store.deleteSessionByIndex(session);
  }
protected:
    inline uint32_t getIdx(const CSessionKey& key) { return CSessionKey::CalcHash(key) % maxCacheSize; };

    sync::Mutex mtx;
    smsc::logger::Logger* logger;
    uint32_t maxCacheSize;
    SessionStore store;
    SessionPtr *cache;
};

}
}

#endif // SCAG_SESSIONS_STORE
