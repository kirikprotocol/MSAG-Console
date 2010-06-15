#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <functional>
#include <memory>

#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

#include "interfaces.h"

#define ACLMGRPREFIX "Acl2:"

namespace smsc {
namespace acls {

using namespace smsc::util;
using namespace smsc::util::config;
using namespace smsc::core::threads;
using namespace smsc::core::synchronization;
using namespace smsc::util;
//using namespace smsc::db;

using namespace std;

typedef smsc::util::config::Manager ConfigManager;

inline uint64_t AclPhoneToInt64(const AclPhoneNumber& phone)
{
  uint64_t result;
  if ( 1 != sscanf(phone.c_str(),"+%lld",&result) )
    if (1 != sscanf(phone.c_str(),"%lld",&result) )
      throw std::runtime_error(string("AclPhoneToInt64 broken on '")+phone+"'");
  return result;
}

struct AclRecord
{
  Mutex&                 refcount_locker_;
  AclInfo                info_;
  unsigned               usecount_;
  vector<uint64_t>       granted_cache_;
  void Release() {
    MutexGuard g(refcount_locker_);
    if ( --usecount_ == 0 ) delete this;
  }
  AclRecord* AddRef()  {
    MutexGuard g(refcount_locker_);
    ++usecount_;
    return this;
  }
  AclRecord(Mutex& locker) : refcount_locker_(locker) {
    usecount_ = 1;
  }
  void LoadFrom(AclInfo info, AclAbstractMgr* mgr)
  {
    info_ = info;
    vector<AclPhoneNumber> temp;
    mgr->lookupByPrefix(info.ident,0,temp);
    granted_cache_.resize(temp.size());
    for ( vector<AclPhoneNumber>::iterator i = temp.begin(), iE = temp.end(); i != iE; ++i )
      granted_cache_.push_back(AclPhoneToInt64(*i));
    sort(granted_cache_.begin(),granted_cache_.end());
  }
};

struct AclRecordGuard
{
  AclRecord* record_;
  AclRecordGuard(AclRecord* rec = 0) : record_(rec) {}
  AclRecordGuard(const AclRecordGuard& x) : record_(x.record_?x.record_->AddRef():0) {}
  AclRecordGuard& operator=(const AclRecordGuard& x) { record_ = x.record_?x.record_->AddRef():0; return *this; }
  ~AclRecordGuard() { if ( record_ ) { record_->Release(); record_ = 0; } }
  AclRecord* forget__() { AclRecord* temp = record_; record_ = 0; return temp; }
  AclRecord* operator ->() { return record_; }
  AclRecord* operator ~() { return record_; }
};

class AclManager2 : public AclAbstractMgr
{
  auto_ptr<AclAbstractMgr> mgr_;
  typedef map<AclIdent,AclRecordGuard> records_t;
  records_t records_;

  Mutex modify_locker_;
  Mutex records_locker_;
  Mutex locker_;

  AclRecordGuard GetCachedAcl(AclIdent id)
  {
    MutexGuard g(records_locker_);
    records_t::iterator i = records_.find(id);
    if ( i == records_.end() ) return AclRecordGuard(0);
    else return (*i).second;
  }

  void SetCachedAcl(AclRecordGuard arg)
  {
    MutexGuard g(records_locker_);
    records_t::iterator i = records_.find(arg->info_.ident);
    if ( i != records_.end() )
      std::swap((*i).second,arg);
    else
      records_.insert(records_t::value_type(arg->info_.ident,arg));
  }

  void UncacheAcl(AclIdent id)
  {
    MutexGuard g(records_locker_);
    records_t::iterator i = records_.find(id);
    if ( i != records_.end() ) records_.erase(i);
  }

  void UpdateAcl(AclIdent);

public:

  AclManager2() {
    mgr_ = auto_ptr<AclAbstractMgr>(AclAbstractMgr::Create());
  }

