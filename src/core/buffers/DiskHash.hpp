#ifndef __DISKHASH_HPP__
#define __DISKHASH_HPP__

#include "File.hpp"
#include "util/crc32.h"
#include <string>
#include "util/debug.h"
#include <exception>

namespace smsc{
namespace core{
namespace buffers{


class DiskHashDuplicateKeyException:public std::exception{
public:
  const char* what()const throw()
  {
    return "Duplicate key found during insert operation";
  }
};

/*
Key interface:
void Read(File&)throw(exception)
void Write(File&)throw(exception)
static uint32_t Size() //0 in case of variable length
uint32_t HashCode()

Value interface:
void Read(File&)throw(exception)
void Write(File&)throw(exception)
static uint32_t Size() //0 in case of variable length

Perfromance data:
static uint32_t MaxInplaceKeySize
static uint32_t MaxInplaceValueSize
*/


struct DefaultPerformanceData{
enum{
  MaxInplaceKeySize=48UL,
  MaxInplaceValueSize=48UL,
  ExpectBigKeyFileSize=0,
  ExpectBigValueFileSize=0
  };
};


static const uint32_t _dh_magic=0x48415348;
static const uint32_t _dh_ver  =0x00010000;

template <class K,class V,class P=DefaultPerformanceData>
class DiskHash{
protected:
  enum{
    flagClosedOk=1,
    flagInplaceKey=2,
    flagInplaceValue=4
  };

  struct DiskHashHeader{
    uint32_t magic;
    uint32_t ver;
    uint32_t count;
    uint32_t size;
    uint32_t flags;
    uint32_t hdrcrc32;
    void Read(File& f)
    {
      magic=f.ReadNetInt32();
      ver=f.ReadNetInt32();
      count=f.ReadNetInt32();
      size=f.ReadNetInt32();
      flags=f.ReadNetInt32();
      hdrcrc32=f.ReadNetInt32();
    }

    static uint32_t fldcrc(uint32_t crc,uint32_t fld)
    {
      uint32_t no=htonl(fld);
      return crc32(crc,&no,4);
    }

    uint32_t CalcHdrCrc()
    {
      int rv=fldcrc(0,magic);
      rv=fldcrc(rv,ver);
      rv=fldcrc(rv,count);
      rv=fldcrc(rv,size);
      rv=fldcrc(rv,flags);
      return rv;
    }

    void Write(File& f)
    {
      f.WriteNetInt32(magic);
      f.WriteNetInt32(ver);
      f.WriteNetInt32(count);
      f.WriteNetInt32(size);
      f.WriteNetInt32(flags);
      hdrcrc32=CalcHdrCrc();
      f.WriteNetInt32(hdrcrc32);
    }
    static int Size()
    {
      return 6*4;
    }
  };

  File f;
  //File fk;
  //File fv;
  std::string name;

  int recsize;
  bool inplacekey;
  bool inplaceval;
  bool isFileOpen;
  bool isCached;
  bool isReadOnly;

  int count;
  int size;

  int reallocPercent;

  void CalcRecSize()
  {
    recsize=4+2;//hashcode+refcount
    inplacekey=K::Size()<=P::MaxInplaceKeySize;
    inplaceval=V::Size()<=P::MaxInplaceValueSize;
    recsize+=inplacekey?K::Size():P::ExpectBigKeyFileSize?8:4;
    recsize+=inplaceval?V::Size():P::ExpectBigValueFileSize?8:4;
  }

  void Rehash()
  {
    __warning2__("disk hash rehashing %d/%d",count,size);
    File g;
    g.RWCreate((name+".tmp").c_str());
    DiskHashHeader h;
    h.size=size*reallocPercent/100;

    g.OpenInMemory(DiskHashHeader::Size()+recsize*h.size);

    h.magic=_dh_magic;
    h.ver=_dh_ver;
    h.count=count;
    h.flags=0;
    h.flags|=inplacekey?flagInplaceKey:0;
    h.flags|=inplaceval?flagInplaceValue:0;
    h.Write(g);
    g.ZeroFill(recsize*h.size);
    f.Seek(h.Size());
    uint16_t fl;
    uint32_t hc;
    K k;
    V v;
    for(int i=0;i<size;i++)
    {
      fl=f.ReadNetInt16();
      if(!fl)
      {
        try{
          f.SeekCur(recsize-2);
        }catch(...)
        {
          printf("fuck:%d/%d\n",i,size);
          throw;
        }
        continue;
      }
      hc=f.ReadNetInt32();
      k.Read(f);
      v.Read(f);
      int attempt=0;
      int idx;
      for(;;attempt++)
      {
        hc=k.HashCode(attempt);
        idx=(hc%h.size)*recsize;
        g.Seek(idx+h.Size());
        fl=g.ReadNetInt16();
        if(!fl)break;
      }
      g.Seek(idx+h.Size());
      fl=1;
      g.WriteNetInt16(fl);
      g.WriteNetInt32(hc);
      k.Write(g);
      v.Write(g);
    }
    //g.MemoryFlush();
    //g.Close();
    f.Close();
    size=h.size;
    remove(name.c_str());
    rename((name+".tmp").c_str(),name.c_str());
    /*
    f.RWOpen(name.c_str());
    f.OpenInMemory(0);
    */
    f.Swap(g);
    __warning__("rehashing finished");
  }

public:
  DiskHash()
  {
    CalcRecSize();
    isFileOpen=false;
    reallocPercent=200;
  }

