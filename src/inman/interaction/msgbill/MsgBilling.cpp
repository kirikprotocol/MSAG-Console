#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/interaction/msgbill/MsgBilling.hpp"
#include "inman/interaction/serializer/SerializeIntegers.hpp"
#include "inman/interaction/serializer/SerializeStdString.hpp"
#include "inman/interaction/serializer/SerializeFxdLenStringT.hpp"

#include "inman/common/cvtutil.hpp"
using smsc::cbs::parseCBS_DCS;
using smsc::cbs::CBS_DCS;

namespace smsc  {
namespace inman {
namespace interaction {
/* ************************************************************************** *
 * class INPBillingHdr_dlg implementation:
 * ************************************************************************** */
void INPBillingHdr_dlg::load(PacketBufferAC & in_buf) throw(SerializerException)
{
  in_buf >> dlgId;
}

void INPBillingHdr_dlg::save(PacketBufferAC & out_buf) const throw(SerializerException)
{
  out_buf << dlgId;
}

/* ************************************************************************** *
 * class ChargeSms implementation:
 * ************************************************************************** */
void ChargeSms::load(PacketBufferAC & in_buf) throw(SerializerException)
{
  //service fields
  in_buf >> chrgFlags;
  in_buf >> smsXSrvsId;
  //data for IN interaction
  in_buf >> dstSubscriberNumber;
  in_buf >> callingPartyNumber;
  in_buf >> callingImsi;
  in_buf >> csInfo.smscAddress;
  in_buf >> submitTimeTZ;
  in_buf >> csInfo.tpShortMessageSpecificInfo;
  in_buf >> csInfo.tpProtocolIdentifier;
  in_buf >> csInfo.tpDataCodingScheme;
  in_buf >> csInfo.tpValidityPeriod;
  in_buf >> locationInformationMSC;
    //data for CDR generation
  in_buf >> callingSMEid;
  in_buf >> routeId;
  in_buf >> serviceId;
  in_buf >> userMsgRef;
  in_buf >> msgId;
  in_buf >> ussdServiceOp;
  in_buf >> partsNum;
  in_buf >> msgLen;
  unsigned char cm;
  in_buf >> cm;
  chrgPolicy = static_cast<CDRRecord::ChargingPolicy>(cm);
  in_buf >> dsmSrvType;
}

void ChargeSms::save(PacketBufferAC & out_buf) const  throw(SerializerException)
{
  //service fields
  out_buf << chrgFlags;
  out_buf << smsXSrvsId;
  //data for IN interaction
  out_buf << dstSubscriberNumber;
  out_buf << callingPartyNumber;
  out_buf << callingImsi;
  out_buf << csInfo.smscAddress;
  out_buf << submitTimeTZ;
  out_buf << csInfo.tpShortMessageSpecificInfo;
  out_buf << csInfo.tpProtocolIdentifier;
  out_buf << csInfo.tpDataCodingScheme;
  out_buf << csInfo.tpValidityPeriod;
  out_buf << locationInformationMSC;
  //data for CDR generation
  out_buf << callingSMEid;
  out_buf << routeId;
  out_buf << serviceId;
  out_buf << userMsgRef;
  out_buf << msgId;
  out_buf << ussdServiceOp;
  out_buf << partsNum;
  out_buf << msgLen;
  out_buf << ((unsigned char)chrgPolicy);
  out_buf << dsmSrvType;
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
  cdr._dsmSrvType = dsmSrvType;
  cdr._userMsgRef = userMsgRef;

  cdr._dstAdr = dstSubscriberNumber;
  cdr._dpLength = (uint32_t)msgLen;
  cdr._smsXMask = smsXSrvsId;
  cdr._chargePolicy = chrgPolicy;
  cdr._chargeType = (chrgFlags & chrgMT) ? CDRRecord::MT_Charge : CDRRecord::MO_Charge;
  cdr._finalized = CDRRecord::dpSubmitted;
}

//-----------------------------------------------
// ChargeSmsResult impl
//-----------------------------------------------
void ChargeSmsResult::load(PacketBufferAC & in_buf) throw(SerializerException)
{
  unsigned short v;
  in_buf >> v;
  value = static_cast<ChargeSmsResult_e>(v);
  in_buf >> errCode;
  in_buf >> errMsg;

  unsigned char tmp;
  in_buf >> tmp;
  contract = static_cast<CDRRecord::ContractType>(tmp);
}

void ChargeSmsResult::save(PacketBufferAC & out_buf) const  throw(SerializerException)
{
  out_buf << (unsigned short)value;
  out_buf << errCode;
  out_buf << errMsg;
  out_buf << (unsigned char)contract;
}

/* ************************************************************************** *
 * class DeliveredSmsData implementation:
 * ************************************************************************** */
void DeliveredSmsData::load(PacketBufferAC & in_buf) throw(SerializerException)
{
  //service fields
  in_buf >> chrgFlags;
  in_buf >> smsXSrvsId;
  //Charging request data ..
  in_buf >> dstSubscriberNumber;
  in_buf >> callingPartyNumber;
  in_buf >> callingImsi;
  in_buf >> csInfo.smscAddress;
  in_buf >> submitTimeTZ;
  in_buf >> csInfo.tpShortMessageSpecificInfo;
  in_buf >> csInfo.tpProtocolIdentifier;
  in_buf >> csInfo.tpDataCodingScheme;
  in_buf >> csInfo.tpValidityPeriod;
  in_buf >> locationInformationMSC;
  //data for CDR generation
  in_buf >> callingSMEid;
  in_buf >> routeId;
  in_buf >> serviceId;
  in_buf >> userMsgRef;
  in_buf >> msgId;
  in_buf >> ussdServiceOp;
  in_buf >> partsNum;
  in_buf >> msgLen;
  unsigned char cm;
  in_buf >> cm;
  chrgPolicy = static_cast<CDRRecord::ChargingPolicy>(cm);
  in_buf >> dsmSrvType;

  //Delivery report data ..
  in_buf >> dlvrRes;
  in_buf >> destImsi;
  in_buf >> destMSC;
  in_buf >> destSMEid;
  in_buf >> divertedAdr;
  in_buf >> finalTimeTZ;
}

void DeliveredSmsData::save(PacketBufferAC & out_buf) const  throw(SerializerException)
{
  //service fields
  out_buf << chrgFlags;
  out_buf << smsXSrvsId;
  //Charging request data ..
  out_buf << dstSubscriberNumber;
  out_buf << callingPartyNumber;
  out_buf << callingImsi;
  out_buf << csInfo.smscAddress;
  out_buf << submitTimeTZ;
  out_buf << csInfo.tpShortMessageSpecificInfo;
  out_buf << csInfo.tpProtocolIdentifier;
  out_buf << csInfo.tpDataCodingScheme;
  out_buf << csInfo.tpValidityPeriod;
  out_buf << locationInformationMSC;
  //data for CDR generation
  out_buf << callingSMEid;
  out_buf << routeId;
  out_buf << serviceId;
  out_buf << userMsgRef;
  out_buf << msgId;
  out_buf << ussdServiceOp;
  out_buf << partsNum;
  out_buf << msgLen;
  out_buf << ((unsigned char)chrgPolicy);
  out_buf << dsmSrvType;

  //Delivery report data ..
  out_buf << dlvrRes;
  out_buf << destImsi;
  out_buf << destMSC;
  out_buf << destSMEid;
  out_buf << divertedAdr;
  out_buf << finalTimeTZ;
}

void DeliveredSmsData::export2CDR(CDRRecord & cdr) const
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
  cdr._dsmSrvType = dsmSrvType;
  cdr._userMsgRef = userMsgRef;

