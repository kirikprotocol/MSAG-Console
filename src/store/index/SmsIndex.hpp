#ifndef __SMSC_STORE_INDEX_SMSINDEX_HPP__
#define __SMSC_STORE_INDEX_SMSINDEX_HPP__

#include <string>
#include <list>
#include "util/int.h"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/ChunkFile.hpp"
#include "util/crc32.h"


#include <sms/sms.h>
#include "core/buffers/Array.hpp"
#include "core/buffers/Hash.hpp"
#include "core/buffers/XHash.hpp"

#include "util/config/ConfigView.h"

#include "core/buffers/RefPtr.hpp"

namespace smsc{
namespace store{
namespace index{

using namespace smsc::sms;
using smsc::util::config::ConfigView;

static const uint8_t T_SMS_ID      = 10;
static const uint8_t T_FROM_DATE   = 20;
static const uint8_t T_TILL_DATE   = 30;
static const uint8_t T_SRC_ADDRESS = 40;
static const uint8_t T_DST_ADDRESS = 50;
static const uint8_t T_SRC_SME_ID  = 60;
static const uint8_t T_DST_SME_ID  = 70;
static const uint8_t T_ROUTE_ID    = 80;
static const uint8_t T_ABN_ADDRESS = 90;
static const uint8_t T_SME_ID      =100;

struct Param{
  enum ParamType{
    tSmsId      = T_SMS_ID,
    tFromDate   = T_FROM_DATE,   tTillDate   = T_TILL_DATE,
    tSrcAddress = T_SRC_ADDRESS, tDstAddress = T_DST_ADDRESS,
    tSrcSmeId   = T_SRC_SME_ID,  tDstSmeId   = T_DST_SME_ID,
    tRouteId    = T_ROUTE_ID,    tAbnAddress = T_ABN_ADDRESS,
    tSmeId      = T_SME_ID
  };
  ParamType type;
  std::string sValue;
  uint64_t iValue;
  time_t   dValue;
};

struct QueryResult{
  uint32_t lastTryTime;
  uint64_t offset;
};

typedef Array<Param> ParamArray;
typedef Array<QueryResult> ResultArray;


using namespace std;
using namespace smsc::core::buffers;
using smsc::util::crc32;

#include "SmsKeys.hpp"

typedef DiskHash<Int64Key,IdLttKey> SmsIdDiskHash;
typedef DiskHash<StrKey<15>,Int64Key> SmeIdDiskHash;
typedef DiskHash<StrKey<32>,Int64Key> RouteIdDiskHash;
typedef DiskHash<StrKey<28>,Int64Key> AddrDiskHash;


typedef ChunkFile<IdLttKey> IntLttChunkFile;
typedef RefPtr<IntLttChunkFile::ChunkHandle> AutoChunkHandle;


class SmsIndex{
public:
  SmsIndex(const char* location)
  {
    loc=location;
  }
  void Init(ConfigView*);
  void IndexateSms(const char* dir,SMSId id,uint64_t offset,SMS& sms);
  int QuerySms(const char* dir,const ParamArray& params,ResultArray& res);
  void BeginTransaction()
  {
    //
  }
  void EndTransaction()
  {
    cache.First();
    char* k;
    CacheItem*  v;
    std::list<std::string> toKill;
    time_t now=time(NULL);
    while(cache.Next(k,v))
    {
      v->ds.Flush();
      if(now-v->lastUsage>8*60*60)
      {
        toKill.push_back(k);
      }
    }

    for(std::list<std::string>::iterator i=toKill.begin();i!=toKill.end();i++)
    {
      delete cache.Get(i->c_str());
      cache.Delete(i->c_str());
    }
  }
  void RollBack()
  {
    char* k;
    CacheItem*  v;
    while(cache.Next(k,v))
    {
      v->ds.Discard();
    }
  }

protected:
  std::string loc;

  struct IndexConfig{
    int smsIdHashSize;
    int smeIdHashSize;
    int routeIdHashSize;
    int addrHashSize;

    int smeIdRootSize;
    int smeIdChunkSize;
    int routeIdRootSize;
    int routeIdChunkSize;
    int addrRootSize;
    int defAddrChunkSize;
    Hash<int> smeAddrChunkSize;
  }config;

  struct DataSet{

    void CreateNew()
    {
      idHash=new SmsIdDiskHash;
      srcIdHash=new SmeIdDiskHash;
      dstIdHash=new SmeIdDiskHash;
      routeIdHash=new RouteIdDiskHash;
      srcAddrHash=new AddrDiskHash;
      dstAddrHash=new AddrDiskHash;

      srcIdData=new IntLttChunkFile;
      dstIdData=new IntLttChunkFile;
      srcAddrData=new IntLttChunkFile;
      dstAddrData=new IntLttChunkFile;
      routeIdData=new IntLttChunkFile;
    }

    File::offset_type Size()
    {
      return
        idHash->Size()+
        srcIdHash->Size()+
        dstIdHash->Size()+
        routeIdHash->Size()+
        srcAddrHash->Size()+
        dstAddrHash->Size()+

        srcIdData->Size()+
        dstIdData->Size()+
        srcAddrData->Size()+
        dstAddrData->Size()+
        routeIdData->Size();
    }

    void Flush()
    {
      idHash->Flush();
      srcIdHash->Flush();
      dstIdHash->Flush();
      routeIdHash->Flush();
      srcAddrHash->Flush();
      dstAddrHash->Flush();

      srcIdData->Flush();
      dstIdData->Flush();
      srcAddrData->Flush();
      dstAddrData->Flush();
      routeIdData->Flush();
    }

    void Discard()
    {
      idHash->DiscardCache();
      srcIdHash->DiscardCache();
      dstIdHash->DiscardCache();
      routeIdHash->DiscardCache();
      srcAddrHash->DiscardCache();
      dstAddrHash->DiscardCache();

      srcIdData->DiscardCache();
      dstIdData->DiscardCache();
      srcAddrData->DiscardCache();
      dstAddrData->DiscardCache();
      routeIdData->DiscardCache();
    }


    RefPtr<SmsIdDiskHash> idHash;
    RefPtr<SmeIdDiskHash> srcIdHash;
    RefPtr<SmeIdDiskHash> dstIdHash;
    RefPtr<RouteIdDiskHash> routeIdHash;
    RefPtr<AddrDiskHash> srcAddrHash;
    RefPtr<AddrDiskHash> dstAddrHash;

    RefPtr<IntLttChunkFile> srcIdData;
    RefPtr<IntLttChunkFile> dstIdData;
    RefPtr<IntLttChunkFile> srcAddrData;
    RefPtr<IntLttChunkFile> dstAddrData;
    RefPtr<IntLttChunkFile> routeIdData;
  };

  struct CacheItem;
  friend struct smsc::store::index::SmsIndex::CacheItem;

  struct CacheItem{
    time_t lastUsage;
    DataSet ds;
  };

  Hash<CacheItem*> cache;
  //std::string cacheDir;
  //Hash<AutoChunkHandle> srcIdCache,dstIdCache,routeIdCache;
  //Hash<uint64_t> srcAddrCache,dstAddrCache;

};

}//namespace index
}//namespace store
}//namespace smsc

#endif
