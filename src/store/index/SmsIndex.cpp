#include "SmsIndex.hpp"
#include <memory>
#include <utility>
#include <algorithm>
#include <vector>

namespace smsc{
namespace store{
namespace index{

void SmsIndex::Init(ConfigView* cfg_)
{
  if(!cfg_)throw std::runtime_error("SmeIndex::Init : ConfigView is NULL!!!");
  ConfigView& cfg=*cfg_;
  config.smsIdHashSize=cfg.getInt("smsIdHashSize");
  config.smeIdHashSize=cfg.getInt("smeIdHashSize");
  config.routeIdHashSize=cfg.getInt("routeIdHashSize");
  config.addrHashSize=cfg.getInt("addrHashSize");

  config.smeIdRootSize=cfg.getInt("smeIdRootSize");
  config.smeIdChunkSize=cfg.getInt("smeIdChunkSize");
  config.routeIdRootSize=cfg.getInt("routeIdRootSize");
  config.routeIdChunkSize=cfg.getInt("routeIdChunkSize");
  config.addrRootSize=cfg.getInt("addrRootSize");
  config.defAddrChunkSize=cfg.getInt("defAddrChunkSize");

  using smsc::util::config::CStrSet;
  auto_ptr<ConfigView> smeConfig(cfg.getSubConfig("smeAddrChunkSize"));
  if(smeConfig.get())
  {
    auto_ptr<CStrSet> params(smeConfig->getIntParamNames());
    CStrSet::iterator i=params->begin();
    for(;i!=params->end();i++)
    {
      config.smeAddrChunkSize.Insert(i->c_str(),smeConfig->getInt(i->c_str()));
    }
  }
}

void SmsIndex::IndexateSms(const char* dir,SMSId id,uint64_t offset,SMS& sms)
{
  string path=loc;
  if(*path.rbegin()!='/')path+='/';
  path+=dir;
  path+='/';

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



  if(!File::Exists((path+"smsid.idx").c_str()))
  {
    srcIdCache.Empty();
    dstIdCache.Empty();
    routeIdCache.Empty();
    srcAddrCache.Empty();
    dstAddrCache.Empty();

    cacheDir=dir;

    idHashCache=new SmsIdDiskHash;
    srcIdHashCache=new SmeIdDiskHash;
    dstIdHashCache=new SmeIdDiskHash;
    routeIdHashCache=new RouteIdDiskHash;
    srcAddrHashCache=new AddrDiskHash;
    dstAddrHashCache=new AddrDiskHash;

    srcIdDataCache=new IntLttChunkFile;
    dstIdDataCache=new IntLttChunkFile;
    srcAddrDataCache=new IntLttChunkFile;
    dstAddrDataCache=new IntLttChunkFile;
    routeIdDataCache=new IntLttChunkFile;

    idHash=idHashCache;
    srcIdHash=srcIdHashCache;
    dstIdHash=dstIdHashCache;
    routeIdHash=routeIdHashCache;
    srcAddrHash=srcAddrHashCache;
    dstAddrHash=dstAddrHashCache;

    srcIdData=srcIdDataCache;
    dstIdData=dstIdDataCache;
    srcAddrData=srcAddrDataCache;
    dstAddrData=dstAddrDataCache;
    routeIdData=routeIdDataCache;



    idHash->Create((path+"smsid.idx").c_str(),config.smsIdHashSize);//!!!!!!!!!!!!!!!!!!!!!

    srcIdHash->Create((path+"srcsmeid.idx").c_str(),config.smeIdHashSize);
    srcIdData->Create((path+"srcsmeid.dat").c_str(),config.smeIdRootSize);

    dstIdHash->Create((path+"dstsmeid.idx").c_str(),config.smeIdHashSize);
    dstIdData->Create((path+"dstsmeid.dat").c_str(),config.smeIdRootSize);

    routeIdHash->Create((path+"routeid.idx").c_str(),config.routeIdHashSize);
    routeIdData->Create((path+"routeid.dat").c_str(),config.routeIdRootSize);

    srcAddrHash->Create((path+"srcaddr.idx").c_str(),config.addrHashSize);
    srcAddrData->Create((path+"srcaddr.dat").c_str(),config.addrRootSize);

    dstAddrHash->Create((path+"dstaddr.idx").c_str(),config.addrHashSize);
    dstAddrData->Create((path+"dstaddr.dat").c_str(),config.addrRootSize);
  }else
  {
    if(cacheDir==dir)
    {
      idHash=idHashCache;
      srcIdHash=srcIdHashCache;
      dstIdHash=dstIdHashCache;
      routeIdHash=routeIdHashCache;
      srcAddrHash=srcAddrHashCache;
      dstAddrHash=dstAddrHashCache;

      srcIdData=srcIdDataCache;
      dstIdData=dstIdDataCache;
      srcAddrData=srcAddrDataCache;
      dstAddrData=dstAddrDataCache;
      routeIdData=routeIdDataCache;
    }else
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

      idHash->Open((path+"smsid.idx").c_str(),false);

      srcIdHash->Open((path+"srcsmeid.idx").c_str(),false);
      srcIdData->Open((path+"srcsmeid.dat").c_str(),false);

      dstIdHash->Open((path+"dstsmeid.idx").c_str(),false);
      dstIdData->Open((path+"dstsmeid.dat").c_str(),false);

      routeIdHash->Open((path+"routeid.idx").c_str(),false);
      routeIdData->Open((path+"routeid.dat").c_str(),false);

      srcAddrHash->Open((path+"srcaddr.idx").c_str(),false);
      srcAddrData->Open((path+"srcaddr.dat").c_str(),false);

      dstAddrHash->Open((path+"dstaddr.idx").c_str(),false);
      dstAddrData->Open((path+"dstaddr.dat").c_str(),false);
    }
  }
  idHash->Insert(id,IdLttKey(offset,sms.lastTime));

