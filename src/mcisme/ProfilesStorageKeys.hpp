//------------------------------------
//  ProfilesStorageKeys.hpp
//  Changed by Routman Michael, 2005-2006
//------------------------------------


#ifndef ___PROFILESSTORAGEKEYS_H
#define ___PROFILESSTORAGEKEYS_H

#include "util/int.h"
#include "core/buffers/File.hpp"
#include "util/crc32.h"

namespace smsc{
namespace mcisme{

using smsc::core::buffers::File;
using smsc::util::crc32;

union AbntProf
{
  int32_t prof;
  struct _fields
  {
    int8_t eventMask;
    int8_t informTemplateId;
    int8_t notifyTemplateId;
    int8_t inform:1;
    int8_t notify:1;
    int8_t wantNotifyMe:1;
    int8_t reserved:5;
  } fields;
};

struct AbntProfKey{
  AbntProf	key;

  AbntProfKey(){key.prof=0;}

  AbntProfKey(uint32_t _key){key.prof=_key;}

  AbntProfKey(uint8_t eventMask, uint8_t informTemplateId, uint8_t notifyTemplateId, bool inform, bool notify, bool wantNotifyMe)
  {
    key.fields.eventMask = eventMask;
    key.fields.informTemplateId = informTemplateId;
    key.fields.notifyTemplateId = notifyTemplateId;
    key.fields.inform = inform;
    key.fields.notify = notify;
    key.fields.wantNotifyMe = wantNotifyMe;
  }

  uint32_t Get()const{return key.prof;}

  static uint32_t Size(){return static_cast<uint32_t>(sizeof(AbntProf));}

  void Read(File& f)
  {
    key.prof=f.ReadNetInt32();
  }

  void Write(File& f)const
  {
    f.WriteNetInt32(key.prof);
  }

  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res=0;
    res=crc32(res,&key.prof,sizeof(key.prof));
    for(;attempt>0;attempt--)res=crc32(res,&key.prof,sizeof(key.prof));
    return res;
  }

  bool operator==(const AbntProfKey& cmp)
  {
    return key.prof==cmp.key.prof;
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
    uint32_t res=0;
    res=crc32(res,str,len);
    for(;attempt>0;attempt--)res=crc32(res,str,len);
    return res;
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
    uint32_t res=0;
    res=crc32(res,&key,sizeof(key));
    res=crc32(res,&ltt,sizeof(ltt));
    for(;attempt>0;attempt--)
    {
      res=crc32(res,&key,sizeof(key));
      res=crc32(res,&ltt,sizeof(ltt));
    }
    return res;
  }
  bool operator==(const IdLttKey& cmp)
  {
    return key==cmp.key && ltt==cmp.ltt;
  }
  bool operator<(const IdLttKey& cmp)const
  {
    return key<cmp.key?true:ltt<cmp.ltt;
  }
};

}
}

#endif