  //struct AclEditor
  virtual void    enumerate(vector<AclNamedIdent>& result);
  virtual AclInfo getInfo(AclIdent);
  virtual void    remove(AclIdent);
  virtual void    create(AclIdent,const char* aclname,const char* descr,const vector<AclPhoneNumber>& phones,AclCacheType act);
  virtual AclIdent create2(const char* aclname,const char* descr,const vector<AclPhoneNumber>& phones,AclCacheType act);
  virtual void    lookupByPrefix(AclIdent,const char* prefix,vector<AclPhoneNumber>&);
  virtual void    removePhone(AclIdent,const AclPhoneNumber&);
  virtual void    addPhone(AclIdent,const AclPhoneNumber&);
  virtual void    updateAclInfo(AclIdent,const char* aclname,const char* acldesc,AclCacheType act);
  //struct AclLookuper
  virtual bool    isGranted(AclIdent aclid,const AclPhoneNumber& phone);
  // struct AclAbstractManager
  virtual void LoadUp(const char* argAclStore,int argPreCreate);
  virtual void enableControllerMode()
  {
    mgr_->enableControllerMode();
  }
};

void AclManager2::UpdateAcl(AclIdent aid)
{
  AclInfo aclinfo = mgr_->getInfo(aid);
  if ( aclinfo.cache == ACT_FULLCACHE ) {
    AclRecordGuard arg ( new AclRecord(locker_) );
    arg->LoadFrom(aclinfo,mgr_.get());
    SetCachedAcl(arg);
  }
  else
    UncacheAcl(aclinfo.ident);
}

void AclManager2::enumerate(vector<AclNamedIdent>& result)
{
  mgr_->enumerate(result);
}

AclInfo AclManager2::getInfo(AclIdent aclident)
{
  AclRecordGuard aclrec = GetCachedAcl(aclident);
  if ( ~aclrec ) return aclrec->info_;
  else return mgr_->getInfo(aclident);
}

void AclManager2::remove(AclIdent aclident)
{
  MutexGuard g(modify_locker_);
  mgr_->remove(aclident);
  UncacheAcl(aclident);
}

void AclManager2::create(AclIdent aclident,const char* aclname,const char* acldescr,const vector<AclPhoneNumber>& phones,AclCacheType act)
{
  MutexGuard g(modify_locker_);
  mgr_->create(aclident,aclname,acldescr,phones,act);
  UpdateAcl(aclident);
}

AclIdent AclManager2::create2(const char* aclname,const char* acldescr,const vector<AclPhoneNumber>& phones,AclCacheType act)
{
  MutexGuard g(modify_locker_);
  AclIdent ident = mgr_->create2(aclname,acldescr,phones,act);
  UpdateAcl(ident);
  return ident;
}

void AclManager2::lookupByPrefix(AclIdent aclident,const char* prefix,vector<AclPhoneNumber>& result)
{
  mgr_->lookupByPrefix(aclident,prefix,result);
}

void AclManager2::removePhone(AclIdent aclident,const AclPhoneNumber& phone)
{
  MutexGuard g(modify_locker_);
  mgr_->removePhone(aclident,phone);
  UpdateAcl(aclident);
}

void AclManager2::addPhone(AclIdent aclident,const AclPhoneNumber& phone)
{
  MutexGuard g(modify_locker_);
  mgr_->addPhone(aclident,phone);
  UpdateAcl(aclident);
}

void AclManager2::updateAclInfo(AclIdent aclident,const char* aclname,const char* acldesc,AclCacheType act)
{
  MutexGuard g(modify_locker_);
  mgr_->updateAclInfo(aclident,aclname,acldesc,act);
  UpdateAcl(aclident);
}

bool AclManager2::isGranted(AclIdent aclid,const AclPhoneNumber& phone)
{
  AclRecordGuard aclrec = GetCachedAcl(aclid);
  if ( ~aclrec ) {
    //printf("%s is in cached acl\n",phone.c_str());
    uint64_t ph = AclPhoneToInt64(phone);
    vector<uint64_t>::iterator i = lower_bound(aclrec->granted_cache_.begin(),aclrec->granted_cache_.end(),ph);
    return i != aclrec->granted_cache_.end() && *i == ph;
  }
  else
    return mgr_->isGranted(aclid,phone);
}

void AclManager2::LoadUp(const char* argAclStore,int argPreCreate)
{
  MutexGuard g(modify_locker_);
  mgr_->LoadUp(argAclStore,argPreCreate);
  vector<AclNamedIdent> acls;
  mgr_->enumerate(acls);
  for ( vector<AclNamedIdent>::iterator i = acls.begin(), iE = acls.end(); i != iE; ++i )
    UpdateAcl((*i).first);
}

AclAbstractMgr* AclAbstractMgr::Create2()
{
  return new AclManager2;
}

} // acls namespace
} // smsc namespace
