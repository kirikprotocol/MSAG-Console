#include "SmsIndex.hpp"
#include "util/debug.h"
#include <memory>
#include <utility>
#include <algorithm>
#include <vector>
#include <set>

namespace smsc{
namespace store{
namespace index{

size_t XTDataSize(const OffsetLtt& data)
{
  return 5+4;
}

void XTWriteData(smsc::core::buffers::File& file,const OffsetLtt& data)
{
  //file.WriteByte(data.off.high);
  //file.WriteNetInt32(data.off.low);
  uint8_t high=(uint8_t)((data.off>>32)&0xff);
  uint32_t low=(uint32_t)(data.off&0xffffffffu);
  file.WriteByte(high);
  file.WriteNetInt32(low);
  file.WriteNetInt32(data.ltt);
}

smsc::core::buffers::File::offset_type XTReadData(smsc::core::buffers::File& file,OffsetLtt& data)
{
  uint8_t high=file.ReadByte();
  uint32_t low=file.ReadNetInt32();
  data.off=high;
  data.off<<=32;
  data.off|=low;
  data.ltt=file.ReadNetInt32();
  return 5+4;
}


void SmsIndex::Init(ConfigView* cfg_)
{
  if(!cfg_)throw std::runtime_error("SmeIndex::Init : ConfigView is NULL!!!");
  ConfigView& cfg=*cfg_;
  maxFlushSpeed=cfg.getInt("maxFlushSpeed");
}


void SmsIndex::IndexateSms(const char* dir,SMSId id,uint64_t offset,SMS& sms)
{
  //hrtime_t idxtime=gethrtime();
  string path=loc;
  if(*path.rbegin()!='/')path+='/';
  path+=dir;
  path+='/';

  DataSet* ds;

  tm t;
  gmtime_r(&sms.lastTime,&t);
  int hour=t.tm_hour;

  char hrbuf[32];
  sprintf(hrbuf,"%02d",hour);
  std::string cacheKey=path+hrbuf;

  CacheItem** ciptr=cache.GetPtr(cacheKey.c_str());
  if(ciptr)
  {
    ds=&(*ciptr)->ds;
    (*ciptr)->usedInLastTransaction=true;
  }else
  {
    CacheItem* ci=new CacheItem;
    ci->ds.CreateNew(path.c_str(),hour,maxFlushSpeed);
    ci->usedInLastTransaction=true;
    cache.Insert(cacheKey.c_str(),ci);
    ds=&ci->ds;
  }

  OffsetLtt val(offset,sms.lastTime);
  char keybuf[128];
  sprintf(keybuf,"%lld",id);
  ds->idIdx->Insert(keybuf,val,true);
  ds->srcIdIdx->Insert(sms.getSourceSmeId(),val);
  ds->dstIdIdx->Insert(sms.getDestinationSmeId(),val);
  ds->routeIdIdx->Insert(sms.getRouteId(),val);
  ds->srcAddrIdx->Insert(sms.getOriginatingAddress().toString().c_str(),val);
  ds->dstAddrIdx->Insert(sms.getDealiasedDestinationAddress().toString().c_str(),val);


  /*
  DataSet ds;
  bool cached=true;

  {
    string idxfile=path;
    idxfile+="smsid.idx";
    if(!File::Exists(idxfile.c_str()) && File::Exists((idxfile+".old").c_str()))
    {
      File::Rename((idxfile+".old").c_str(),idxfile.c_str());
    }
  }

  if(!File::Exists((path+"smsid.idx").c_str()))
  {
    ds.CreateNew(config.maxFlushSpeed);

    ds.idHash->Create((path+"smsid.idx").c_str(),config.smsIdHashSize,cached);

    ds.srcIdHash->Create((path+"srcsmeid.idx").c_str(),config.smeIdHashSize,cached);
    ds.srcIdData->Create((path+"srcsmeid.dat").c_str(),config.smeIdRootSize,cached);

    ds.dstIdHash->Create((path+"dstsmeid.idx").c_str(),config.smeIdHashSize,cached);
    ds.dstIdData->Create((path+"dstsmeid.dat").c_str(),config.smeIdRootSize,cached);

    ds.routeIdHash->Create((path+"routeid.idx").c_str(),config.routeIdHashSize,cached);
    ds.routeIdData->Create((path+"routeid.dat").c_str(),config.routeIdRootSize,cached);

    ds.srcAddrHash->Create((path+"srcaddr.idx").c_str(),config.addrHashSize,cached);
    ds.srcAddrData->Create((path+"srcaddr.dat").c_str(),config.addrRootSize,cached);

    ds.dstAddrHash->Create((path+"dstaddr.idx").c_str(),config.addrHashSize,cached);
    ds.dstAddrData->Create((path+"dstaddr.dat").c_str(),config.addrRootSize,cached);

    ds.Flush();

    CacheItem *ci=new CacheItem;
    ci->ds=ds;
    ci->lastUsage=time(NULL);
    ci->usedInLastTransaction=true;
    cache.Insert(dir,ci);
  }else
  {
    if(cache.Exists(dir))
    {
      CacheItem* ci=cache.Get(dir);
      ds=ci->ds;
      ci->lastUsage=time(NULL);
      ci->usedInLastTransaction=true;
    }else
    {
      ds.CreateNew(config.maxFlushSpeed);

      ds.idHash->Open((path+"smsid.idx").c_str(),false,cached);

      ds.srcIdHash->Open((path+"srcsmeid.idx").c_str(),false,cached);
      ds.srcIdData->Open((path+"srcsmeid.dat").c_str(),false,cached);

      ds.dstIdHash->Open((path+"dstsmeid.idx").c_str(),false,cached);
      ds.dstIdData->Open((path+"dstsmeid.dat").c_str(),false,cached);

      ds.routeIdHash->Open((path+"routeid.idx").c_str(),false,cached);
      ds.routeIdData->Open((path+"routeid.dat").c_str(),false,cached);

      ds.srcAddrHash->Open((path+"srcaddr.idx").c_str(),false,cached);
      ds.srcAddrData->Open((path+"srcaddr.dat").c_str(),false,cached);

      ds.dstAddrHash->Open((path+"dstaddr.idx").c_str(),false,cached);
      ds.dstAddrData->Open((path+"dstaddr.dat").c_str(),false,cached);

      CacheItem *ci=new CacheItem;
      ci->ds=ds;
      ci->lastUsage=time(NULL);
      ci->usedInLastTransaction=true;
      cache.Insert(dir,ci);
    }
  }
  ds.idHash->Insert(id,IdLttKey(offset,sms.lastTime));

  Int64Key v;
  AutoChunkHandle h;

#define IDX(field,storage,sz) \
  if(ds.storage##Hash->LookUp(sms.field,v)) \
  { \
    h=AutoChunkHandle(ds.storage##Data->OpenChunk(v.key)); \
  }else \
  { \
    h=AutoChunkHandle(ds.storage##Data->CreateChunk(sz)); \
    ds.storage##Hash->Insert(sms.field,h->GetId()); \
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
    dealiasedDestinationAddress.toString().c_str(),dstAddr,
    config.smeAddrChunkSize.Exists(sms.dstSmeId)?
      config.smeAddrChunkSize.Get(sms.dstSmeId):config.defAddrChunkSize
  );
#undef IDX
  //idxtime=gethrtime()-idxtime;
  //__warning2__("indexed in %lld mcsec",idxtime/1000);
  */
}

typedef vector<pair<uint32_t,uint64_t> > ResVector;

class IReader{
public:
  virtual bool Read(IdLttKey&)=0;
  virtual ~IReader(){}
};

class ChunkHandleReader:public IReader{
public:
  ChunkHandleReader(IntLttChunkFile::ChunkHandle* h):handle(h)
  {
  }
  bool Read(IdLttKey& key)
  {
    return handle->Read(key);
  }
protected:
  RefPtr<IntLttChunkFile::ChunkHandle> handle;
};

class UnionChunkHandleReader:public IReader{
public:
  UnionChunkHandleReader(IntLttChunkFile::ChunkHandle* h1,IntLttChunkFile::ChunkHandle* h2):
    handle1(h1),handle2(h2),readFirst(true)
  {
  }
  bool Read(IdLttKey& key)
  {
    if(readFirst)
    {
      bool rv=handle1->Read(key);
      if(rv)
      {
        uset.insert(key);
        return true;
      }
      readFirst=false;
    }
    for(;;)
    {
      bool rv=handle2->Read(key);
      if(!rv)return false;
      if(uset.find(key)!=uset.end())
      {
        continue;
      }
      uset.insert(key);
      return true;
    }
  }
protected:
  bool readFirst;
  std::set<IdLttKey> uset;
  RefPtr<IntLttChunkFile::ChunkHandle> handle1;
  RefPtr<IntLttChunkFile::ChunkHandle> handle2;
};

void ReadToVector(IReader* h,ResVector& v,uint32_t from,uint32_t till)
{
  IdLttKey k;
  while(h->Read(k))
  {
    if(k.ltt<from || k.ltt>till)continue;
    v.push_back(make_pair(k.ltt,k.key));
  }
}

/*
struct CmpPair{
  bool operator()(const pair<uint64_t,uint32_t>& a,const pair<uint64_t,uint32_t>& b)
  {
    return a.first<b.first;
  }
};
*/

struct FileAndKey{
  std::string file;
  std::string key;
  FileAndKey* orfk;
  FileAndKey():orfk(0)
  {
  }
  FileAndKey(std::string argFile,std::string argKey):file(argFile),key(argKey),orfk(0)
  {
  }
  FileAndKey(const FileAndKey& fk)
  {
    file=fk.file;
    key=fk.key;
    if(fk.orfk)
    {
      orfk=new FileAndKey(*fk.orfk);
    }else
    {
      orfk=0;
    }
  }
  ~FileAndKey()
  {
    if(orfk)delete orfk;
  }

};

struct DataFilter{
  time_t fromDate;
  time_t tillDate;
  DataFilter(time_t argFrom,time_t argTo):fromDate(argFrom),tillDate(argTo)
  {

  }
  bool operator()(const OffsetLtt& key)const
  {
    return key.ltt<fromDate || key.ltt>tillDate;
  }
};

int SmsIndex::QuerySms(const char* dir,const ParamArray& params,ResultArray& res)
{
  string path=loc;
  if(*path.rbegin()!='/')path+='/';
  path+=dir;
  path+='/';
  if(!File::Exists((path+".version2").c_str()))
  {
    return QuerySmsV1(dir,params,res);
  }

  typedef std::vector<OffsetLtt> LookupVector;
  time_t fromDate=0;
  time_t tillDate=0xFFFFFFFFFFULL;

  typedef std::vector<FileAndKey> FileKeyVector;

  FileKeyVector fkVector;

  for(int i=0;i<params.Count();i++)
  {
    const Param& p=params[i];
    switch(p.type)
    {
      case Param::tSmsId:{
        FileAndKey fk;
        fk.file="smsid";
        char buf[64];
        sprintf(buf,"%lld",p.iValue);
        fk.key=buf;
        fkVector.push_back(fk);
      }break;
      case Param::tFromDate:{
        fromDate=p.dValue;
      }break;
      case Param::tTillDate:{
        tillDate=p.dValue;
      }break;
      case Param::tSrcAddress:{
        fkVector.push_back(FileAndKey("srcaddr",p.sValue));
      }break;
      case Param::tDstAddress:{
        fkVector.push_back(FileAndKey("dstaddr",p.sValue));
      }break;
      case Param::tSrcSmeId:{
        fkVector.push_back(FileAndKey("srcid",p.sValue));
      }break;
      case Param::tDstSmeId:{
        fkVector.push_back(FileAndKey("dstid",p.sValue));
      }break;
      case Param::tRouteId:{
        fkVector.push_back(FileAndKey("routeid",p.sValue));
      }break;
      case Param::tAbnAddress:{
        FileAndKey fk("srcaddr",p.sValue);
        fk.orfk=new FileAndKey("dstaddr",p.sValue);
        fkVector.push_back(fk);
      }break;
      case Param::tSmeId:{
        FileAndKey fk("srcid",p.sValue);
        fk.orfk=new FileAndKey("dstid",p.sValue);
        fkVector.push_back(fk);
      }break;
    }
  }
  int fromh=0;
  int toh=23;
  /*
  if(fromDate!=0)
  {
    tm t;
    gmtime_r(&fromDate,&t);
    fromh=t.tm_hour;
  }
  if(tillDate!=0xffffffffffull)
  {
    tm t;
    gmtime_r(&tillDate,&t);
    toh=t.tm_hour;
  }
  */

  bool firstFK=true;
  LookupVector lv,lvtmp,lvtmp2;
  ResultArray rv;

  for(FileKeyVector::iterator it=fkVector.begin();it!=fkVector.end();it++)
  {
    lvtmp.clear();
    for(int i=fromh;i<=toh;i++)
    {
      char hbuf[32];
      char hbuf2[32];
      sprintf(hbuf2,"%02d",i);
      sprintf(hbuf,"_%02d.idx",i);
      std::string fileName=path+it->file+hbuf;
      std::string cacheKey=path+hbuf2;
      CacheItem** ciptr=cache.GetPtr(cacheKey.c_str());
      if(ciptr)
      {
        RefPtr<OffsetXTree> idx=(*ciptr)->ds.GetIdx(it->file);
        idx->Lookup(it->key.c_str(),lvtmp);
      }else
      {
        if(!File::Exists(fileName.c_str()))continue;
        File f;
        f.ROpen(fileName.c_str());
        OffsetXTree::Lookup(f,it->key.c_str(),lvtmp);
      }

      if(it->orfk)
      {
        if(ciptr)
        {
          RefPtr<OffsetXTree> idx=(*ciptr)->ds.GetIdx(it->orfk->file);
          idx->Lookup(it->orfk->key.c_str(),lvtmp);
        }else
        {
          fileName=path+it->orfk->file+hbuf;
          if(File::Exists(fileName.c_str()))
          {
            File f;
            f.ROpen(fileName.c_str());
            OffsetXTree::Lookup(f,it->orfk->key.c_str(),lvtmp);
          }
        }
      }
    }
    //fromdate tilldate!!!
    {
      __trace__("loaded:");    
      for(LookupVector::iterator lit=lvtmp.begin();lit!=lvtmp.end();lit++)
      {
        __trace2__("%d,%d",lit->off,lit->ltt);
      }
    }
    DataFilter df(fromDate,tillDate);
    lvtmp2.clear();
    std::insert_iterator<LookupVector> ins1(lvtmp2,lvtmp2.begin());
    std::remove_copy_if(lvtmp.begin(),lvtmp.end(),ins1,df);
    std::sort(lvtmp2.begin(),lvtmp2.end());
    lvtmp.clear();
    std::insert_iterator<LookupVector> ins2(lvtmp,lvtmp.begin());
    std::unique_copy(lvtmp2.begin(),lvtmp2.end(),ins2);
    {
      __trace__("filtered and sorted:");    
      for(LookupVector::iterator lit=lvtmp.begin();lit!=lvtmp.end();lit++)
      {
        __trace2__("%d,%d",lit->off,lit->ltt);
      }
    }
    if(firstFK)
    {
      __trace__("first param - make copy");
      lv=lvtmp;
      firstFK=false;
    }else
    {
      __trace__("additional param - set_intersection");
      lvtmp2.swap(lv);
      lv.clear();
      std::insert_iterator<LookupVector> ins3(lv,lv.begin());
      std::set_intersection(lvtmp.begin(),lvtmp.end(),lvtmp2.begin(),lvtmp2.end(),ins3);
      {
        for(LookupVector::iterator lit=lv.begin();lit!=lv.end();lit++)
        {
          __trace2__("%d,%d",lit->off,lit->ltt);
        }
      }
    }
  }
  for(LookupVector::iterator it=lv.begin();it!=lv.end();it++)
  {
    QueryResult qr;
    qr.lastTryTime=it->ltt;
    qr.offset=it->off;
    res.Push(qr);
  }
  return res.Count();
}

int SmsIndex::QuerySmsV1(const char* dir,const ParamArray& params,ResultArray& res)
{
  string path=loc;
  if(*path.rbegin()!='/')path+='/';
  path+=dir;
  path+='/';

  typedef vector<RefPtr<IntLttChunkFile> > ChunkFiles;
  typedef vector<RefPtr<IReader> > SrcVector;
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
        fromDate=(uint32_t)p.dValue;
      }break;
      case Param::tTillDate:{
        tillDate=(uint32_t)p.dValue;
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
          sources.push_back(\
            RefPtr<IReader>( \
              new ChunkHandleReader(f->OpenChunk(v.key)) \
            )\
          ); \
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
      case Param::tAbnAddress:{
        IntLttChunkFile::ChunkHandle* h1=0;
        IntLttChunkFile::ChunkHandle* h2=0;
        {
          AddrDiskHash h;
          h.Open((path+"srcaddr" ".idx").c_str());
          Int64Key v;
          if(h.LookUp(p.sValue.c_str(),v))
          {
            RefPtr<IntLttChunkFile> f(new IntLttChunkFile);
            srcfiles.push_back(f);
            f->Open((path+"srcaddr" ".dat").c_str());
            h1=f->OpenChunk(v.key);
          }
        }
        {
          AddrDiskHash h;
          h.Open((path+"dstaddr" ".idx").c_str());
          Int64Key v;
          if(h.LookUp(p.sValue.c_str(),v))
          {
            RefPtr<IntLttChunkFile> f(new IntLttChunkFile);
            srcfiles.push_back(f);
            f->Open((path+"dstaddr" ".dat").c_str());
            h2=f->OpenChunk(v.key);
          }
        }
        if(h1==0 && h2==0)return 0;
        if(h1!=0 && h2==0)
        {
          sources.push_back
          (
            RefPtr<IReader>(new ChunkHandleReader(h1))
          );
        }else if(h1==0 && h2!=0)
        {
          sources.push_back
          (
            RefPtr<IReader>(new ChunkHandleReader(h2))
          );
        }else
        {
          sources.push_back
          (
            RefPtr<IReader>(new UnionChunkHandleReader(h1,h2))
          );
        }
      }break;
      case Param::tSmeId:{
        IntLttChunkFile::ChunkHandle* h1=0;
        IntLttChunkFile::ChunkHandle* h2=0;
        {
          SmeIdDiskHash h;
          h.Open((path+"srcsmeid" ".idx").c_str());
          Int64Key v;
          if(h.LookUp(p.sValue.c_str(),v))
          {
            RefPtr<IntLttChunkFile> f(new IntLttChunkFile);
            srcfiles.push_back(f);
            f->Open((path+"srcsmeid" ".dat").c_str());
            h1=f->OpenChunk(v.key);
          }
        }
        {
          SmeIdDiskHash h;
          h.Open((path+"dstsmeid" ".idx").c_str());
          Int64Key v;
          if(h.LookUp(p.sValue.c_str(),v))
          {
            RefPtr<IntLttChunkFile> f(new IntLttChunkFile);
            srcfiles.push_back(f);
            f->Open((path+"dstsmeid" ".dat").c_str());
            h2=f->OpenChunk(v.key);
          }
        }
        if(h1==0 && h2==0)return 0;
        if(h1!=0 && h2==0)
        {
          sources.push_back
          (
            RefPtr<IReader>(new ChunkHandleReader(h1))
          );
        }else if(h1==0 && h2!=0)
        {
          sources.push_back
          (
            RefPtr<IReader>(new ChunkHandleReader(h2))
          );
        }else
        {
          sources.push_back
          (
            RefPtr<IReader>(new UnionChunkHandleReader(h1,h2))
          );
        }
      }break;
    }
  }
  if(sources.size()==1)
  {
    ReadToVector(sources.front().Get(),rv,fromDate,tillDate);
    sort(rv.begin(),rv.end());
    for(ResVector::iterator i=rv.begin();i!=rv.end();i++)
    {
      QueryResult qr;
      qr.offset=i->second;
      qr.lastTryTime=i->first;
      res.Push(qr);
    }
    return (int)rv.size();
  }
  ReadToVector(sources.front().Get(),rv,fromDate,tillDate);
  sort(rv.begin(),rv.end());
  /*
  printf("After first read: %d\n",rv.size());
  for(int i=0;i<rv.size();i++)
  {
    printf("%d,%lld\n",rv[i].first,rv[i].second);
  }
  */
  sources.erase(sources.begin());
  while(sources.size()>0)
  {
    tmp1.resize(0);
    ReadToVector(sources.front().Get(),tmp1,fromDate,tillDate);
    sort(tmp1.begin(),tmp1.end());
    tmp2.resize(0);
    set_intersection(rv.begin(),rv.end(),tmp1.begin(),tmp1.end(),back_inserter(tmp2));
    swap(tmp2,rv);
    sources.erase(sources.begin());
    /*
    printf("After next read: %d\n",rv.size());
    for(int i=0;i<rv.size();i++)
    {
      printf("%d,%lld\n",rv[i].first,rv[i].second);
    }
    */
  }

