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

};

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
    }
    return true;
  }
  static bool commandHasTimeout(CommandType cmd)
  {
    return cmd->cmdid!=ALERT && cmd->cmdid!=DELIVERY_RESP;
  }
  static bool stateIsFinal(StateType state)
  {
    return state!=DELIVERING_STATE;
  }
  static bool stateIsSuperFinal(StateType state)
  {
    return state==DELIVERED_STATE || state==UNDELIVERABLE_STATE ||
           state==DELETED_STATE || state==EXPIRED_STATE;
  }
};

};//system
};//smsc

#endif
