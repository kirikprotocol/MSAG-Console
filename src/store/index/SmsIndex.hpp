#ifndef __SMSC_STORE_INDEX_SMSINDEX_HPP__
#define __SMSC_STORE_INDEX_SMSINDEX_HPP__

#include <string>
#include <list>
#include <set>
#include "util/int.h"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/ChunkFile.hpp"
#include "util/crc32.h"
#include "util/debug.h"
#include "core/buffers/DiskXTree.hpp"


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

static const uint8_t T_SMS_ID      =  10;
static const uint8_t T_FROM_DATE   =  20;
static const uint8_t T_TILL_DATE   =  30;
static const uint8_t T_SRC_ADDRESS =  40;
static const uint8_t T_DST_ADDRESS =  50;
static const uint8_t T_SRC_SME_ID  =  60;
static const uint8_t T_DST_SME_ID  =  70;
static const uint8_t T_ROUTE_ID    =  80;
static const uint8_t T_ABN_ADDRESS =  90;
static const uint8_t T_SME_ID      = 100;
static const uint8_t T_STATUS      = 110; // Used externally (to filter query results only)
static const uint8_t T_LAST_RESULT = 120; // Used externally (to filter query results only)

struct Param{
  enum ParamType{
    tSmsId      = T_SMS_ID,
    tFromDate   = T_FROM_DATE,   tTillDate   = T_TILL_DATE,
    tSrcAddress = T_SRC_ADDRESS, tDstAddress = T_DST_ADDRESS,
    tSrcSmeId   = T_SRC_SME_ID,  tDstSmeId   = T_DST_SME_ID,
    tRouteId    = T_ROUTE_ID,    tAbnAddress = T_ABN_ADDRESS,
    tSmeId      = T_SME_ID,      tStatus     = T_STATUS,
    tLastResult = T_LAST_RESULT
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

using namespace smsc::core::buffers;

typedef Array<Param> ParamArray;
typedef Array<QueryResult> ResultArray;


using namespace std;
using smsc::util::crc32;

#include "SmsKeys.hpp"

typedef DiskHash<Int64Key,IdLttKey> SmsIdDiskHash;
typedef DiskHash<StrKey<15>,Int64Key> SmeIdDiskHash;
typedef DiskHash<StrKey<32>,Int64Key> RouteIdDiskHash;
typedef DiskHash<StrKey<28>,Int64Key> AddrDiskHash;


typedef DiskXTree<OffsetLtt> OffsetXTree;

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
    __trace__("IDX: begin transaction");
    char* k;
    CacheItem* v;
    cache.First();
    while(cache.Next(k,v))
    {
      v->usedInLastTransaction=false;
    }
  }

  void EndTransaction()
  {
    __trace__("IDX: flushing transaction");
    char* k;
    CacheItem* v;
    cache.First();
    std::list<std::string> toKill;
    while(cache.Next(k,v))
    {
      if(v->usedInLastTransaction)
      {
        if(!File::Exists((v->ds.dir+".version2").c_str()))
        {
          File f;
          f.RWCreate((v->ds.dir+".version2").c_str());
        }
        v->ds.Flush();
      }else
      {
        toKill.push_back(k);
      }
    }
    for(std::list<std::string>::iterator it=toKill.begin();it!=toKill.end();it++)
    {
      CacheItem* ci=cache.Get(it->c_str());
      delete ci;
      cache.Delete(it->c_str());
    }
    __trace__("IDX: transaction finished");
  }
  void RollBack()
  {
    __trace__("IDX: rollback transaction");
    char* k;
    CacheItem* v;
    cache.First();
    while(cache.Next(k,v))
    {
      if(v->usedInLastTransaction)
      {
        v->ds.Discard();
      }
    }

    __trace__("IDX: rollback transaction finished");
  }

protected:
  std::string loc;

  int QuerySmsV1(const char* dir,const ParamArray& params,ResultArray& res);


  struct DataSet{

    void CreateNew(const char* argDir,int argHour,int argFlushSpeed);
    struct RenameGuard{
      std::string fn;
      std::string normfn;
      bool released;
      bool needUnlink;
      RenameGuard(const char* f):fn(f),released(false)
      {
        std::string::size_type pos=fn.rfind('.');
        normfn=fn.substr(0,pos);
        needUnlink=File::Exists(normfn.c_str());
      }
      ~RenameGuard()
      {
        if(released)
        {
          try{
            if(needUnlink)File::Unlink(normfn.c_str());
            File::Rename(fn.c_str(),normfn.c_str());
          }catch(std::exception& e)
          {
            smsc_log_error(smsc::logger::Logger::getInstance("sms.idx"),"Failed to unlink+rename index file:'%s'",e.what());
          }
        }else
        {
          try{
            File::Unlink(fn.c_str());
          }catch(std::exception& e)
          {
            smsc_log_error(smsc::logger::Logger::getInstance("sms.idx"),"Failed to unlink index file:'%s'",e.what());
          }
        }
      }
      void release()
      {
        released=true;
      }
    };

    void Flush();

    char* MakeFileName(const char* name,char* buf,bool isNew)
    {
      sprintf(buf,"%s/%s_%02d.idx%s",dir.c_str(),name,hour,isNew?".new":"");
      return buf;
    }

    OffsetXTree* CreateOrReadIdx(const char* idxname);

    RefPtr<OffsetXTree> GetIdx(const std::string name)
    {
      if(name=="smsid")
      {
        return idIdx;
      }else if(name=="srcid")
      {
        return srcIdIdx;
      }else if(name=="dstid")
      {
        return dstIdIdx;
      }else if(name=="routeid")
      {
        return routeIdIdx;
      }else if(name=="srcaddr")
      {
        return srcAddrIdx;
      }else if(name=="dstaddr")
      {
        return dstAddrIdx;
      }
      throw smsc::util::Exception("Unknown index:%s",name.c_str());
    }

    void Discard();

    int maxFlushSpeed;
    int hour;
    std::string dir;

    RefPtr<OffsetXTree> idIdx;
    RefPtr<OffsetXTree> srcIdIdx;
    RefPtr<OffsetXTree> dstIdIdx;
    RefPtr<OffsetXTree> routeIdIdx;
    RefPtr<OffsetXTree> srcAddrIdx;
    RefPtr<OffsetXTree> dstAddrIdx;

  };

  struct CacheItem;
  friend struct smsc::store::index::SmsIndex::CacheItem;

  struct CacheItem{
    bool   usedInLastTransaction;
    DataSet ds;
  };

  int maxFlushSpeed;
  Hash<CacheItem*> cache;
};

}//namespace index
}//namespace store
}//namespace smsc

#endif
