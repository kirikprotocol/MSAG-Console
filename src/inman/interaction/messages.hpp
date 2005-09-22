#ident "$Id$"

#ifndef __SMSC_INMAN_INTERACTION_MESSAGES__
#define __SMSC_INMAN_INTERACTION_MESSAGES__

#include "inman/interaction/serializer.hpp"

using std::runtime_error;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializableObject;

namespace smsc  {
namespace inman {
namespace interaction {

// 1. ChargeSms   	    ( SMSC --> INMAN )
// 2. ChargeSmsResult   ( SMSC <-- INMAN )
// 3. DeliverySmsResult ( SMSC --> INMAN )


// Use 'visitor' pattern
class InmanHandler;
class InmanCommand : public SerializableObject
{
	public:
		virtual void handle( InmanHandler* ) = 0;
};

class SmscHandler;
class SmscCommand : public SerializableObject
{
	public:
		virtual void handle( SmscHandler* ) = 0;
};

enum
{
	CHARGE_SMS_TAG 			= 1,
	CHARGE_SMS_RESULT_TAG  	= 2,
	DELIVERY_SMS_RESULT_TAG = 3
};

typedef enum
{
	CHARGING_POSSIBLE		= 0,
	CHARGING_NOT_POSSIBLE   = 1
} 
ChargeSmsResult_t;

typedef enum
{
	DELIVERY_SUCCESSED = 0,
	DELIVERY_FAILED    = 1
} 
DeliverySmsResult_t;


class ChargeSms : public InmanCommand
{
public:
	void setDestinationSubscriberNumber(const std::string& imsi);
	void setCallingPartyNumber(const std::string& imsi);
	void setIMSI(const std::string& imsi);
	void setSMSCAddress(const std::string& imsi);
	void setTimeAndTimezone(time_t tmVal);
	void setTPShortMessageSpecificInfo(unsigned char );
	void setTPProtocolIdentifier(unsigned char );
	void setTPDataCodingScheme(unsigned char );
	void setTPValidityPeriod(time_t vpVal);
	void setlocationInformationMSC(const std::string& imsi);

	ChargeSms();
	virtual ~ChargeSms();

	virtual void handle(InmanHandler*);

protected:
    virtual void load(ObjectBuffer& in);
    virtual void save(ObjectBuffer& out);
private:
	std::string   destinationSubscriberNumber;
	std::string   callingPartyNumber;
	std::string   imsi;
	std::string   smscAddress;
	time_t		  timeAndTimezine;
	unsigned char tpShortMessageSpecificInfo;
	unsigned char tpTPProtocolIdentifier;
	unsigned char tpTPDataCodingScheme;
	time_t 		  tpValidityPeriod;
	std::string   locationInformationMSC;
};

class ChargeSmsResult : public SmscCommand
{
public:
	ChargeSmsResult();
	ChargeSmsResult(ChargeSmsResult_t value);
	virtual ~ChargeSmsResult();
	ChargeSmsResult_t GetValue() const;
	virtual void handle(SmscHandler*);
protected:
    virtual void load(ObjectBuffer& in);
    virtual void save(ObjectBuffer& out);
		
	ChargeSmsResult_t value;
};

class DeliverySmsResult : public InmanCommand
{
public:
	
	DeliverySmsResult();
	DeliverySmsResult(DeliverySmsResult_t);
	virtual ~DeliverySmsResult();
	DeliverySmsResult_t GetValue() const;
	virtual void handle(InmanHandler*);
protected:
    virtual void load(ObjectBuffer& in);
    virtual void save(ObjectBuffer& out);
	DeliverySmsResult_t value;
};

class InmanHandler
{
	public:
		virtual void onChargeSms(ChargeSms* sms) = 0;
		virtual void onDeliverySmsResult(DeliverySmsResult* sms) = 0;
};

class SmscHandler
{
	public:
		virtual void onChargeSmsResult(ChargeSmsResult* sms) = 0;
};

}
}
}

#endif
