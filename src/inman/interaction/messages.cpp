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

void ChargeSms::setDestinationSubscriberNumber(const std::string& imsi)
{
}

void ChargeSms::ChargeSms::setCallingPartyNumber(const std::string& imsi)
{
}

void ChargeSms::setIMSI(const std::string& imsi)
{
}

void ChargeSms::setSMSCAddress(const std::string& imsi)
{
}

void ChargeSms::setTimeAndTimezone(time_t tmVal)
{
}

void ChargeSms::setTPShortMessageSpecificInfo(unsigned char )
{
}

void ChargeSms::setTPProtocolIdentifier(unsigned char )
{
}

void ChargeSms::setTPDataCodingScheme(unsigned char )
{
}

void ChargeSms::setTPValidityPeriod(time_t vpVal)
{
}

void ChargeSms::setlocationInformationMSC(const std::string& imsi)
{
}

void ChargeSms::load(ObjectBuffer& in)
{
}

void ChargeSms::save(ObjectBuffer& out)
{
    out << (USHORT_T) CHARGE_SMS_TAG;
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

