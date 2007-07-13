#ident "$Id$"
#ifndef __SMSC_INMAN_CAPSMS_COMPS_HPP__
#define __SMSC_INMAN_CAPSMS_COMPS_HPP__

#include <map>

#include "inman/comp/compdefs.hpp"
#include "inman/comp/CapOpErrors.hpp"
#include "logger/Logger.h"

using smsc::logger::Logger;

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
//this is the clone of MiscCallInfo.h::messageType_e
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
namespace comp {


extern const char * _nmMonitorMode(enum MonitorMode m_mode);
extern const char * _nmEventTypeSMS(enum EventTypeSMS event);
extern const char * _nmMessageType(enum messageType msg_type);

struct CapSMSOp {
    enum Code {
	InitialDPSMS 			= 60,
	FurnishChargingInformationSMS	= 61,
	ConnectSMS			= 62,
	RequestReportSMSEvent		= 63,
	EventReportSMS			= 64,
	ContinueSMS			= 65,
	ReleaseSMS			= 66,
	ResetTimerSMS			= 67
    };
    static const char * code2Name(unsigned char op_code)
    {
        switch (op_code) {
        case InitialDPSMS: return "InitialDPSMS"; break;
        case FurnishChargingInformationSMS: return "FurnishChargingInformationSMS"; break;
        case ConnectSMS: return "ConnectSMS"; break;
        case RequestReportSMSEvent: return "RequestReportSMSEvent"; break;
        case EventReportSMS: return "EventReportSMS"; break;
        case ContinueSMS: return "ContinueSMS"; break;
        case ReleaseSMS: return "ReleaseSMS"; break;
        case ResetTimerSMS: return "ResetTimerSMS"; break;
        default:;
        }
        return "IllegalOp";
    }
};

struct CAP3SMSerrCode {
    enum {
        missingParameter = CAPOpErrorId::missingParameter,
        parameterOutOfRange = CAPOpErrorId::parameterOutOfRange,
        systemFailure = CAPOpErrorId::systemFailure,
        taskRefused = CAPOpErrorId::taskRefused,
        unexpectedComponentSequence = CAPOpErrorId::unexpectedComponentSequence, 
        unexpectedDataValue = CAPOpErrorId::unexpectedDataValue,
        unexpectedParameter = CAPOpErrorId::unexpectedParameter
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


class SMSEventDPs : public std::map<EventTypeSMS_e, MonitorMode_e> {
public:
    bool hasMMode(MonitorMode_e ref_mode)
    {
        for (SMSEventDPs::const_iterator cit = begin(); cit != end(); ++cit) {
            if (cit->second == ref_mode)
                return true;
        }
        return false;
    }
};

// Direction: gsmSSF or gprsSSF -> gsmSCF, Timer: Tidpsms
// This operation is used after a TDP to indicate request for service.
class PrivateInitialDPSMSArg;
//NOTE: requires the preceeding call of tzset()
class InitialDPSMSArg: public Component { //SSF -> SCF
public:
    InitialDPSMSArg(DeliveryMode_e idpMode, unsigned int serviceKey);
    ~InitialDPSMSArg();

    enum { //errors in addition to CAP3SMSerrCode
        missingCustomerRecord = 6
    } IDPErrCodes;

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

    void setLocationInformationMSC(const TonNpiAddress& addr) throw(CustomException);
    void setLocationInformationMSC(const char* text) throw(CustomException);

    void encode(std::vector<unsigned char>& buf) throw(CustomException);

private:
    Logger*  compLogger;
    PrivateInitialDPSMSArg* comp;
};
      
//  Direction: gsmSSF or gprsSSF -> gsmSCF, Timer: Terbsms
//  This operation is used to notify the gsmSCF of a Short Message related event
//  (FSM events such as submission, delivery or failure) previously requested by
//  the gsmSCF in a RequestReportSMSEvent operation.
class EventReportSMSArg: public Component { //SSF -> SCF
public:
    EventReportSMSArg(EventTypeSMS_e et, messageType_e mt);
    ~EventReportSMSArg() {}

    void encode(std::vector<unsigned char>& buf) throw(CustomException);
    const std::string & print(std::string & dump);

private:
    EventTypeSMS_e	eventType;
    messageType_e	messageType;
    Logger*  compLogger;
};

//  Direction: gsmSCF -> gsmSSF or gprsSSF, Timer: Trrbsms
//  This operation is used to request the gsmSSF or gprsSSF to monitor for a
//  Short Message related event (FSM events such as submission, delivery or failure)
//  and to send a notification to the gsmSCF when the event is detected.
//  NOTE: Inman uses only SCF -> SSF
class RequestReportSMSEventArg: public Component { //SSF -> SCF, SCF -> SSF
public:
    RequestReportSMSEventArg();
    ~RequestReportSMSEventArg() { }

    const SMSEventDPs& SMSEvents(void) const { return events; }
    const std::string & printEvents(std::string & dump);

    void  decode(const std::vector<unsigned char>& buf) throw(CustomException);

private:
    SMSEventDPs events;
    Logger*     compLogger;
};

//  Direction: gsmSCF -> gsmSSF or gprsSSF, Timer: Tconsms
//  This operation is used to request the smsSSF to perform the SMS processing
//  actions to route or forward a short message to a specified destination.
class ConnectSMSArg: public Component { //SCF -> SSF
public:
    enum Params { connNone = 0, connDSN = 0x01, connCPN = 0x02, connSMSC = 0x04 };

    ConnectSMSArg();
    ~ConnectSMSArg();

    inline unsigned char paramsMask(void) const { return mask; }
    inline const TonNpiAddress&	destinationSubscriberNumber(void) const { return dstSN; }
    inline const TonNpiAddress&	callingPartyNumber(void) const { return clngPN; }
    inline const TonNpiAddress&	SMSCAddress(void) const { return sMSCAdr; }

    void decode(const std::vector<unsigned char>& buf) throw(CustomException);

protected:
    TonNpiAddress	dstSN, clngPN, sMSCAdr;
    unsigned char       mask;

private:
    Logger* compLogger;
};

//  Direction: gsmSCF -> gsmSSF or gprsSSF, Timer: Tfcisms 
//  This operation is used to request the smsSSF to generate, register a
//  charging record or to include some information in the default SM record.
//  The registered charging record is intended for off line charging of the
//  Short Message.
class FurnishChargingInformationSMSArg: public Component { //SCF -> SSF 
public:
    FurnishChargingInformationSMSArg();
    ~FurnishChargingInformationSMSArg();

    void decode(const std::vector<unsigned char>& buf) throw(CustomException);

private:
    Logger* compLogger;
};


//  Direction: gsmSCF -> gsmSSF or gprsSSF, Timer: Trelsms
//  This operation is used to prevent an attempt to submit or deliver a short message. 
class ReleaseSMSArg: public Component { //SCF -> SSF 
public:
    unsigned char rPCause;

    ReleaseSMSArg();
    ~ReleaseSMSArg();

    void decode(const std::vector<unsigned char>& buf) throw(CustomException);

private:
    Logger* compLogger;
};

// Direction: gsmSCF -> smsSSF, Timer: Trtsms 
// This operation is used to request the smsSSF to refresh an application
// timer in the smsSSF.
class ResetTimerSMSArg: public Component { //SCF -> SSF 
public:
    time_t	timerValue;

    ResetTimerSMSArg();
    ~ResetTimerSMSArg();

    void decode(const std::vector<unsigned char>& buf) throw(CustomException);

private:
    Logger* compLogger;
};

}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_CAPSMS_COMPS_HPP__ */

