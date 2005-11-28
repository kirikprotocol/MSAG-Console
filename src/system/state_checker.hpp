#ifndef __SMSC_SYSTEM_STATE_CHECKER_HPP__
#define __SMSC_SYSTEM_STATE_CHECKER_HPP__

#include "smeman/smsccmd.h"
#include "sms/sms.h"

namespace smsc{
namespace system{

using namespace  smsc::smeman;

typedef int StateType;
typedef SmscCommand CommandType;

namespace StateTypeValue
{
  using namespace smsc::sms;
  const int ENROUTE_STATE = ENROUTE;
  const int DELIVERED_STATE= DELIVERED;
  const int EXPIRED_STATE=EXPIRED;
  const int UNDELIVERABLE_STATE=UNDELIVERABLE;
  const int DELETED_STATE=DELETED;

  const int UNKNOWN_STATE =    0x1000;
  const int ERROR_STATE =      0x8000;


  const int DELIVERING_STATE = 0x1001;

  const int CHARGING_STATE   = 0x1002;
  const int CHARGINGFWD_STATE= 0x1003;
}

using namespace StateTypeValue;


class StateChecker
{
public:
  static bool commandIsValid(StateType state,CommandType cmd)
  {
    switch(state)
    {
      case UNKNOWN_STATE:return true;
      case ENROUTE_STATE:
      {
        switch(cmd->cmdid)
        {
          case FORWARD:return true;
          case REPLACE:return true;
          case QUERY:return true;
          case CANCEL:return true;
          case SUBMIT:return true;
          default:return false;
        }
      }
      case DELIVERED_STATE:
      {
        switch(cmd->cmdid)
        {
          case QUERY:return true;
          default:return false;
        }
      }
      case EXPIRED_STATE:return false;
      case UNDELIVERABLE_STATE:return false;
      case DELETED_STATE:return false;
      case DELIVERING_STATE:
      {
        switch(cmd->cmdid)
        {
          case DELIVERY_RESP://так надо! :)
          case ALERT:return true;
          default:return false;
        }
      }
      case CHARGING_STATE:
      {
        return cmd->cmdid==INSMSCHARGERESPONSE;
      }
      case CHARGINGFWD_STATE:
      {
        return cmd->cmdid==INFWDSMSCHARGERESPONSE;
      }
    }
    return true;
  }
  static bool commandHasTimeout(CommandType cmd)
  {
    return cmd->cmdid!=ALERT && cmd->cmdid!=DELIVERY_RESP && cmd->cmdid!=FORWARD;
  }
  static bool stateIsFinal(StateType state)
  {
    return state!=DELIVERING_STATE && state!=CHARGING_STATE && state!=CHARGINGFWD_STATE;
  }
  static bool stateIsSuperFinal(StateType state)
  {
    return state==DELIVERED_STATE || state==UNDELIVERABLE_STATE ||
           state==DELETED_STATE || state==EXPIRED_STATE;
  }
};

}//system
}//smsc

#endif
