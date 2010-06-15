#ifndef SMSC_ACLS_INTERFACES_H
#define SMSC_ACLS_INTERFACES_H

#include <string>
#include <vector>
#include <algorithm>
#include <functional>

#include "util/int.h"

namespace smsc {
namespace acls {

typedef std::string    AclPhoneNumber;
inline const std::string& AclPhoneAsString(const AclPhoneNumber& a) { return a; }
inline const AclPhoneNumber MakeAclPhoneNumber(const std::string& val) { return val; }

typedef std::string    AclName;
typedef std::string    AclDescription;
typedef uint32_t  AclIdent;

typedef const char*    AclPChar;
typedef std::pair<AclIdent,AclName> AclNamedIdent;
inline AclNamedIdent MakeAclNamedIdent(AclIdent argIdent,const AclName& name)
{ return AclNamedIdent(argIdent,name); }

enum AclCacheType {
  ACT_UNKNOWN   = '0',
  ACT_DBSDIRECT = '1',
  ACT_FULLCACHE = '2'
};
inline char AclCacheTypeAsChar(AclCacheType act) { return char(act); }
inline AclCacheType MakeAclCacheType(char act) { return AclCacheType(act); }

struct AclInfo
{
  AclIdent        ident;
  AclName         name;
  AclDescription  description;
  AclCacheType    cache;
};
inline AclInfo MakeAclInfo(AclIdent argIdent,const char * name,const char * desc,AclCacheType cache)
{
  if (name == NULL)
    name = "";
  if (desc == NULL)
    desc = "";
  AclInfo aclinfo = { argIdent, name,desc,cache };
  return aclinfo;
}


struct AclEditor
{
  virtual void    enumerate(std::vector<AclNamedIdent>& result) = 0;
  virtual AclInfo getInfo(AclIdent) = 0;
  virtual void    remove(AclIdent) = 0;
  virtual void    create(
                        AclIdent,
                        const char* aclname,
                        const char* descr,
                        const std::vector<AclPhoneNumber>& phones = std::vector<AclPhoneNumber>(),
                        AclCacheType act = ACT_DBSDIRECT) = 0;
  virtual AclIdent create2(
                        const char* aclname,
                        const char* descr,
                        const std::vector<AclPhoneNumber>& phones = std::vector<AclPhoneNumber>(),
                        AclCacheType act = ACT_DBSDIRECT) = 0;
  virtual void    lookupByPrefix(AclIdent,const char* prefix,std::vector<AclPhoneNumber>&) = 0;
  virtual void    removePhone(AclIdent,const AclPhoneNumber&) = 0;
  virtual void    addPhone(AclIdent,const AclPhoneNumber&) = 0;
  virtual void    updateAclInfo(
                        AclIdent,
                        const char* aclname,
                        const char* acldesc,
                        AclCacheType act = ACT_UNKNOWN) = 0;
protected:
  AclEditor() {}
  virtual ~AclEditor() {}
private:
  AclEditor(const AclEditor&);
  AclEditor& operator = (const AclEditor&);
};

struct AclLookuper
{
  virtual bool isGranted(AclIdent aclid,const AclPhoneNumber& phone) = 0;
protected:
  AclLookuper() {}
  virtual ~AclLookuper() {}
private:
  AclLookuper(const AclLookuper&);
  AclLookuper& operator = (const AclLookuper&);
};

struct AclAbstractMgr : public AclEditor, public AclLookuper
{
  //virtual void AddRef() = 0;
  //virtual void Release() = 0;
  virtual void LoadUp(const char* argAclStore,int argPreCreate) = 0;
  virtual void enableControllerMode()=0;
  //virtual void LoadUp(smsc::db::DataSource*) = 0;
  //virtual void Relax() = 0;
  //protected:
  virtual ~AclAbstractMgr() {}
  static AclAbstractMgr* Create();
  static AclAbstractMgr* Create2();

};

} // acls namespace
} // smsc namespace

#endif //#ifndef SMSC_ACLS_INTERFACES_H
