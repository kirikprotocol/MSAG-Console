#ifndef __SMSC_STORE_INDEX_SMSINDEX_HPP__
#define __SMSC_STORE_INDEX_SMSINDEX_HPP__

#include <string>
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

struct Param{
  enum ParamType{
    tSmsId      = T_SMS_ID,
    tFromDate   = T_FROM_DATE,   tTillDate   = T_TILL_DATE,
    tSrcAddress = T_SRC_ADDRESS, tDstAddress = T_DST_ADDRESS,
    tSrcSmeId   = T_SRC_SME_ID,  tDstSmeId   = T_DST_SME_ID,
    tRouteId    = T_ROUTE_ID
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

  std::string cacheDir;
  Hash<AutoChunkHandle> srcIdCache,dstIdCache,routeIdCache;
  Hash<uint64_t> srcAddrCache,dstAddrCache;

  RefPtr<SmsIdDiskHash> idHashCache;
  RefPtr<SmeIdDiskHash> srcIdHashCache;
  RefPtr<SmeIdDiskHash> dstIdHashCache;
  RefPtr<RouteIdDiskHash> routeIdHashCache;
  RefPtr<AddrDiskHash> srcAddrHashCache;
  RefPtr<AddrDiskHash> dstAddrHashCache;

  RefPtr<IntLttChunkFile> srcIdDataCache;
  RefPtr<IntLttChunkFile> dstIdDataCache;
  RefPtr<IntLttChunkFile> srcAddrDataCache;
  RefPtr<IntLttChunkFile> dstAddrDataCache;
  RefPtr<IntLttChunkFile> routeIdDataCache;
};

}//namespace index
}//namespace store
}//namespace smsc

#endif
