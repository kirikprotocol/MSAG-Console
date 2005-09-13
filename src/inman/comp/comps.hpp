#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_COMPS_HPP__
#define __SMSC_INMAN_INAP_COMPS_HPP__

#include <vector>
#include <map>
#include <stdexcept>
#include "inman/common/util.hpp"

using std::runtime_error;
using smsc::inman::common::format;

namespace smsc {
namespace inman {
namespace comp{


typedef std::runtime_error EncodeError;
typedef std::runtime_error DecodeError;

struct InapOpCode
{
	enum
	{
		InitialDPSMS 				  = 0x3C, // 60
		FurnishChargingInformationSMS = 0x3D, // 61
		ConnectSMS					  = 0x3E, // 62
		RequestReportSMSEvent		  = 0x3F, // 63
		EventReportSMS				  = 0x40, // 64
		ContinueSMS					  = 0x41, // 65
		ReleaseSMS					  = 0x42, // 66
		ResetTimerSMS				  = 0x43  // 67
	};
};

using std::vector;
using std::map;

class Component
{
  public:
    virtual void encode(vector<unsigned char>& buf) = 0;
    virtual void decode(const vector<unsigned char>& buf) = 0;
};

typedef enum DeliveryMode
{
	DeliveryMode_Originating,
	DeliveryMode_Terminating
} DeliveryMode_e;

class InternalInitialDPSMSArg;
class InitialDPSMSArg: public Component
{
    public:

      InitialDPSMSArg();
      ~InitialDPSMSArg();
      void setDestinationSubscriberNumber();
      void setCallingPartyNumber();
      void setMode(DeliveryMode_e mode);
      void setIMSI();
      void setlocationInformationMSC();
      void setSMSCAddress();
      void setTimeAndTimezone();
      void setTPShortMessageSpecificInfo();
      void setTPProtocolIdentifier();
      void setTPDataCodingScheme();
      void setTPValidityPeriod();
    public:
      void encode(vector<unsigned char>& buf);
      void decode(const vector<unsigned char>& buf);
    private:
      InternalInitialDPSMSArg* internal;
};

typedef enum EventTypeSMS 
{
     EventTypeSMS_sms_CollectedInfo  = 1,
     EventTypeSMS_o_smsFailure = 2,
     EventTypeSMS_o_smsSubmission  = 3,
     EventTypeSMS_sms_DeliveryRequested  = 11,
     EventTypeSMS_t_smsFailure = 12,
     EventTypeSMS_t_smsDelivery  = 13,
     EventTypeSMS_t_NONE = 66
} EventTypeSMS_e;

typedef enum MonitorMode 
{
     MonitorMode_interrupted = 0,
     MonitorMode_notifyAndContinue = 1,
     MonitorMode_transparent = 2
} MonitorMode_e;

typedef enum MessageType 
{
	MessageType_request	= 0,
	MessageType_notification	= 1
} MessageType_e;

class InternalRequestReportSMSEventArg;
class RequestReportSMSEventArg: public Component
{
 public:

   struct SMSEvent 
   {
     EventTypeSMS_e event;
     MonitorMode_e monitorType;
   };

   typedef vector<SMSEvent> SMSEventVector;

    public:
      RequestReportSMSEventArg();
      ~RequestReportSMSEventArg();
      const SMSEventVector& getSMSEvents();

      void encode(vector<unsigned char>& buf);
      void decode(const vector<unsigned char>& buf);
  private:
    InternalRequestReportSMSEventArg* internal;
};

class EventReportSMSArg: public Component
{
public:

	EventTypeSMS_e 	 eventType;
	MessageType_e messageType;
public:
	EventReportSMSArg(EventTypeSMS_e eventType, MessageType_e messageType);
	~EventReportSMSArg();
    void encode(vector<unsigned char>& buf);
    void decode(const vector<unsigned char>& buf);
};

class ConnectSMSArg: public Component{};
class FurnishChargingInformationSMSArg: public Component{};
class ReleaseSMSArg: public Component{};
class ResetTimerSMSArg: public Component{};

}
}
}

#endif
