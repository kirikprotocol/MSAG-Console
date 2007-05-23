#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/comp/cap_sms/CapSMSComps.hpp"

namespace smsc {
namespace inman {
namespace comp {

const char * _nmMonitorMode(enum MonitorMode m_mode)
{
    switch (m_mode) {
    case MonitorMode_interrupted:       return "interrupted(0)";
    case MonitorMode_notifyAndContinue: return "notifyAndContinue(1)";
    case MonitorMode_transparent:       return "transparent(2)";
    default:;
    }
    return "unknown";
}

const char * _nmEventTypeSMS(enum EventTypeSMS event)
{
    switch (event) {
    case EventTypeSMS_sms_CollectedInfo:    return "sms_CollectedInfo(1)";
    case EventTypeSMS_o_smsFailure:         return "o_smsFailure(2)";
    case EventTypeSMS_o_smsSubmission:      return "o_smsSubmission(3)";
    case EventTypeSMS_sms_DeliveryRequested: return "sms_DeliveryRequested(11)";
    case EventTypeSMS_t_smsFailure:         return "t_smsFailure(12)";
    case EventTypeSMS_t_smsDelivery:        return "t_smsDelivery(13)";
    case EventTypeSMS_t_NONE:               return "t_NONE(66)";
    default:;
    }
    return "unknown";
}

const char * _nmMessageType(enum messageType msg_type)
{
    switch (msg_type) {
    case MessageType_request:	    return "request(0)";
    case MessageType_notification:  return "notification(1)";
    default:;
    }
    return "unknown";
}

}//namespace comps
}//namespace inman
}//namespace smsc

