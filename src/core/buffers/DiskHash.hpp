#ifndef __DISKHASH_HPP__
#define __DISKHASH_HPP__

#include "File.hpp"
#include "util/crc32.h"
#include <string>

namespace smsc{
namespace core{
namespace buffers{


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
  };

  File f;
  //File fk;
  //File fv;
  string name;

  int recsize;
  bool inplacekey;
  bool inplaceval;
  bool fileopen;

  int count;
  int size;

  void ReadHeader(DiskHashHeader& h)
  {
    h.magic=f.ReadNetInt32();
    h.ver=f.ReadNetInt32();
    h.count=f.ReadNetInt32();
    h.size=f.ReadNetInt32();
    h.flags=f.ReadNetInt32();
    h.hdrcrc32=f.ReadNetInt32();
  }

  void WriteHeader(const DiskHashHeader& h)
  {
    f.Seek(0);

    f.WriteNetInt32(h.magic);
    f.WriteNetInt32(h.ver);
    f.WriteNetInt32(h.count);
    f.WriteNetInt32(h.size);
    f.WriteNetInt32(h.flags);
    f.WriteNetInt32(h.hdrcrc32);
  }

  void CalcRecSize()
  {
    recsize=4+2;//hashcode+refcount
    inplacekey=K::Size()<=P::MaxInplaceKeySize;
    inplaceval=V::Size()<=P::MaxInplaceValueSize;
    recsize+=inplacekey?K::Size():P::ExpectBigKeyFileSize?8:4;
    recsize+=inplaceval?V::Size():P::ExpectBigValueFileSize?8:4;
  }

public:
  DiskHash()
  {
    CalcRecSize();
    fileopen=false;
  }

  void Open(const char* file,bool readonly=false)
  {
    name=file;
    if(readonly)
      f.ROpen(file);
    else
      f.RWOpen(file);
    f.SetUnbuffered();
    DiskHashHeader h;
    ReadHeader(h);
    if(h.magic!=_dh_magic)throw RTERROR("invalid file magic");
    if(h.ver!=_dh_ver)throw RTERROR("incompatible version!");
    uint32_t crc=crc32(0,&h,sizeof(h)-sizeof(uint32_t));
    if(crc!=h.hdrcrc32)throw RTERROR("header crc failire");
    size=h.size;
    count=h.count;
  }

  void Create(const char* file,int prealloc)
  {
    if(File::Exists(file))RTERROR("disk hash already exists");
    f.RWCreate(file);
    f.SetUnbuffered();
    DiskHashHeader h;
    h.magic=_dh_magic;
    h.ver=_dh_ver;
    h.count=0;
    h.size=prealloc;
    h.flags=0;
    h.flags|=inplacekey?flagInplaceKey:0;
    h.flags|=inplaceval?flagInplaceValue:0;
    h.hdrcrc32=crc32(0,&h,sizeof(h)-sizeof(uint32_t));
    WriteHeader(h);
    auto_ptr<char> buf(new char[recsize]);
    memset(buf.get(),0,recsize);
    for(int i=0;i<prealloc;i++)
    {
      f.Write(buf.get(),recsize);
    }
    size=h.size;
    count=0;
    fileopen=true;
  }
  void Close()
  {
    if(!fileopen)RTERROR("Attempt to close not opened file");
    h.flags|=flagClosedOk;
    h.count=count;
    WriteHeader(h);
    f.Close();
  }

  void Insert(const K& key,const V& value)
  {
    if(!fileopen)RTERROR("Attempt to insert into not opened hash file");
    if(!inplacekey || !inplaceval)RTERROR("Non inplace kv not implemented yet");
    int attempt=0;
    for(;;attempt++)
    {
      if(attempt>=size/2)throw RTERROR("Too many Hash recalc attempts");
      uint32_t hc=key.HashCode(attempt);
      uint32_t idx=(hc%size)*recsize;
      f.Seek(sizeof(DiskHashHeader)+idx);
      uint16_t fl=f.ReadNetInt16();
      if(fl)continue;
      f.Seek(sizeof(DiskHashHeader)+idx);
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
    if(!fileopen)RTERROR("Attempt to insert into not opened hash file");
    int attempt=0;
    for(;;attempt++)
    {
      uint32_t hc=key.HashCode(attempt);
      uint32_t idx=(hc%size)*recsize;
      f.Seek(sizeof(DiskHashHeader)+idx);
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
