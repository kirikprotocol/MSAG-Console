#include "system/smsc.hpp"
#include "system/alert_agent.hpp"

namespace smsc{
namespace system{

int AlertAgent::Execute()
{
  mon.Lock();
  Array<SMSId> ids;
  while(!isStopping)
  {
    if(queue.Count()==0)mon.wait();
    if(queue.Count()==0)continue;
    SmscCommand cmd;
    queue.Pop(cmd);
    mon.Unlock();
    ////
    // processing here

    try{
      smsc::store::IdIterator *it=store->getReadyForDelivery(cmd->get_address());
      SMSId id;
      while(it->getNextId(id))
      {
        ids.Push(id);
      }
      delete it;
      __trace2__("AlertAgent: found %d messages for %s",ids.Count(),cmd->get_address().toString().c_str());
      time_t now=time(NULL);
      for(int i=0;i<ids.Count();i++)
      {
        try{
          SMS s;
          store->retriveSms(ids[i],s);
          SmeIndex idx=psmsc->getSmeIndex(s.dstSmeId);
          psmsc->UpdateSmsSchedule(s.getNextTime(),ids[i],now,idx);
        }catch(...)
        {
          __warning2__("AlertAgent: failed to retrieve sms: %lld",ids[i]);
        }
      }
      ids.Empty();
    }catch(...)
    {
      __trace__("AlertAgent: database exception");
    }

    // end of processing
    ////
    mon.Lock();
  }
  mon.Unlock();
  return 0;
}

}//system
}//smsc
