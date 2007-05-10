static char const ident[] = "$Id$";

#include <assert.h>

#include "inman/interaction/MsgBilling.hpp"

#include "inman/common/cvtutil.hpp"
using smsc::cbs::parseCBS_DCS;
using smsc::cbs::CBS_DCS;

namespace smsc  {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * Billing CommandSet:
 * ************************************************************************** */
INPCSBilling::INPCSBilling()
{
    pckFct.registerProduct(mkPckIdx(CHARGE_SMS_TAG, HDR_DIALOG), 
                           new PckFactory::ProducerT< SPckChargeSms >());
    pckFct.registerProduct(mkPckIdx(CHARGE_SMS_RESULT_TAG, HDR_DIALOG), 
                           new PckFactory::ProducerT< SPckChargeSmsResult >());
    pckFct.registerProduct(mkPckIdx(DELIVERY_SMS_RESULT_TAG, HDR_DIALOG), 
                           new PckFactory::ProducerT< SPckDeliverySmsResult >());
/*
    objFct.registerProduct(CHARGE_SMS_TAG, new ObjFactory::ProducerT< ChargeSms >());
    objFct.registerProduct(CHARGE_SMS_RESULT_TAG, new ObjFactory::ProducerT< ChargeSmsResult >());
    objFct.registerProduct(DELIVERY_SMS_RESULT_TAG, new ObjFactory::ProducerT< DeliverySmsResult >());

    hdrFct.registerProduct(HDR_DIALOG, new ObjFactory::ProducerT<CsBillingHdr_dlg>());
    hdrFct.registerProduct(HDR_SESSIONED_DLG, new ObjFactory::ProducerT<CsBillingHdr_sess>());
*/
}

/* ************************************************************************** *
 * Billing command headers:
 * ************************************************************************** */
CsBillingHdr_dlg::CsBillingHdr_dlg()
    : SerializableObjectAC(INPCSBilling::HDR_DIALOG)
{ }
CsBillingHdr_sess::CsBillingHdr_sess()
    : SerializableObjectAC(INPCSBilling::HDR_SESSIONED_DLG)
{ }

/* ************************************************************************** *
 * class ChargeSms implementation:
 * ************************************************************************** */
ChargeSms::ChargeSms()
    : INPBillingCmd(INPCSBilling::CHARGE_SMS_TAG)
    , partsNum(1), forwarded(false), extCode(0), smsXSrvsId(0)
{ }

void ChargeSms::load(ObjectBuffer& in) throw(SerializerException)
{
    in >> extCode;
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
    //extensions
    if (extCode) {
        in >> smsXSrvsId;
    }
}

void ChargeSms::save(ObjectBuffer& out) const
{
    out << extCode;
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
    //extensions
    if (extCode) {
        out << smsXSrvsId;
    }
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
    cdr._smsXMask = smsXSrvsId;
}

//-----------------------------------------------
// ChargeSmsResult impl
//-----------------------------------------------
ChargeSmsResult::ChargeSmsResult()
    : INPBillingCmd(INPCSBilling::CHARGE_SMS_RESULT_TAG)
    , value(CHARGING_POSSIBLE), errCode(0)
{
}

ChargeSmsResult::ChargeSmsResult(ChargeSmsResult_t res/* = CHARGING_NOT_POSSIBLE*/,
                                uint32_t err_code/* = 0*/, const char * err_msg/* = NULL*/)
    : INPBillingCmd(INPCSBilling::CHARGE_SMS_RESULT_TAG)
    , value(res), errCode(err_code)
{
    if (err_msg)
        errMsg = err_msg;
}


void ChargeSmsResult::load(ObjectBuffer& in) throw(SerializerException)
{
    unsigned short v;
    in >> v;
    value = static_cast<ChargeSmsResult_t>(v);
    in >> errCode;
    in >> errMsg;
}

void ChargeSmsResult::save(ObjectBuffer& out) const
{
    out << (unsigned short)value;
    out << errCode;
    out << errMsg;
}

//-----------------------------------------------
// ChargeSmsResult impl
//-----------------------------------------------
DeliverySmsResult::DeliverySmsResult()
    : INPBillingCmd(INPCSBilling::DELIVERY_SMS_RESULT_TAG)
    , value(0), final(true)
{
}

DeliverySmsResult::DeliverySmsResult(uint32_t val, bool finalAttemp /*= true*/)
    : INPBillingCmd(INPCSBilling::DELIVERY_SMS_RESULT_TAG)
    , value(val), final(finalAttemp)
{
}

void DeliverySmsResult::load(ObjectBuffer& in) throw(SerializerException)
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

void DeliverySmsResult::save(ObjectBuffer& out) const
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

} //interaction
} //inman
} //smsc
