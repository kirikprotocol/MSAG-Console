
#include <string>
#include <vector>
#include <algorithm>
#include <functional>

namespace smsc {
namespace acls {

typedef std::string    AclPhoneNumber;
typedef std::string    AclName;
typedef std::string    AclDescription;
typedef unsigned long  AclIdent;
typedef const char*    AclPChar;
typedef std::pair<AclIdent,AclName> AclNamedIdent; 

enum AclCacheType { 
  ACT_UNKNOWN   = '0',
  ACT_DBSDIRECT = '1',
  ACT_FULLCACHE = '2'
};

struct AclInfo
{
  AclIdent        ident;
  AclName         name;
  AclDescription  desctiption;
  AclCacheType    cache;
};

struct AclEditor
{
  virtual void    enumerate(std::vector<AclNamedIdent>& result) = 0;
  virtual AclInfo getInfo(AclIdent) = 0;  
  virtual bool    remove(AclIdent) = 0;
  virtual void    create(
                        AclIdent,
                        const char* aclname,
                        const char* descr,
                        const std::vector<AclPhoneNumber>& phones = std::vector<AclPhoneNumber>(),
                        AclCacheType act = ACT_DBSDIRECT) = 0;
  virtual bool    lookupByPrefix(AclIdent,const char* prefix,std::vector<AclPhoneNumber>&) = 0;
  virtual bool    removePhone(AclIdent,const AclPhoneNumber&) = 0;
  virtual void    addPhone(AclIdent,const AclPhoneNumber&) = 0;
  virtual void    updateAclInfo(
                        AclIdent,
                        const char* aclname,
                        const char* acldesc,
                        AclCacheType act = ACT_UNKNOWN) = 0;
};

struct AclLookuper
{
  virtual bool isGranted(AclIdent aclid,const AclPhoneNumber& phone) = 0;
};

} // acls namespace
} // smsc namespace
