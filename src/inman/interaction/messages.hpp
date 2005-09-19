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
namespace cmd		  {

// 1. ChargeSms   	  	( SMSC --> INMAN )
// 2. ChargeSmsResult  ( SMSC <-- INMAN )
// 3. CommitResult  ( SMSC --> INMAN )

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


class CommandHandler;

class ChargeSms : public SerializableObject
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

protected:
    virtual void load(ObjectBuffer& in);
    virtual void save(ObjectBuffer& out);
};

class ChargeSmsResult : public SerializableObject
{
public:


	ChargeSmsResult();

	ChargeSmsResult(ChargeSmsResult_t value);

	virtual ~ChargeSmsResult();

	ChargeSmsResult_t GetValue() const;

protected:
    virtual void load(ObjectBuffer& in);
    virtual void save(ObjectBuffer& out);
		
	ChargeSmsResult_t value;
};

class DeliverySmsResult : public SerializableObject
{
public:
	
	DeliverySmsResult();
	DeliverySmsResult(DeliverySmsResult_t);
	virtual ~DeliverySmsResult();

	DeliverySmsResult_t GetValue() const;

protected:
    virtual void load(ObjectBuffer& in);
    virtual void save(ObjectBuffer& out);

	DeliverySmsResult_t value;
};

class ClientHandler
{
	public:
		virtual void handle(ChargeSmsResult* sms) = 0;
};

class ServerHandler
{
	public:
		virtual void handle(ChargeSms* sms) = 0;
		virtual void handle(DeliverySmsResult* sms) = 0;
};

}
}
}
}

#endif
