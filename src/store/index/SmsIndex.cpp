#include "SmsIndex.hpp"
#include "core/buffers/DiskHash.hpp"
#include "core/buffers/ChunkFile.hpp"
#include "util/crc32.h"
#include <memory>
#include <utility>
#include <algorithm>
#include <vector>

namespace smsc{
namespace store{
namespace index{

using namespace smsc::core::buffers;
using smsc::util::crc32;
using namespace std;

struct Int64Key{
  uint64_t key;

  Int64Key():key(0){}
  Int64Key(uint64_t key):key(key){}
  Int64Key(const Int64Key& src)
  {
    key=src.key;
  }
  Int64Key& operator=(const Int64Key& src)
  {
    key=src.key;
    return *this;
  }
  uint64_t Get()const{return key;}

  static uint32_t Size(){return 8;}
  void Read(File& f)
  {
    key=f.ReadNetInt64();
  }
  void Write(File& f)const
  {
    f.WriteNetInt64(key);
  }
  uint32_t HashCode(uint32_t attempt)const
  {
    return crc32(attempt,&key,sizeof(key));
  }
  bool operator==(const Int64Key& cmp)
  {
    return key==cmp.key;
  }
};

template <int N>
class StrKey{
protected:
  char str[N+1];
  uint8_t len;
public:
  StrKey()
  {
    memset(str,0,N+1);
    len=0;
  }
  StrKey(const char* s)
  {
    int l=strlen(s);
    strncpy(str,s,N);
    str[N]=0;
    len=l>N?N:l;
  }
  StrKey(const StrKey& src)
  {
    strcpy(str,src.str);
    len=src.len;
  }
  StrKey& operator=(const StrKey& src)
  {
    strcpy(str,src.str);
    len=src.len;
    return *this;
  }

  bool operator==(const StrKey& cmp)
  {
    return cmp.len==len && !strcmp(cmp.str,str);
  }

  const char* toString(){return str;}
  static uint32_t Size(){return N+1;}
  void Read(File& f)
  {
    f.XRead(len);
    f.Read(str,N);
    str[len]=0;
  }
  void Write(File& f)const
  {
    f.XWrite(len);
    f.Write(str,N);
  }
  uint32_t HashCode(uint32_t attempt)const
  {
    return crc32(attempt,str,len);
  }
};

struct IdLttKey{
  uint64_t key;
  uint32_t ltt;

  IdLttKey():key(0),ltt(0){}
  IdLttKey(uint64_t key,uint32_t ltt):key(key),ltt(ltt){}
  IdLttKey(const IdLttKey& src)
  {
    key=src.key;
    ltt=src.ltt;
  }
  IdLttKey& operator=(const IdLttKey& src)
  {
    key=src.key;
    ltt=src.ltt;
    return *this;
  }

