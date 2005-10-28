static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>

#include "inman/interaction/messages.hpp"
#include "inman/common/util.hpp"

using std::runtime_error;
using smsc::inman::common::format;


namespace smsc  {
namespace inman {                                 
namespace interaction {

/* ************************************************************************** *
 * class SerializerInap implementation:
 * ************************************************************************** */

SerializerInap::SerializerInap()
{
    registerProduct( CHARGE_SMS_TAG, new ProducerT< ChargeSms >() );
    registerProduct( CHARGE_SMS_RESULT_TAG, new ProducerT< ChargeSmsResult>() );
    registerProduct( DELIVERY_SMS_RESULT_TAG, new ProducerT< DeliverySmsResult>() );
}

SerializerInap::~SerializerInap()
{
}

SerializerInap* SerializerInap::getInstance()
{
    static SerializerInap instance;
    return &instance;
}

/*
 * Inman messages are transferred as raw data of arbitrary length and have
 * the following serialization format:

  2b        4b         2b       up to ...b
-------   ---------   -----   -------------------------------------
 format : dialogId  : msgId :  message data                        |
                            |                                      |                                    |
                            --- processed by load()/save() method --
*/

SerializableObject* SerializerInap::deserialize(ObjectBuffer& in)
{
    unsigned short objectId, version;
    unsigned int   dialogId;

    in >> version;

    if( version != FORMAT_VERSION ) 
        throw runtime_error( format("Invalid version: 0x%X", version) );

    in >> dialogId;
    in >> objectId;

    SerializableObject* obj = create( objectId );
    if( !obj ) 
        throw runtime_error( format("Invalid object ID: 0x%X", objectId) );

    obj->setDialogId(dialogId);
    obj->setObjectId(objectId);
    obj->load( in );

    return obj;
}


void SerializerInap::serialize(SerializableObject* obj, ObjectBuffer& out)
{
    assert( obj );
    out << (unsigned short) FORMAT_VERSION;
    out << (unsigned int) obj->getDialogId();
    out << (unsigned short) obj->getObjectId();
    obj->save( out );
}

/* ************************************************************************** *
 * class ChargeSms implementation:
 * ************************************************************************** */

ChargeSms::ChargeSms()
{
    setObjectId((unsigned short)CHARGE_SMS_TAG);
}

ChargeSms::~ChargeSms()
{
}

void ChargeSms::setDestinationSubscriberNumber(const std::string& value)
{
	destinationSubscriberNumber = value;
}

void ChargeSms::ChargeSms::setCallingPartyNumber(const std::string& value)
{
	callingPartyNumber = value;
}

void ChargeSms::setIMSI(const std::string& value)
{
	imsi = value;
}

void ChargeSms::setSMSCAddress(const std::string& value)
{
	smscAddress = value;
}

void ChargeSms::setTimeAndTimezone(time_t value)
{
	timeAndTimezine = value;
}

void ChargeSms::setTPShortMessageSpecificInfo(unsigned char value)
{
	tpShortMessageSpecificInfo = value;
}

void ChargeSms::setTPProtocolIdentifier(unsigned char value)
{
	tpProtocolIdentifier = value;
}

void ChargeSms::setTPDataCodingScheme(unsigned char value)
{
	tpDataCodingScheme = value;
}

void ChargeSms::setTPValidityPeriod(time_t value)
{
	tpValidityPeriod = value;
}

void ChargeSms::setLocationInformationMSC(const std::string& value)
{
	locationInformationMSC = value;
}

std::string   ChargeSms::getDestinationSubscriberNumber() const
{
	return destinationSubscriberNumber;
}

std::string   ChargeSms::getCallingPartyNumber() const
{
	return callingPartyNumber;
}

std::string   ChargeSms::getIMSI() const
{
	return imsi;
}

std::string   ChargeSms::getSMSCAddress() const
{
	return smscAddress;
}

time_t		  ChargeSms::getTimeAndTimezone() const
{
	return timeAndTimezine;
}

unsigned char ChargeSms::getTPShortMessageSpecificInfo() const
{
	return tpShortMessageSpecificInfo;	
}

unsigned char ChargeSms::getTPProtocolIdentifier() const
{
	return tpProtocolIdentifier;
}

unsigned char ChargeSms::getTPDataCodingScheme() const
{
	return tpDataCodingScheme;
}
time_t 		  ChargeSms::getTPValidityPeriod() const
{
	return tpValidityPeriod;
}

std::string   ChargeSms::getLocationInformationMSC() const
{
	return locationInformationMSC;	
}

void ChargeSms::load(ObjectBuffer& in)
{
	in >> destinationSubscriberNumber;
	in >> callingPartyNumber;
	in >> imsi;
	in >> smscAddress;
	in >> timeAndTimezine;
	in >> tpShortMessageSpecificInfo;
	in >> tpProtocolIdentifier;
	in >> tpDataCodingScheme;
	in >> tpValidityPeriod;
	in >> locationInformationMSC;
}

void ChargeSms::save(ObjectBuffer& out)
{
    out << destinationSubscriberNumber;
    out << callingPartyNumber;
    out << imsi;
    out << smscAddress;
    out << timeAndTimezine;
    out << tpShortMessageSpecificInfo;
    out << tpProtocolIdentifier;
    out << tpDataCodingScheme;
    out << tpValidityPeriod;
    out << locationInformationMSC;
}

void ChargeSms::handle(InmanHandler* handler)
{
	assert( handler );
	handler->onChargeSms( this );
}

//-----------------------------------------------
// ChargeSmsResult impl
//-----------------------------------------------


ChargeSmsResult::ChargeSmsResult() : value( CHARGING_POSSIBLE )
{
    setObjectId((unsigned short)CHARGE_SMS_RESULT_TAG);
}

ChargeSmsResult::ChargeSmsResult(ChargeSmsResult_t val) : value( val )
{
}

ChargeSmsResult::~ChargeSmsResult()
{
}

ChargeSmsResult_t ChargeSmsResult::GetValue() const
{
	return value;
}

void ChargeSmsResult::load(ObjectBuffer& in)
{
	unsigned short v;
	in >> v;
	value = static_cast<ChargeSmsResult_t>(v);
}

void ChargeSmsResult::save(ObjectBuffer& out)
{
    out << (unsigned short)value;
}

void ChargeSmsResult::handle(SmscHandler* handler)
{
	assert( handler );
	handler->onChargeSmsResult( this );
}

//-----------------------------------------------
// ChargeSmsResult impl
//-----------------------------------------------


DeliverySmsResult::DeliverySmsResult() : value( DELIVERY_SUCCESSED )
{
    setObjectId((unsigned short)DELIVERY_SMS_RESULT_TAG);
}

DeliverySmsResult::DeliverySmsResult(DeliverySmsResult_t val) : value( val )
{
}

DeliverySmsResult::~DeliverySmsResult()
{
}

DeliverySmsResult_t DeliverySmsResult::GetValue() const
{
	return value;
}

void DeliverySmsResult::load(ObjectBuffer& in)
{
	unsigned short v;
	in >> v;
	value = static_cast<DeliverySmsResult_t>(v);
}

void DeliverySmsResult::save(ObjectBuffer& out)
{
    out << (unsigned short)value;
}

void DeliverySmsResult::handle(InmanHandler* handler)
{
	assert( handler );
	handler->onDeliverySmsResult( this );
}

} //interaction
} //inman
} //smsc