  ~DiskHash()
  {
    Close();
  }

  void Open(const char* file,bool readonly=false,bool cached=false)
  {
    name=file;
    if(readonly)
      f.ROpen(file);
    else
    {
      if(!File::Exists(name.c_str()) && File::Exists((name+".old").c_str()))
      {
        rename((name+".old").c_str(),name.c_str());
      }
      f.RWOpen(file);
    }
    name=file;
    isCached=cached;
    isReadOnly=readonly;
    f.SetUnbuffered();
    if(cached)f.OpenInMemory(0);
    DiskHashHeader h;
    h.Read(f);
    if(h.magic!=_dh_magic)throw RTERROR("invalid file magic");
    if(h.ver!=_dh_ver)throw RTERROR("incompatible version!");
    uint32_t crc=h.CalcHdrCrc();
    if(crc!=h.hdrcrc32)throw RTERROR("header crc failire");
    size=h.size;
    count=h.count;
  }

  void Create(const char* file,int prealloc,bool cached)
  {
    if(File::Exists(file))RTERROR("disk hash already exists");
    name=file;
    f.RWCreate(file);
    f.SetUnbuffered();
    isReadOnly=false;
    isCached=cached;
    DiskHashHeader h;
    if(cached)f.OpenInMemory(recsize*prealloc+h.Size());
    h.magic=_dh_magic;
    h.ver=_dh_ver;
    h.count=0;
    h.size=prealloc;
    h.flags=0;
    h.flags|=inplacekey?flagInplaceKey:0;
    h.flags|=inplaceval?flagInplaceValue:0;
    h.Write(f);
    f.ZeroFill(recsize*prealloc);
    //auto_ptr<char> buf(new char[recsize]);
    //memset(buf.get(),0,recsize);
    //for(int i=0;i<prealloc;i++)
    //{
    //  f.Write(buf.get(),recsize);
    //}
    size=h.size;
    count=0;
    isFileOpen=true;
  }

  File::offset_type Size()
  {
    return f.Size();
  }

  void Flush()
  {
    f.MemoryFlush();
  }

  void Close()
  {
    if(!isFileOpen)return;
    if(!isReadOnly)
    {
      DiskHashHeader h;
      f.Seek(0);
      h.Read(f);
      h.flags|=flagClosedOk;
      h.count=count;
      f.Seek(0);
      h.Write(f);
      if(isCached)f.MemoryFlush();
    }
    f.Close();
  }

  void DiscardCache()
  {
    if(isFileOpen)
    {
      f.Close();
      isFileOpen=false;
    }
  }

  void Insert(const K& key,const V& value)
  {
    if(!isFileOpen)RTERROR("Attempt to insert into not opened hash file");
    if(!inplacekey || !inplaceval)RTERROR("Non inplace kv not implemented yet");
    int attempt=0;

    if(count>=size*3/4)
    {
      Rehash();
    }

    for(;;attempt++)
    {
      if(attempt>20)
      {
        Rehash();
        attempt=0;
      }
      uint32_t hc=key.HashCode(attempt);
      uint32_t idx=(hc%size)*recsize;
      f.Seek(DiskHashHeader::Size()+idx);
      uint16_t fl=f.ReadNetInt16();
      if(fl)
      {
        f.ReadNetInt32();
        K k;
        k.Read(f);
        if(k==key)throw DiskHashDuplicateKeyException();
        continue;
      }
      f.Seek(DiskHashHeader::Size()+idx);
      fl=1;
      f.WriteNetInt16(fl);
      f.WriteNetInt32(hc);
      key.Write(f);
      value.Write(f);
      count++;
      return;
    }
  }
  bool LookUp(const K& key,V& value)
  {
    if(!isFileOpen)RTERROR("Attempt to insert into not opened hash file");
    int attempt=0;
    for(;;attempt++)
    {
      if(attempt==size)return false;
      uint32_t hc=key.HashCode(attempt);
      uint32_t idx=(hc%size)*recsize;
      f.Seek(DiskHashHeader::Size()+idx);
      uint16_t fl=f.ReadNetInt16();
      if(!fl)return false;
      uint32_t fhc=f.ReadNetInt32();
      if(fhc!=hc)continue;
      K fkey;
      fkey.Read(f);
      if(!(fkey==key))continue;
      value.Read(f);
      return true;
    }
  }
};

} //namespace buffers
} //namespace core
} //namespace smsc


#endif
