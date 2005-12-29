#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_COMPS_HPP__
#define __SMSC_INMAN_INAP_COMPS_HPP__

#include <map>

#include "inman/common/adrutil.hpp"
#include "inman/comp/compdefs.hpp"
#include "logger/Logger.h"

using smsc::logger::Logger;

using std::map;

/* GVR NOTE: while linking the below enums are taken from generated
 * asn1/c codec, so they should not have namespace prefix.
 */

#ifndef	_MonitorMode_H_
//this is the clone of MonitorMode.h::MonitorMode_e
typedef enum MonitorMode {
    MonitorMode_interrupted =		0,
    MonitorMode_notifyAndContinue =	1,
    MonitorMode_transparent =		2
} MonitorMode_e;
#endif /* _MonitorMode_H_ */

#ifndef	_MiscCallInfo_H_
//this is the clone of MiscCallInfo.h::MessageType_e
typedef enum messageType {
    MessageType_request		= 0,
    MessageType_notification	= 1
} messageType_e;
#endif /* _MiscCallInfo_H_ */

#ifndef	_EventTypeSMS_H_
//this is the clone of EventTypeSMS.h::EventTypeSMS_e
typedef enum EventTypeSMS {
    EventTypeSMS_sms_CollectedInfo =	1,
    EventTypeSMS_o_smsFailure =		2,
    EventTypeSMS_o_smsSubmission =	3,
    EventTypeSMS_sms_DeliveryRequested = 11,
    EventTypeSMS_t_smsFailure =		12,
    EventTypeSMS_t_smsDelivery =	13,
    EventTypeSMS_t_NONE =		66
} EventTypeSMS_e;
#endif /* _EventTypeSMS_H_ */


namespace smsc {
namespace inman {
namespace comp{

struct InapOpCode {
    enum {
	InitialDPSMS 			= 60,
	FurnishChargingInformationSMS	= 61,
	ConnectSMS			= 62,
	RequestReportSMSEvent		= 63,
	EventReportSMS			= 64,
	ContinueSMS			= 65,
	ReleaseSMS			= 66,
	ResetTimerSMS			= 67
    };
};

enum TP_VP_format {
    tp_vp_none = 0,
    tp_vp_enhanced, // '01'
    tp_vp_relative, // '10' 
    tp_vp_absolute  // '11'
};

typedef enum DeliveryMode {
    DeliveryMode_Originating = 1, //EventTypeSMS_sms_CollectedInfo
    DeliveryMode_Terminating = 11 //EventTypeSMS_sms_DeliveryRequested
} DeliveryMode_e;

// Direction: gsmSSF or gprsSSF -> gsmSCF, Timer: Tidpsms
// This operation is used after a TDP to indicate request for service.
class PrivateInitialDPSMSArg;
//NOTE: requires the preceeding call of tzset()
class InitialDPSMSArg: public Component //SSF -> SCF
{
public:
    InitialDPSMSArg(DeliveryMode_e idpMode, unsigned int serviceKey);
    ~InitialDPSMSArg();

    void setDestinationSubscriberNumber(const TonNpiAddress& addr); // missing for MT
    void setDestinationSubscriberNumber(const char * text);

    void setCalledPartyNumber(const TonNpiAddress& addr);           // missing for MO
    void setCalledPartyNumber(const char * text);

    void setCallingPartyNumber(const TonNpiAddress& addr);
    void setCallingPartyNumber(const char * text);

    //imsi contains sequence of ASCII digits
    void setIMSI(const std::string& imsi) throw(CustomException);

    void setSMSCAddress(const TonNpiAddress& addr);
    void setSMSCAddress(const char * text);

    //NOTE: requires the preceeding call of tzset()
    void setTimeAndTimezone(time_t tmVal) throw(CustomException);
    void setTPValidityPeriod(time_t vpVal, enum TP_VP_format fmt) throw(CustomException);

    void setTPShortMessageSpecificInfo(unsigned char );
    void setTPProtocolIdentifier(unsigned char );
    void setTPDataCodingScheme(unsigned char );

    void setLocationInformationMSC(const TonNpiAddress& addr);
    void setLocationInformationMSC(const char* text);