  for(ResVector::iterator i=rv.begin();i!=rv.end();i++)
  {
    QueryResult qr;
    qr.offset=i->second;
    qr.lastTryTime=i->first;
    res.Push(qr);
  }

  return (int)rv.size();
}


void SmsIndex::DataSet::Flush()
{
  char buf[1024];
  File f;
  f.setMaxFlushSpeed(maxFlushSpeed);
  f.WOpen(MakeFileName("smsid",buf,true));
  RenameGuard smsidguard(buf);
  idIdx->WriteToFile(f);
  f.WOpen(MakeFileName("srcid",buf,true));
  RenameGuard srcidguard(buf);
  srcIdIdx->WriteToFile(f);
  f.WOpen(MakeFileName("dstid",buf,true));
  dstIdIdx->WriteToFile(f);
  RenameGuard dstidguard(buf);
  f.WOpen(MakeFileName("routeid",buf,true));
  RenameGuard routeidguard(buf);
  routeIdIdx->WriteToFile(f);
  f.WOpen(MakeFileName("srcaddr",buf,true));
  RenameGuard srcaddrguard(buf);
  srcAddrIdx->WriteToFile(f);
  f.WOpen(MakeFileName("dstaddr",buf,true));
  RenameGuard dstaddrguard(buf);
  dstAddrIdx->WriteToFile(f);

  smsidguard.release();
  srcidguard.release();
  dstidguard.release();
  routeidguard.release();
  srcaddrguard.release();
  dstaddrguard.release();
}

void SmsIndex::DataSet::Discard()
{
  char buf[1024];
  File f;
  f.setMaxFlushSpeed(maxFlushSpeed);
  f.ROpen(MakeFileName("smsid",buf,false));
  idIdx->ReadFromFile(f);
  f.ROpen(MakeFileName("srcid",buf,false));
  srcIdIdx->ReadFromFile(f);
  f.ROpen(MakeFileName("dstid",buf,false));
  dstIdIdx->ReadFromFile(f);
  f.ROpen(MakeFileName("routeid",buf,false));
  routeIdIdx->ReadFromFile(f);
  f.ROpen(MakeFileName("srcaddr",buf,false));
  srcAddrIdx->ReadFromFile(f);
  f.ROpen(MakeFileName("dstaddr",buf,false));
  dstAddrIdx->ReadFromFile(f);
}


OffsetXTree* SmsIndex::DataSet::CreateOrReadIdx(const char* idxname)
{
  OffsetXTree* idx=new OffsetXTree;
  char buf[1024];
  File f;
  MakeFileName(idxname,buf,false);
  if(File::Exists(buf))
  {
    f.ROpen(buf);
    idx->ReadFromFile(f);
  }
  return idx;
}

void SmsIndex::DataSet::CreateNew(const char* argDir,int argHour,int argFlushSpeed)
{
  maxFlushSpeed=argFlushSpeed;
  dir=argDir;
  hour=argHour;

  idIdx=CreateOrReadIdx("smsid");
  srcIdIdx=CreateOrReadIdx("srcid");
  dstIdIdx=CreateOrReadIdx("dstid");
  routeIdIdx=CreateOrReadIdx("routeid");
  srcAddrIdx=CreateOrReadIdx("srcaddr");
  dstAddrIdx=CreateOrReadIdx("dstaddr");
}


}//namespace index
}//namespace store
}//namespace smsc
