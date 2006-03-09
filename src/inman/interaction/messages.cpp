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

//SerializerInap implements two deserialization modes:
// - complete: both the packet and contained object are deserialized
// - partial:  only packet prefix is parsed(required to identify object), remaining data
//             is set as object data buffer.
SerializerInap::SerializerInap()
    : _mode(SerializerInap::dsrlPartial)
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

SerializableObject* SerializerInap::deserializePrefix(ObjectBuffer& in) throw(CustomException)
{
    unsigned short objectId = 0, version;
    unsigned int   dialogId;

    try {
        in >> version;
        in >> dialogId;
        in >> objectId;
    } catch (SerializerException & exc) {
        throw SerializerException("SrlzrInap: invalid packet structure",
                                  SerializerException::invPacket, exc.what());
    }
    if (version != FORMAT_VERSION)
        throw SerializerException(format("SrlzrInap: Invalid format version: 0x%X", version).c_str(),
                                  SerializerException::invPacket, NULL);

    SerializableObject* obj = create(objectId);
    if( !obj )
        throw SerializerException(format("SrlzrInap: Invalid object ID: 0x%X", objectId).c_str(),
                                  SerializerException::invObject, NULL);

    obj->setDialogId(dialogId);
    obj->setObjectId(objectId);
    return obj;
}


SerializableObject* SerializerInap::deserializeAndOwn(ObjectBuffer * in, bool ownBuf/* = true*/) throw(CustomException)
{
    SerializableObject* obj = deserializePrefix(*in);

    if (_mode == SerializerInap::dsrlComplete) {
        try { obj->load(*in); }
        catch (SerializerException & exc) {
            throw SerializerException(
                format("SrlzrInap: deserialization failure obj 0x%X (dlgId: %u)",
                        obj->getObjectId(), obj->getDialogId()).c_str(),
                        SerializerException::invObjData, exc.what());
        }
        if (ownBuf)
            delete in;
    } else
        obj->setDataBuf(in, ownBuf);
    return obj;
}