  Int64Key v;
  AutoChunkHandle h;

  if(cacheDir==dir)
  {
#define IDX1(field,storage,sz) \
  if(storage##Cache.Exists(sms.field)) \
  { \
    h=storage##Cache.Get(sms.field);\
  }else \
  { \
    h=storage##Data->CreateChunk(sz); \
    storage##Hash->Insert(sms.field,h->GetId()); \
    storage##Cache.Insert(sms.field,h); \
  } \
  h->Write(IdLttKey(offset,sms.lastTime));

#define IDX2(field,storage,sz) \
  if(storage##Cache.Exists(sms.field)) \
  { \
    ChunkId ci=storage##Cache.Get(sms.field);\
    h=storage##Data->OpenChunk(ci);\
  }else \
  { \
    h=storage##Data->CreateChunk(sz); \
    storage##Hash->Insert(sms.field,h->GetId()); \
    storage##Cache.Insert(sms.field,h->GetId()); \
  } \
  h->Write(IdLttKey(offset,sms.lastTime));


  IDX1(srcSmeId,srcId,config.smeIdChunkSize);
  IDX1(dstSmeId,dstId,config.smeIdChunkSize);
  IDX1(routeId,routeId,config.routeIdChunkSize);
  IDX2
  (
    originatingAddress.toString().c_str(),srcAddr,
    config.smeAddrChunkSize.Exists(sms.srcSmeId)?
      config.smeAddrChunkSize.Get(sms.srcSmeId):config.defAddrChunkSize
  );
  IDX2
  (
    destinationAddress.toString().c_str(),dstAddr,
    config.smeAddrChunkSize.Exists(sms.dstSmeId)?
      config.smeAddrChunkSize.Get(sms.dstSmeId):config.defAddrChunkSize
  );

#undef IDX

  }else
  {

#define IDX(field,storage,sz) \
  if(storage##Hash->LookUp(sms.field,v)) \
  { \
    h=AutoChunkHandle(storage##Data->OpenChunk(v.key)); \
  }else \
  { \
    h=AutoChunkHandle(storage##Data->CreateChunk(sz)); \
    storage##Hash->Insert(sms.field,h->GetId()); \
  } \
  h->Write(IdLttKey(offset,sms.lastTime));

  IDX(srcSmeId,srcId,config.smeIdChunkSize);
  IDX(dstSmeId,dstId,config.smeIdChunkSize);
  IDX(routeId,routeId,config.routeIdChunkSize);
  IDX
  (
    originatingAddress.toString().c_str(),srcAddr,
    config.smeAddrChunkSize.Exists(sms.srcSmeId)?
      config.smeAddrChunkSize.Get(sms.srcSmeId):config.defAddrChunkSize
  );
  IDX
  (
    destinationAddress.toString().c_str(),dstAddr,
    config.smeAddrChunkSize.Exists(sms.dstSmeId)?
      config.smeAddrChunkSize.Get(sms.dstSmeId):config.defAddrChunkSize
  );
#undef IDX
  }
}

typedef vector<pair<uint64_t,uint32_t> > ResVector;

void ReadToVector(IntLttChunkFile::ChunkHandle& h,ResVector& v,uint32_t from,uint32_t till)
{
  IdLttKey k;
  while(h.Read(k))
  {
    if(k.ltt<from || k.ltt>till)continue;
    v.push_back(make_pair(k.key,k.ltt));
  }
}

struct CmpPair{
  bool operator()(const pair<uint64_t,uint32_t>& a,const pair<uint64_t,uint32_t>& b)
  {
    return a.first<b.first;
  }
};

int SmsIndex::QuerySms(const char* dir,const ParamArray& params,ResultArray& res)
{
  string path=loc;
  if(*path.rbegin()!='/')path+='/';
  path+=dir;
  path+='/';

  typedef vector<RefPtr<IntLttChunkFile> > ChunkFiles;
  typedef vector<RefPtr<IntLttChunkFile::ChunkHandle> > SrcVector;
  SrcVector sources;
  ChunkFiles srcfiles;

  srcfiles.reserve(6);

  ResVector rv,tmp1,tmp2;

  uint32_t fromDate=0;
  uint32_t tillDate=0xFFFFFFFFU;

  for(int i=0;i<params.Count();i++)
  {
    const Param& p=params[i];
    switch(p.type)
    {
      case Param::tSmsId:{
        SmsIdDiskHash h;
        h.Open((path+"smsid.idx").c_str());
        IdLttKey v;
        if(h.LookUp(p.iValue,v))
        {
          QueryResult qr;
          qr.offset=v.key;
          qr.lastTryTime=v.ltt;
          res.Push(qr);
          return 1;
        }else
        {
          return 0;
        }
      }break;
      case Param::tFromDate:{
        fromDate=p.dValue;
      }break;
      case Param::tTillDate:{
        tillDate=p.dValue;
      }break;

#define SRC(hash,name) \
        hash h; \
        h.Open((path+name ".idx").c_str()); \
        Int64Key v; \
        if(h.LookUp(p.sValue.c_str(),v)) \
        { \
          RefPtr<IntLttChunkFile> f(new IntLttChunkFile); \
          srcfiles.push_back(f); \
          f->Open((path+name ".dat").c_str()); \
          sources.push_back(RefPtr<IntLttChunkFile::ChunkHandle>(f->OpenChunk(v.key))); \
        }else \
        { \
          return 0; \
        }
      case Param::tSrcAddress:{
        SRC(AddrDiskHash,"srcaddr");
      }break;
      case Param::tDstAddress:{
        SRC(AddrDiskHash,"dstaddr");
      }break;
      case Param::tSrcSmeId:{
        SRC(SmeIdDiskHash,"srcsmeid");
      }break;
      case Param::tDstSmeId:{
        SRC(SmeIdDiskHash,"dstsmeid");
      }break;
      case Param::tRouteId:{
        SRC(RouteIdDiskHash,"routeid");
      }break;
    }
  }
  if(sources.size()==1)
  {
    IntLttChunkFile::ChunkHandle& h=*sources[0];
    IdLttKey r;
    int cnt=0;
    while(h.Read(r))
    {
      QueryResult qr;
      qr.offset=r.key;
      qr.lastTryTime=r.ltt;
      res.Push(qr);
      cnt++;
    }
    return cnt;
  }
  ReadToVector(*sources.front(),rv,fromDate,tillDate);
  sort(rv.begin(),rv.end(),CmpPair());
  sources.erase(sources.begin());
  while(sources.size()>0)
  {
    ReadToVector(*sources.front(),tmp1,fromDate,tillDate);
    sort(tmp1.begin(),tmp1.end(),CmpPair());
    tmp2.resize(0);
    set_intersection(rv.begin(),rv.end(),tmp1.begin(),tmp1.end(),back_inserter(tmp2));
    swap(tmp2,rv);
    sources.erase(sources.begin());
  }

  for(ResVector::iterator i=rv.begin();i!=rv.end();i++)
  {
    QueryResult qr;
    qr.offset=i->first;
    qr.lastTryTime=i->second;
    res.Push(qr);
  }

  return rv.size();
}


}//namespace index
}//namespace store
}//namespace smsc
