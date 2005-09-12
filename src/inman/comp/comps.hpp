#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_COMPS_HPP__
#define __SMSC_INMAN_INAP_COMPS_HPP__

#include <vector>
#include <map>

namespace smsc {
namespace inman {
namespace comp{

struct InapOpCode
{
  enum
  {
    InitialDPSMS          = 60,
    FurnishChargingInformationSMS = 61,
    ConnectSMS            = 62,
    RequestReportSMSEvent     = 63,
    EventReportSMS          = 64,
    ContinueSMS           = 65,
    ReleaseSMS            = 66,
    ResetTimerSMS         = 67
  };
};

using std::vector;
using std::map;

class Component
{
  public:
    virtual int encode(vector<unsigned char>& buf) = 0;
    virtual int decode(const vector<unsigned char>& buf) = 0;
};

class InternalInitialDPSMSArg;
class InitialDPSMSArg: public Component
{
    public:
      InitialDPSMSArg();
      ~InitialDPSMSArg();
      void setDestinationSubscriberNumber();
      void setCallingPartyNumber();
      void setMode();
      void setIMSI();
      void setlocationInformationMSC();
      void setSMSCAddress();
      void setTimeAndTimezone();
      void setTPShortMessageSpecificInfo();
      void setTPProtocolIdentifier();
      void setTPDataCodingScheme();
      void setTPValidityPeriod();
    int encode(vector<unsigned char>& buf);
    int decode(const vector<unsigned char>& buf);
    private:
      InternalInitialDPSMSArg* internal;
};
class InternalRequestReportSMSEventArg;
class RequestReportSMSEventArg: public Component
{
 public:
   typedef enum EventTypeSMS {
     EventTypeSMS_sms_CollectedInfo  = 1,
     EventTypeSMS_o_smsFailure = 2,
     EventTypeSMS_o_smsSubmission  = 3,
     EventTypeSMS_sms_DeliveryRequested  = 11,
     EventTypeSMS_t_smsFailure = 12,
     EventTypeSMS_t_smsDelivery  = 13
     EventTypeSMS_t_NONE = 66;
   } EventTypeSMS_e;
   typedef enum MonitorMode {
     MonitorMode_interrupted = 0,
     MonitorMode_notifyAndContinue = 1,
     MonitorMode_transparent = 2
   } MonitorMode_e;
   struct SMSEvent {
     EventTypeSMS_e event;
     MonitorMode_e monitorType;
   };
   typedef vector<SMSEvent> SMSEventVector;
    public:
      RequestReportSMSEventArg();
      ~RequestReportSMSEventArg();
      const SMSEventVector& getSMSEvents();
    int encode(vector<unsigned char>& buf);
    int decode(const vector<unsigned char>& buf);
  private:
    InternalRequestReportSMSEventArg* internal;
};

class ConnectSMSArg: public Component{};
class FurnishChargingInformationSMSArg: public Component{};
class ReleaseSMSArg: public Component{};
class ResetTimerSMSArg: public Component{};
class EventReportSMSArg: public Component{};

}
}
}

#endif