SerializableObject* SerializerInap::deserialize(ObjectBuffer& in) throw(CustomException)
{
    SerializableObject* obj = deserializePrefix(in);

    if (_mode == SerializerInap::dsrlComplete) {
        try { obj->load(in); }
        catch (SerializerException & exc) {
            throw SerializerException(
                format("SrlzrInap: deserialization failure obj 0x%X (dlgId: %u)",
                        obj->getObjectId(), obj->getDialogId()).c_str(),
                        SerializerException::invObjData, exc.what());
        }
    } else
        obj->setDataBuf(&in, false);

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
    : partsNum(1), forwarded(false)
{
    setObjectId((unsigned short)CHARGE_SMS_TAG);
}

ChargeSms::~ChargeSms()
{
}

//data for CAP3 InitialDP OPERATION
void ChargeSms::setSMSCAddress(const std::string& value)
{
    csInfo.smscAddress = value;
}

void ChargeSms::setTPShortMessageSpecificInfo(unsigned char value)
{
    csInfo.tpShortMessageSpecificInfo = value;
}

void ChargeSms::setTPProtocolIdentifier(unsigned char value)
{
    csInfo.tpProtocolIdentifier = value;
}

void ChargeSms::setTPDataCodingScheme(unsigned char value)
{
    csInfo.tpDataCodingScheme = value;
}

void ChargeSms::setTPValidityPeriod(time_t value)
{
    csInfo.tpValidityPeriod = value;
}


void ChargeSms::load(ObjectBuffer& in) throw(CustomException)
{
    in >> dstSubscriberNumber;
    in >> callingPartyNumber;
    in >> callingImsi;
    in >> csInfo.smscAddress;
    in >> submitTimeTZ;
    in >> csInfo.tpShortMessageSpecificInfo;
    in >> csInfo.tpProtocolIdentifier;
    in >> csInfo.tpDataCodingScheme;
    in >> csInfo.tpValidityPeriod;
    in >> locationInformationMSC;
    //data for CDR generation
    in >> callingSMEid;
    in >> routeId;
    in >> serviceId;
    in >> userMsgRef;
    in >> msgId;
    in >> ussdServiceOp;
    in >> partsNum;
    in >> msgLen;
    in >> forwarded;
}

void ChargeSms::save(ObjectBuffer& out)
{
    out << dstSubscriberNumber;
    out << callingPartyNumber;
    out << callingImsi;
    out << csInfo.smscAddress;
    out << submitTimeTZ;
    out << csInfo.tpShortMessageSpecificInfo;
    out << csInfo.tpProtocolIdentifier;
    out << csInfo.tpDataCodingScheme;
    out << csInfo.tpValidityPeriod;
    out << locationInformationMSC;
    //data for CDR generation
    out << callingSMEid;
    out << routeId;
    out << serviceId;
    out << userMsgRef;
    out << msgId;
    out << ussdServiceOp;
    out << partsNum;
    out << msgLen;
    out << forwarded;
}


void ChargeSms::export2CDR(CDRRecord & cdr) const
{
    cdr._msgId = msgId;
    cdr._mediaType = (parseCBS_DCS(csInfo.tpDataCodingScheme) == CBS_DCS::dcBINARY8) ?
                        CDRRecord::dpBinary : CDRRecord::dpText ;
    cdr._bearer = (ussdServiceOp < 0) ? CDRRecord::dpSMS : CDRRecord::dpUSSD;
    cdr._submitTime = submitTimeTZ;
    cdr._partsNum = partsNum;

    cdr._srcAdr = callingPartyNumber;
    cdr._srcIMSI = callingImsi;
    cdr._srcMSC = locationInformationMSC;
    cdr._srcSMEid = callingSMEid;

    cdr._routeId = routeId;
    cdr._serviceId = serviceId;
    cdr._userMsgRef = userMsgRef;

    cdr._dstAdr = dstSubscriberNumber;
    cdr._dpLength = (uint32_t)msgLen;
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
    : InmanErrorCode(0)
    , value(CHARGING_POSSIBLE)
{
    setObjectId((unsigned short)CHARGE_SMS_RESULT_TAG);
}

ChargeSmsResult::ChargeSmsResult(uint32_t errCode, ChargeSmsResult_t res/* = CHARGING_NOT_POSSIBLE*/)
    : InmanErrorCode(errCode)
    , value(res)
{
    setObjectId((unsigned short)CHARGE_SMS_RESULT_TAG);
}

ChargeSmsResult::ChargeSmsResult(InmanErrorType errType, uint16_t errCode,
                                 ChargeSmsResult_t res/* = CHARGING_NOT_POSSIBLE*/)
    : InmanErrorCode(errType, errCode)
    , value(res)
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

void ChargeSmsResult::load(ObjectBuffer& in) throw(CustomException)
{
    unsigned short v;
    in >> v;
    value = static_cast<ChargeSmsResult_t>(v);
    in >> _errcode;
}

void ChargeSmsResult::save(ObjectBuffer& out)
{
    out << (unsigned short)value;
    out << _errcode;
}

void ChargeSmsResult::handle(SmscHandler* handler)
{
    assert( handler );
    handler->onChargeSmsResult( this );
}

//-----------------------------------------------
// ChargeSmsResult impl
//-----------------------------------------------


DeliverySmsResult::DeliverySmsResult()
    : value(0), final(true)
{
    setObjectId((unsigned short)DELIVERY_SMS_RESULT_TAG);
}

DeliverySmsResult::DeliverySmsResult(uint32_t val, bool finalAttemp /*= true*/)
    : value(val), final(finalAttemp)
{
    setObjectId((unsigned short)DELIVERY_SMS_RESULT_TAG);
}

DeliverySmsResult::~DeliverySmsResult()
{
}

void DeliverySmsResult::setResultValue(uint32_t res)
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
void DeliverySmsResult::setDivertedAdr(const std::string& dvrt_adr)
{
    divertedAdr = dvrt_adr;
}
void DeliverySmsResult::setDeliveryTime(time_t final_tm)
{
    finalTimeTZ = final_tm;
}


uint32_t DeliverySmsResult::GetValue() const
{
    return value;
}

void DeliverySmsResult::load(ObjectBuffer& in) throw(CustomException)
{
    in >> value;
    in >> final;
    //optional data for CDR generation (on successfull delivery)
    in >> destImsi;
    in >> destMSC;
    in >> destSMEid;
    in >> divertedAdr;
    in >> finalTimeTZ;
}

void DeliverySmsResult::save(ObjectBuffer& out)
{
    out << value;
    out << final;
    //optional data for CDR generation (on successfull delivery)
    out << destImsi;
    out << destMSC;
    out << destSMEid;
    out << divertedAdr;
    out << finalTimeTZ;
}

void DeliverySmsResult::export2CDR(CDRRecord & cdr) const
{
    cdr._dlvrRes = value;
    cdr._dstIMSI = destImsi;
    cdr._dstMSC = destMSC;
    cdr._dstSMEid = destSMEid;
    cdr._finalTime = finalTimeTZ;
    cdr._divertedAdr = divertedAdr;
    cdr._cdrType = divertedAdr.size() ? CDRRecord::dpDiverted: CDRRecord::dpOrdinary;
    cdr._finalized = final;
}

void DeliverySmsResult::handle(InmanHandler* handler)
{
    assert( handler );
    handler->onDeliverySmsResult( this );
}

} //interaction
} //inman
} //smsc
