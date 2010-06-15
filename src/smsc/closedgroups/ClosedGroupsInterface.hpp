#ifndef __SMSC_CLOSEDGROUPS_CLOSEDGROUPSINTERFACE_HPP__
#define __SMSC_CLOSEDGROUPS_CLOSEDGROUPSINTERFACE_HPP__

#include <vector>

#include "sms/sms.h"

namespace smsc{
namespace closedgroups{

class ClosedGroupsInterface{
public:

  virtual void AddGroup(int id,const char* name)=0;
  virtual void DeleteGroup(int id)=0;
  virtual void AddAddrToGroup(int id,const smsc::sms::Address& addr)=0;
  virtual bool Check(int groupId,const smsc::sms::Address& addr)const=0;
  virtual const char* GetClosedGroupName(int id)const=0;
  virtual void RemoveAddrFromGroup(int id,const smsc::sms::Address& addr)=0;

  virtual void AddAbonent(int id,const smsc::sms::Address& addr)=0;
  virtual void RemoveAbonent(int id,const smsc::sms::Address& addr)=0;

  virtual void ListAbonents(int id,std::vector<smsc::sms::Address>& list)=0;

  virtual void Load(const char* filename)=0;

  virtual void enableControllerMode()=0;

  static ClosedGroupsInterface* getInstance()
  {
    return instance;
  }


protected:
  static ClosedGroupsInterface* instance;
};

}//namespace closedgroups
}//namespace smsc

#endif