  cdr._dstAdr = dstSubscriberNumber;
  cdr._dpLength = (uint32_t)msgLen;
  cdr._smsXMask = smsXSrvsId;
  cdr._chargePolicy = chrgPolicy;
  cdr._chargeType = (chrgFlags & ChargeSms::chrgMT) ? CDRRecord::MT_Charge : CDRRecord::MO_Charge;

  //Delivery report data ..
  cdr._dlvrRes = dlvrRes;
  cdr._dstIMSI = destImsi;
  cdr._dstMSC = destMSC;
  cdr._dstSMEid = destSMEid;
  cdr._finalTime = finalTimeTZ;
  cdr._divertedAdr = divertedAdr;
  cdr._cdrType = !divertedAdr.empty() ? CDRRecord::dpDiverted: CDRRecord::dpOrdinary;
  cdr._finalized = CDRRecord::dpCollected;
}


/* ************************************************************************** *
 * class DeliverySmsResult implementation:
 * ************************************************************************** */
void DeliverySmsResult::load(PacketBufferAC & in_buf) throw(SerializerException)
{
  in_buf >> value;
  in_buf >> final;
  //optional data for CDR generation (on successfull delivery)
  in_buf >> destImsi;
  in_buf >> destMSC;
  in_buf >> destSMEid;
  in_buf >> divertedAdr;
  in_buf >> finalTimeTZ;
}

void DeliverySmsResult::save(PacketBufferAC & out_buf) const throw(SerializerException)
{
  out_buf << value;
  out_buf << final;
    //optional data for CDR generation (on successfull delivery)
  out_buf << destImsi;
  out_buf << destMSC;
  out_buf << destSMEid;
  out_buf << divertedAdr;
  out_buf << finalTimeTZ;
}

void DeliverySmsResult::export2CDR(CDRRecord & cdr) const
{
  cdr._dlvrRes = value;
  cdr._dstIMSI = destImsi;
  cdr._dstMSC = destMSC;
  cdr._dstSMEid = destSMEid;
  cdr._finalTime = finalTimeTZ;
  cdr._divertedAdr = divertedAdr;
  cdr._cdrType = !divertedAdr.empty() ? CDRRecord::dpDiverted: CDRRecord::dpOrdinary;
  cdr._finalized = final ? CDRRecord::dpCollected : CDRRecord::dpDelivered;
}

} //interaction
} //inman
} //smsc
