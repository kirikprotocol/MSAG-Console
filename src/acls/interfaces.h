
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
typedef pair<AclIdent,AclName> AclNamedIdent; 

enum AclCacheType { 
  ACT_UNKNOWN,
  ACT_DBSDIRECT,
  ACT_FULLCACHE,
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
  virtual void    enumerate(vector<AclNamedIdent>& result) = 0;
  virtual AclInfo getInfo(AclIdent) = 0;  
  virtual bool    remove(AclIdent) = 0;
  virtual void    create(AclIdent,const char* aclname,const char* descr,const vector<AclPhoneNumber>& phones = vector<AclPhoneNumber>(),AclCacheType act = ACT_DBSDIRECT) = 0;
  virtual bool    lookupByPrefix(AclIdent,const char* prefix,vector<AclPhoneNumber>&) = 0;
  virtual bool    removePhone(AclIdent,const AclPhoneNumber&) = 0;
  virtual void    addPhone(AclIdent,const AclPhoneNumber&) = 0;
  virtual void    updateAclInfo(AclIdent,const char* aclname,const char* acldesc,AclCacheType act = ACT_UNKNOWN) = 0;
};

struct AclLookuper
{
  virtual bool isGranted(AclIdent aclid,const AclPhoneNumber& phone) = 0;
};

} // acls namespace
} // smsc namespace
