static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>

#include "inman/common/errors.hpp"
#include "inman/common/util.hpp"
#include "core/buffers/TmpBuf.hpp"

#include "inman/interaction/messages.hpp"

namespace smsc  {
namespace inman {
namespace interaction {


ChargeSms::ChargeSms()
{
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
    out << (USHORT_T) CHARGE_SMS_TAG;
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
}

void ChargeSmsResult::save(ObjectBuffer& out)
{
   	out << (USHORT_T) CHARGE_SMS_RESULT_TAG;
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
}

void DeliverySmsResult::save(ObjectBuffer& out)
{
    out << (USHORT_T) DELIVERY_SMS_RESULT_TAG;
}

void DeliverySmsResult::handle(InmanHandler* handler)
{
	assert( handler );
	handler->onDeliverySmsResult( this );
}

}
}
}

