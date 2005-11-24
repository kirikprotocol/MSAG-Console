static char const ident[] = "$Id$";

#include <assert.h>
#include <string>
#include <stdexcept>

#include "inman/interaction/messages.hpp"
#include "inman/common/util.hpp"
#include "inman/common/cvtutil.hpp"

using std::runtime_error;
using smsc::inman::common::format;

using smsc::cbs::parseCBS_DCS;
using smsc::cbs::CBS_DCS;

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
 * Inman messages are transferred as length prefixed packet and have
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
        throw runtime_error( format("SrlzrInap: Invalid fromat version: 0x%X", version) );

    in >> dialogId;
    in >> objectId;

    SerializableObject* obj = create( objectId );
    if( !obj ) 
        throw runtime_error( format("SrlzrInap: Invalid object ID: 0x%X", objectId) );

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
    : partsNum(1)
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

void ChargeSms::setCallingIMSI(const std::string& value)
{
    callingImsi = value;
}

void ChargeSms::setSMSCAddress(const std::string& value)
{
    smscAddress = value;
}

void ChargeSms::setSubmitTimeTZ(time_t value)
{
    submitTimeTZ = value;
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

//data for CDR generation

void ChargeSms::setCallingSMEid(const std::string & sme_id)
{
    callingSMEid = sme_id;
}

void ChargeSms::setRouteId(const std::string & route_id)
{
    routeId = route_id;
}

void ChargeSms::setServiceId(int32_t service_id)
{
    serviceId = service_id;
}

void ChargeSms::setUserMsgRef(uint32_t msg_ref)
{
    userMsgRef = msg_ref;
}

void ChargeSms::setMsgId(uint64_t msg_id)
{
    msgId = msg_id;
}

void ChargeSms::setServiceOp(int32_t service_op)
{
    ussdServiceOp = service_op;
}

void ChargeSms::setPartsNum(uint8_t parts_num)
{
    partsNum = parts_num;
}


const std::string& ChargeSms::getDestinationSubscriberNumber(void) const
{
    return destinationSubscriberNumber;
}

const std::string& ChargeSms::getCallingPartyNumber(void) const
{
    return callingPartyNumber;
}

const std::string& ChargeSms::getCallingIMSI(void) const
{
    return callingImsi;
}

const std::string& ChargeSms::getSMSCAddress(void) const
{
    return smscAddress;
}

time_t ChargeSms::getSubmitTimeTZ(void) const
{
    return submitTimeTZ;
}

unsigned char ChargeSms::getTPShortMessageSpecificInfo(void) const
{
    return tpShortMessageSpecificInfo;
}

unsigned char ChargeSms::getTPProtocolIdentifier(void) const
{
    return tpProtocolIdentifier;
}

unsigned char ChargeSms::getTPDataCodingScheme(void) const
{
    return tpDataCodingScheme;
}
time_t ChargeSms::getTPValidityPeriod(void) const
{
    return tpValidityPeriod;
}

const std::string&  ChargeSms::getLocationInformationMSC(void) const
{
    return locationInformationMSC;
}

void ChargeSms::load(ObjectBuffer& in)
{
    in >> destinationSubscriberNumber;
    in >> callingPartyNumber;
    in >> callingImsi;
    in >> smscAddress;
    in >> submitTimeTZ;
    in >> tpShortMessageSpecificInfo;
    in >> tpProtocolIdentifier;
    in >> tpDataCodingScheme;
    in >> tpValidityPeriod;
    in >> locationInformationMSC;
    //data for CDR generation
    in >> callingSMEid;
    in >> routeId;
    in >> serviceId;
    in >> userMsgRef;
    in >> msgId;
    in >> ussdServiceOp;
    in >> partsNum;
}

void ChargeSms::save(ObjectBuffer& out)
{
    out << destinationSubscriberNumber;
    out << callingPartyNumber;
    out << callingImsi;
    out << smscAddress;
    out << submitTimeTZ;
    out << tpShortMessageSpecificInfo;
    out << tpProtocolIdentifier;
    out << tpDataCodingScheme;
    out << tpValidityPeriod;
    out << locationInformationMSC;
    //data for CDR generation
    out << callingSMEid;
    out << routeId;
    out << serviceId;
    out << userMsgRef;
    out << msgId;
    out << ussdServiceOp;
    out << partsNum;
}


void ChargeSms::export2CDR(CDRRecord & cdr) const
{
    cdr._msgId = msgId;
    cdr._cdrType = CDRRecord::dpOrdinary;
    cdr._mediaType = (parseCBS_DCS(tpDataCodingScheme) == CBS_DCS::dcBINARY8) ?
                        CDRRecord::dpBinary : CDRRecord::dpText ;
    cdr._bearer = CDRRecord::dpUSSD;
    cdr._submitTime = submitTimeTZ;
    cdr._partsNum = partsNum;

    cdr._srcAdr = callingPartyNumber;
    cdr._srcIMSI = callingImsi;
    cdr._srcMSC = locationInformationMSC;
    cdr._srcSMEid = callingSMEid;

    cdr._routeId = routeId;
    cdr._serviceId = serviceId;
    cdr._userMsgRef = userMsgRef;

    cdr._dstAdr = destinationSubscriberNumber;
}


void ChargeSms::handle(InmanHandler* handler)
{
    assert( handler );
    handler->onChargeSms( this );
}

//-----------------------------------------------
// ChargeSmsResult impl
//-----------------------------------------------


ChargeSmsResult::ChargeSmsResult()
    : value(CHARGING_POSSIBLE)
    , rPC(0)
{
    setObjectId((unsigned short)CHARGE_SMS_RESULT_TAG);
}

ChargeSmsResult::ChargeSmsResult(uint32_t rPCcode)
    : value(CHARGING_NOT_POSSIBLE)
    , rPC(rPCcode)
{
    setObjectId((unsigned short)CHARGE_SMS_RESULT_TAG);
}

ChargeSmsResult::~ChargeSmsResult()
{
}

ChargeSmsResult_t ChargeSmsResult::GetValue() const
{
    return value;
}

ChargeSmsResult::ChargeErrorClass  ChargeSmsResult::GetErrorClass(void) const
{
    if (!rPC)
        return ChargeSmsResult::chgOk;
    if (rPC <= INMAN_RPCAUSE_LIMIT)
        return  ChargeSmsResult::chgRPCause;
    if (rPC <= INMAN_PROTOCOL_ERROR_LIMIT)
        return ChargeSmsResult::chgTCPerror;
    return ChargeSmsResult::chgCAP3error;
}
//return combined nonzero code holding RP cause or CAP3 error, or protocol error
uint32_t ChargeSmsResult::GetErrorCode(void) const
{
    return rPC;
}

uint8_t  ChargeSmsResult::GetRPCause(void) const
{
    return (rPC <= INMAN_RPCAUSE_LIMIT) ? (uint8_t)rPC : 0;
}

//return nonzero CAP3 error code
uint32_t  ChargeSmsResult::GetCAP3Error(void) const
{
    return (rPC >= INMAN_SCF_ERROR_BASE) ? (rPC - INMAN_SCF_ERROR_BASE) : 0;
}


void ChargeSmsResult::load(ObjectBuffer& in)
{
    unsigned short v;
    in >> v;
    value = static_cast<ChargeSmsResult_t>(v);
    in >> rPC;
}

void ChargeSmsResult::save(ObjectBuffer& out)
{
    out << (unsigned short)value;
    out << rPC;
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
    setObjectId((unsigned short)DELIVERY_SMS_RESULT_TAG);
}

DeliverySmsResult::~DeliverySmsResult()
{
}

void DeliverySmsResult::setResultValue(DeliverySmsResult_t res)
{
    value = res;
}
void DeliverySmsResult::setDestIMSI(const std::string& imsi)
{
    destImsi = imsi;
}
void DeliverySmsResult::setDestMSC(const std::string& msc)
{
    destMSC = msc;
}
void DeliverySmsResult::setDestSMEid(const std::string& sme_id)
{
    destSMEid = sme_id;
}
void DeliverySmsResult::setDeliveryTime(time_t final_tm)
{
    finalTimeTZ = final_tm;
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
    //optional data for CDR generation (on successfull delivery)
    if (!value) {
        in >> destImsi;
        in >> destMSC;
        in >> destSMEid;
        in >> finalTimeTZ;
    }
}

void DeliverySmsResult::save(ObjectBuffer& out)
{
    out << (unsigned short)value;
    //optional data for CDR generation (on successfull delivery)
    if (!value) {
        out << destImsi;
        out << destMSC;
        out << destSMEid;
        out << finalTimeTZ;
    }
}

void DeliverySmsResult::export2CDR(CDRRecord & cdr) const
{
    if (!(cdr._dlvrRes = (CDRRecord::CDRDeliveryStatus)value)) {
        cdr._dstIMSI = destImsi;
        cdr._dstMSC = destMSC;
        cdr._dstSMEid = destSMEid;
        cdr._finalTime = finalTimeTZ;
    }
}

void DeliverySmsResult::handle(InmanHandler* handler)
{
    assert( handler );
    handler->onDeliverySmsResult( this );
}

} //interaction
} //inman
} //smsc

