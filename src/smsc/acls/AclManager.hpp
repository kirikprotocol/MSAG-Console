/*
 * AclManager.hpp
 *
 *  Created on: Jun 25, 2010
 *      Author: skv
 */

#ifndef __SMSC_ACL_ACLMANAGER_HPP__
#define __SMSC_ACL_ACLMANAGER_HPP__

#include "core/buffers/FixedLengthString.hpp"
#include "util/int.h"
#include <vector>
#include <string>
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/FixedRecordFile.hpp"
#include <set>
#include <map>


namespace smsc{
namespace acl{

namespace buf=smsc::core::buffers;
namespace sync=smsc::core::synchronization;

typedef buf::FixedLengthString<32> AclPhoneNumber;
typedef buf::FixedLengthString<32> AclName;
typedef buf::FixedLengthString<128> AclDescription;
typedef uint32_t  AclIdent;

struct AclInfo
{
  AclIdent        ident;
  AclName         name;
  AclDescription  description;

  buf::File::offset_type offset;

  void Read(buf::File& f)
  {
    ident=f.ReadNetInt32();
    f.Read(name.str,32);
    f.Read(description.str,128);
  }
  void Write(buf::File& f)const
  {
    f.WriteNetInt32(ident);
    f.Write(name.c_str(),32);
    f.Write(description.c_str(),128);
  }

  static size_t Size()
  {
    return 4+32+128;
  }

};

struct AclRecord{
  AclPhoneNumber number;

  AclRecord(){}
  AclRecord(const AclPhoneNumber& argNumber):number(argNumber)
  {

  }
  buf::File::offset_type offset;

  bool operator<(const AclRecord& rhs)const
  {
    return number<rhs.number;
  }

  void Read(buf::File& f)
  {
    f.Read(number.str,32);
  }
  void Write(buf::File& f)const
  {
    f.Write(number.str,32);
  }
  static size_t Size()
  {
    return 32;
  }
};


class AclStore
{
public:
  AclStore();
  ~AclStore();

  static void Init();
  static void Shutdown();
  static AclStore* getInstance()
  {
    return instance;
  }

  void    enumerate(std::vector<AclInfo>& result);
  AclInfo getInfo(AclIdent ident);
  void    remove(AclIdent ident);
  void    create(AclIdent ident,const char* aclname,const char* descr,const std::vector<std::string>& phones);
  AclIdent create2(const char* aclname,const char* descr,const std::vector<std::string>& phones);
  void    lookupByPrefix(AclIdent ident,const char* prefix,std::vector<AclPhoneNumber>& result);
  void    removePhone(AclIdent ident,const AclPhoneNumber& phone);
  void    addPhone(AclIdent ident,const AclPhoneNumber& phone);
  void    updateAclInfo(AclIdent ident,const char* aclname,const char* acldesc);

  bool isGranted(AclIdent aclid,const AclPhoneNumber& phone);

  void Load(const char* argAclStoreDir);
  void enableControllerMode();

protected:
  bool allowDiskWrites;
  buf::FixedRecordFile<AclInfo> aclStoreFile;
  std::string path;

  static AclStore* instance;

  std::string mkStoreFileName(AclIdent ident)
  {
    char tmp[32];
    sprintf(tmp,"acl%04d.bin",ident);
    return path+tmp;
  }

  sync::Mutex mtx;

  uint32_t maxIdent;

  struct AclData{
    AclData():store("ACLDATA",0x01){}
    AclInfo info;
    typedef std::set<AclRecord> AclSet;
    AclSet numbers;
    buf::FixedRecordFile<AclRecord> store;
  };

  typedef std::map<AclIdent,AclData*> AclMap;
  AclMap aclMap;

};

}
}

#endif /* __SMSC_ACL_ACLMANAGER_HPP__ */