    void encode(vector<unsigned char>& buf) throw(CustomException);

private:
    Logger*  compLogger;
    PrivateInitialDPSMSArg* comp;
};
      
//  Direction: gsmSSF or gprsSSF -> gsmSCF, Timer: Terbsms
//  This operation is used to notify the gsmSCF of a Short Message related event
//  (FSM events such as submission, delivery or failure) previously requested by
//  the gsmSCF in a RequestReportSMSEvent operation.
class EventReportSMSArg: public Component //SSF -> SCF
{
    public:
	EventTypeSMS_e	eventType;
	messageType_e	messageType;

	EventReportSMSArg(EventTypeSMS_e et, messageType_e mt);
	~EventReportSMSArg() {}

	void encode(vector<unsigned char>& buf) throw(CustomException);

    private:
	Logger*  compLogger;
};

//  Direction: gsmSCF -> gsmSSF or gprsSSF, Timer: Trrbsms
//  This operation is used to request the gsmSSF or gprsSSF to monitor for a
//  Short Message related event (FSM events such as submission, delivery or failure)
//  and to send a notification to the gsmSCF when the event is detected.
class InternalRequestReportSMSEventArg;
// NOTE: Inman uses only SCF -> SSF
class RequestReportSMSEventArg: public Component //SSF -> SCF, SCF -> SSF
{
    public:
	struct SMSEvent 
	{
	  EventTypeSMS_e event;
	  MonitorMode_e monitorType;
	};

	typedef vector<SMSEvent> SMSEventVector;

	RequestReportSMSEventArg();
	~RequestReportSMSEventArg();

	const SMSEventVector& getSMSEvents();

	void  decode(const vector<unsigned char>& buf) throw(CustomException);

    private:
	InternalRequestReportSMSEventArg* comp;
	Logger*  compLogger;
};

//  Direction: gsmSCF -> gsmSSF or gprsSSF, Timer: Tconsms
//  This operation is used to request the smsSSF to perform the SMS processing
//  actions to route or forward a short message to a specified destination.
class ConnectSMSArg: public Component //SCF -> SSF
{
    public:
	ConnectSMSArg();
	~ConnectSMSArg();

	const TonNpiAddress&	destinationSubscriberNumber() { return dstSN; }
	const TonNpiAddress&	callingPartyNumber() { return clngPN; }
	const TonNpiAddress&	SMSCAddress() { return sMSCAdr; }

	void decode(const vector<unsigned char>& buf) throw(CustomException);

    protected:
	TonNpiAddress	dstSN, clngPN, sMSCAdr;
    private:
	Logger* compLogger;
};

//  Direction: gsmSCF -> gsmSSF or gprsSSF, Timer: Tfcisms 
//  This operation is used to request the smsSSF to generate, register a
//  charging record or to include some information in the default SM record.
//  The registered charging record is intended for off line charging of the
//  Short Message.
class PrivateFurnishChargingInformationSMSArg;
class FurnishChargingInformationSMSArg: public Component //SCF -> SSF 
{
    public:
	FurnishChargingInformationSMSArg();
	~FurnishChargingInformationSMSArg();

	void decode(const vector<unsigned char>& buf) throw(CustomException);

    private:
	PrivateFurnishChargingInformationSMSArg* comp;
	Logger* compLogger;
};

//  Direction: gsmSCF -> gsmSSF or gprsSSF, Timer: Trelsms
//  This operation is used to prevent an attempt to submit or deliver a short message. 
class ReleaseSMSArg: public Component //SCF -> SSF 
{
    public:
	unsigned char rPCause;

	ReleaseSMSArg();
	~ReleaseSMSArg();

	void decode(const vector<unsigned char>& buf) throw(CustomException);

    private:
	Logger* compLogger;
};

// Direction: gsmSCF -> smsSSF, Timer: Trtsms 
// This operation is used to request the smsSSF to refresh an application
// timer in the smsSSF.
class ResetTimerSMSArg: public Component //SCF -> SSF 
{
    public:
	time_t	timerValue;

	ResetTimerSMSArg();
	~ResetTimerSMSArg();

	void decode(const vector<unsigned char>& buf) throw(CustomException);

    private:
	Logger* compLogger;
};

}//namespace comp
}//namespace inman
}//namespace smsc

#endif
