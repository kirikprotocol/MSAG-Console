#ifndef __SMSC_CLOSEDGROUPS_CLOSEDGROUPSMANAGER_HPP__
#define __SMSC_CLOSEDGROUPS_CLOSEDGROUPSMANAGER_HPP__

#include <set>
#include <map>

#include "core/buffers/FixedLengthString.hpp"
#include "sms/sms.h"
#include "core/synchronization/Mutex.hpp"
#include "util/Exception.hpp"
#include "ClosedGroupsInterface.hpp"
#include "logger/Logger.h"

namespace smsc{
namespace closedgroups{

namespace sync=smsc::core::synchronization;
namespace buf=smsc::core::buffers;

class ClosedGroupsManager:public ClosedGroupsInterface{
public:
  ClosedGroupsManager()
  {
    allowFileModification=false;
    log=smsc::logger::Logger::getInstance("cgm");
  }
  static void Init()
  {
    instance=new ClosedGroupsManager();
  }
  static void Shutdown()
  {
    delete instance;
    instance=0;
  }

  void enableControllerMode()
  {
    allowFileModification=true;
  }

  virtual void Load(const char* filename);

  virtual void AddGroup(int id,const char* name)
  {
    sync::MutexGuard mg(mtx);
    smsc_log_debug(log,"add group %d,%s",id,name);
    GroupsMap::iterator it=groups.lower_bound(id);
    if(it->first==id)
    {
      throw smsc::util::Exception("AddGroup: Group with id=%d already exists",id);
    }
    Group* grp=new Group;
    grp->name=name;
    grp->id=id;
    groups.insert(it,GroupsMap::value_type(id,grp));
  }
  virtual void DeleteGroup(int id)
  {
    sync::MutexGuard mg(mtx);
    GroupsMap::iterator it=groups.find(id);
    if(it==groups.end())
    {
      throw smsc::util::Exception("DeleteGroup: Group with id=%d doesn't exists",id);
    }
    smsc_log_debug(log,"delete group %d, abonents.size=%lu",id,it->second->abonents.size());
    if(it->second->abonents.size()!=0)
    {
      throw smsc::util::Exception("DeleteGroup: cannot delete group with id=%d(%s), group is not empty",id,it->second->name.c_str());
    }
    delete it->second;
    groups.erase(it);
  }
  virtual void AddAddrToGroup(int id,const smsc::sms::Address& addr)
  {
    sync::MutexGuard mg(mtx);
    GroupsMap::iterator it=groups.find(id);
    if(it==groups.end())
    {
      throw smsc::util::Exception("AddAddrToGroup: Group with id=%d doesn't exists",id);
    }
    smsc_log_debug(log,"add addr to group %d (%s)",id,addr.toString().c_str());
    it->second->addresses.insert(addr);
  }
  virtual void RemoveAddrFromGroup(int id,const smsc::sms::Address& addr)
  {
    sync::MutexGuard mg(mtx);
    GroupsMap::iterator it=groups.find(id);
    if(it==groups.end())
    {
      throw smsc::util::Exception("RemoveAddrFromGroup: Group with id=%d doesn't exists",id);
    }
    Group::MaskSet::iterator mit=it->second->addresses.find(addr);
    if(mit==it->second->addresses.end())
    {
      throw smsc::util::Exception("RemoveAddrFromGroup: address %s doesn't exists in group wityh id=%d(%s)",addr.toString().c_str(),id,it->second->name.c_str());
    }
    smsc_log_debug(log,"remove addr from group %d (%s)",id,addr.toString().c_str());
    it->second->addresses.erase(mit);
  }

  virtual bool Check(int groupId,const smsc::sms::Address& addr)const
  {
    sync::MutexGuard mg(mtx);
    GroupsMap::const_iterator it=groups.find(groupId);
    if(it==groups.end())return false;
    return it->second->addresses.find(addr)!=it->second->addresses.end();
  }

  virtual const char* GetClosedGroupName(int id)const
  {
    sync::MutexGuard mg(mtx);
    GroupsMap::const_iterator it=groups.find(id);
    if(it==groups.end())return "<UNKNOWN CLOSEDGROUP>";
    return it->second->name.c_str();
  }

  virtual void AddAbonent(int id,const smsc::sms::Address& addr)
  {
    sync::MutexGuard mg(mtx);
    GroupsMap::iterator it=groups.find(id);
    if(it==groups.end())
    {
      throw smsc::util::Exception("AddAbonent: Group with id=%d doesn't exists",id);
    }
    Group::AddrSet::iterator ait=it->second->abonents.find(addr);
    if(ait!=it->second->abonents.end())
    {
      throw smsc::util::Exception("AddAbonent: address %s already exists in group wityh id=%d(%s)",addr.toString().c_str(),id,it->second->name.c_str());
    }
    smsc_log_debug(log,"add abonent to group %d (%s)",id,addr.toString().c_str());
    it->second->abonents.insert(addr);
  }
  virtual void RemoveAbonent(int id,const smsc::sms::Address& addr)
  {
    sync::MutexGuard mg(mtx);
    GroupsMap::iterator it=groups.find(id);
    if(it==groups.end())
    {
      throw smsc::util::Exception("RemoveAbonent: Group with id=%d doesn't exists",id);
    }
    Group::AddrSet::iterator ait=it->second->abonents.find(addr);
    if(ait==it->second->abonents.end())
    {
      throw smsc::util::Exception("RemoveAbonent: address %s doesn't exists in group wityh id=%d(%s)",addr.toString().c_str(),id,it->second->name.c_str());
    }
    smsc_log_debug(log,"remove abonent from group %d (%s)",id,addr.toString().c_str());
    it->second->abonents.erase(ait);
  }
  virtual void ListAbonents(int id,std::vector<smsc::sms::Address>& list)
  {
    sync::MutexGuard mg(mtx);
    GroupsMap::iterator it=groups.find(id);
    if(it==groups.end())
    {
      throw smsc::util::Exception("ListAbonents: Group with id=%d doesn't exists",id);
    }
    list.insert(list.begin(),it->second->abonents.begin(),it->second->abonents.end());
  }
protected:
  struct MaskComparator{
    bool operator()(const smsc::sms::Address& addra,const smsc::sms::Address& addrb)const
    {
      if(addra.type<addrb.type)return true;
      if(addra.type>addrb.type)return false;
      if(addra.plan<addrb.plan)return true;
      if(addra.plan>addrb.plan)return false;
      const char* a=addra.value;
      const char* b=addrb.value;
      while(*a && *b)
      {
        if(*a=='?' || *b=='?')return addra.length<addrb.length;//strlen(a)<strlen(b);
        if(*a<*b)return true;
        if(*a>*b)return false;
        a++;
        b++;
      }
      return !*a && *b;
    }
  };
  struct Group;
  friend struct Group;
  struct Group{
    int id;
    buf::FixedLengthString<64> name;
    typedef std::multiset<smsc::sms::Address,MaskComparator> MaskSet;
    typedef std::multiset<smsc::sms::Address> AddrSet;
    MaskSet addresses;
    AddrSet abonents;
  };
  typedef std::map<int,Group*> GroupsMap;
  GroupsMap groups;
  mutable sync::Mutex mtx;
  bool allowFileModification;
  smsc::logger::Logger* log;

};

}
}

#endif