  static uint32_t Size(){return 12;}
  void Read(File& f)
  {
    key=f.ReadNetInt64();
    ltt=f.ReadNetInt32();
  }
  void Write(File& f)const
  {
    f.WriteNetInt64(key);
    f.WriteNetInt32(ltt);
  }
  static void WriteBadValue(File& f)
  {
    uint64_t a=~0;
    uint32_t b=~0;
    f.WriteInt64(a);
    f.WriteInt32(b);
  }
  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t rv=crc32(attempt,&key,sizeof(key));
    return crc32(rv,&ltt,sizeof(ltt));
  }
  bool operator==(const IdLttKey& cmp)
  {
    return key==cmp.key && ltt==cmp.ltt;
  }
};



typedef DiskHash<Int64Key,IdLttKey> SmsIdDiskHash;
typedef DiskHash<StrKey<15>,Int64Key> SmeIdDiskHash;
typedef DiskHash<StrKey<32>,Int64Key> RouteIdDiskHash;
typedef DiskHash<StrKey<28>,Int64Key> AddrDiskHash;

struct ChunkFileData{
  enum{
    RootChunks=1024,
    ChunkRecordsCount=32
  };
};


typedef ChunkFile<IdLttKey,ChunkFileData> IntLttChunkFile;
typedef std::auto_ptr<IntLttChunkFile::ChunkHandle> AutoChunkHandle;

void SmsIndex::IndexateSms(const char* dir,SMSId id,uint64_t offset,SMS& sms)
{
  string path=loc;
  if(*path.rbegin()!='/')path+='/';
  path+=dir;
  path+='/';

  SmsIdDiskHash idHash;
  SmeIdDiskHash srcIdHash;
  SmeIdDiskHash dstIdHash;
  RouteIdDiskHash routeIdHash;
  AddrDiskHash srcAddrHash;
  AddrDiskHash dstAddrHash;

  IntLttChunkFile srcIdData;
  IntLttChunkFile dstIdData;
  IntLttChunkFile srcAddrData;
  IntLttChunkFile dstAddrData;
  IntLttChunkFile routeIdData;


  if(!File::Exists((path+"smsid.idx").c_str()))
  {
    idHash.Create((path+"smsid.idx").c_str(),300*1024);//!!!!!!!!!!!!!!!!!!!!!

    srcIdHash.Create((path+"srcsmeid.idx").c_str(),256);
    srcIdData.Create((path+"srcsmeid.dat").c_str());

    dstIdHash.Create((path+"dstsmeid.idx").c_str(),256);
    dstIdData.Create((path+"dstsmeid.dat").c_str());

    routeIdHash.Create((path+"routeid.idx").c_str(),256);
    routeIdData.Create((path+"routeid.dat").c_str());

    srcAddrHash.Create((path+"srcaddr.idx").c_str(),100*1024);
    srcAddrData.Create((path+"srcaddr.dat").c_str());

    dstAddrHash.Create((path+"dstaddr.idx").c_str(),100*1024);
    dstAddrData.Create((path+"dstaddr.dat").c_str());
  }else
  {
    idHash.Open((path+"smsid.idx").c_str(),false);

    srcIdHash.Open((path+"srcsmeid.idx").c_str(),false);
    srcIdData.Open((path+"srcsmeid.dat").c_str(),false);

    dstIdHash.Open((path+"dstsmeid.idx").c_str(),false);
    dstIdData.Open((path+"dstsmeid.dat").c_str(),false);

    routeIdHash.Open((path+"routeid.idx").c_str(),false);
    routeIdData.Open((path+"routeid.dat").c_str(),false);

    srcAddrHash.Open((path+"srcaddr.idx").c_str(),false);
    srcAddrData.Open((path+"srcaddr.dat").c_str(),false);

    dstAddrHash.Open((path+"dstaddr.idx").c_str(),false);
    dstAddrData.Open((path+"dstaddr.dat").c_str(),false);
  }
  idHash.Insert(id,IdLttKey(offset,sms.lastTime));

  Int64Key v;
  AutoChunkHandle h;

#define IDX(field,storage) \
  if(storage##Hash.LookUp(sms.field,v)) \
  { \
    h=AutoChunkHandle(storage##Data.OpenChunk(v.key)); \
  }else \
  { \
    h=AutoChunkHandle(storage##Data.CreateChunk()); \
    storage##Hash.Insert(sms.field,h->GetId()); \
  } \
  h->Write(IdLttKey(id,sms.lastTime));

  IDX(srcSmeId,srcId);
  IDX(dstSmeId,dstId);
  IDX(routeId,routeId);
  IDX(originatingAddress.toString().c_str(),srcAddr);
  IDX(destinationAddress.toString().c_str(),dstAddr);
}

template <class T>
class RefPtr{
protected:
  struct RefPtrData{
    RefPtrData():refCount(0),ptr(0)
    {
    }
    int refCount;
    T  *ptr;
    void Lock(){}
    void Unlock(){}
  };
public:
  explicit RefPtr(T* ptr=NULL)
  {
    data=new RefPtrData;
    data->ptr=ptr;
    Ref();
  }
  RefPtr(const RefPtr& src)
  {
    data=src.data;
    Ref();
  }
  ~RefPtr()
  {
    Unref();
  }
  RefPtr& operator=(const RefPtr& src)
  {
    Unref();
    data=src.data;
    Ref();
    return *this;
  }
  RefPtr& operator=(T* ptr)
  {
    Unref();
    data=new RefPtrData;
    data->ptr=ptr;
    Ref();
    return *this;
  }
  T& operator*()
  {
    return *data->ptr;
  }
  T* operator->()
  {
    return data->ptr;
  }
  T* Get()
  {
    return data->ptr;
  }
protected:
  void Ref()
  {
    data->Lock();
    data->refCount++;
    data->Unlock();
  }
  void Unref()
  {
    if(!data || !data->refCount)return;
    data->Lock();
    int count=--data->refCount;
    data->Unlock();
    if(!count)
    {
      if(data->ptr)delete data->ptr;
      delete data;
    }
  }
  RefPtrData *data;
};//RefPtr

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
