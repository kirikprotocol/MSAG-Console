/*
  $Id$
*/

//
// этот файл содержит код для доступа к SMS запакоманному в пакет SMPP
//

#if !defined __Cxx_Header__smpp_sms_h__
#define __Cxx_Header__smpp_sms_h__

#include "util/debug.h"
#include "smpp_structures.h"
#include "sms/sms.h"
#include "smpp_time.h"
#include <string>

namespace smsc{
namespace smpp{

//using smsc::sms::SMS;
//using smsc::sms::Address;
using std::string;
using namespace smsc::sms;

inline void fillOptional(SmppOptional& optional,SMS* sms)
{
  if ( sms->hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) )
  {
    optional.set_userMessageReference(
      sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
  }
  if ( sms->hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) )
  {
    optional.set_receiptedMessageId(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
  }
  if ( sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) )
    optional.set_ussdServiceOp(sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP));
  if ( sms->hasIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT) )
    optional.set_destAddrSubunit(sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT));
  if ( sms->hasIntProperty(Tag::SMPP_PAYLOAD_TYPE) )
    optional.set_payloadType(sms->getIntProperty(Tag::SMPP_PAYLOAD_TYPE));
  if ( sms->hasIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES) )
    optional.set_msMsgWaitFacilities(sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES));
  if ( sms->hasIntProperty(Tag::SMPP_USER_RESPONSE_CODE) )
    optional.set_userResponseCode(sms->getIntProperty(Tag::SMPP_USER_RESPONSE_CODE));
  if ( sms->hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM) )
    optional.set_sarMsgRefNum(sms->getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM));
  if ( sms->hasIntProperty(Tag::SMPP_LANGUAGE_INDICATOR) )
    optional.set_languageIndicator(sms->getIntProperty(Tag::SMPP_LANGUAGE_INDICATOR));
  if ( sms->hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS) )
    optional.set_sarTotalSegments(sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS));
  if ( sms->hasIntProperty(Tag::SMPP_NUMBER_OF_MESSAGES) )
    optional.set_numberOfMessages(sms->getIntProperty(Tag::SMPP_NUMBER_OF_MESSAGES));
  if ( sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) ){
    unsigned len;
    const char * data = sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    optional.set_messagePayload(data,len);
  }

  if( sms->hasIntProperty(Tag::SMPP_MS_VALIDITY) )
    optional.set_msValidity(sms->getIntProperty(Tag::SMPP_MS_VALIDITY));

  if ( sms->hasIntProperty(Tag::SMPP_MSG_STATE) )
  {
    switch(sms->getIntProperty(Tag::SMPP_MSG_STATE))
    {
      case ENROUTE: optional.set_messageState(1);break;
      case DELIVERED: optional.set_messageState(2);break;
      case EXPIRED:optional.set_messageState(3);break;
      case DELETED:optional.set_messageState(4);break;
      case UNDELIVERABLE:optional.set_messageState(5);break;
    };
  }
}

inline bool fillSmppPduFromSms(PduXSm* pdu,SMS* sms)
{
  __require__ ( pdu != NULL );
  __require__ ( sms != NULL );
  __require__ (smppPduHasSms((SmppHeader*)pdu));
  /*if ( pdu->header.comandId == DATA_SM )
  {
    // ..... ????
  }
  else*/
  {
    PduPartSm& message = pdu->get_message();
    PduAddress& src = message.get_source();
    {
      char val[21];
      Address& addr = sms->getOriginatingAddress();
      int val_length = addr.getValue(val);
      __require__ ( val_length <= (signed int)sizeof(val) );   // fatal if out of range !!!!
      src.set_value(val);
      src.set_typeOfNumber(addr.getTypeOfNumber());
      src.set_numberingPlan(addr.getNumberingPlan());
    }
    PduAddress& dest = message.get_dest();
    {
      char val[21];
      Address& addr = sms->getDestinationAddress();
      int val_length = addr.getValue(val);
      __require__ ( val_length <= (signed int)sizeof(val) );   // fatal if out of range !!!!
      dest.set_value(val);
      dest.set_typeOfNumber(addr.getTypeOfNumber());
      dest.set_numberingPlan(addr.getNumberingPlan());
    }
    {
      //char msg[256];
      //const smsc::sms::Body& sms_body = sms->getMessageBody();
      unsigned len = 0;
      const char* short_msg = sms->getBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,&len);
      unsigned msg_length = sms->getIntProperty(Tag::SMPP_SM_LENGTH);
      __require__(msg_length <= len);
      message.set_shortMessage(short_msg,msg_length);
      //message.set_smLength((uint8_t)msg_length);
      //message.set_dataCoding((uint8_t)sms_body.getCodingScheme());
      message.set_dataCoding((uint8_t)sms->getIntProperty(Tag::SMPP_DATA_CODING));
    }
    //message.set_protocolId(sms->getProtocolIdentifier());
    message.set_protocolId(sms->getIntProperty(Tag::SMPP_PROTOCOL_ID));
    //message.set_priorityFlag(sms->getPriority());
    message.set_priorityFlag(sms->getIntProperty(Tag::SMPP_PRIORITY));
    //message.set_registredDelivery(/*sms->isStatusReportRequested()*/
    //    sms->getDeliveryReport());
    message.set_registredDelivery(sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY));
    {
      char smpp_time[SMPP_TIME_BUFFER_LENGTH];
      if ( cTime2SmppTime(sms->getNextTime(),smpp_time) )
        message.set_scheduleDeliveryTime(smpp_time);
      if ( cTime2SmppTime(sms->getValidTime(),smpp_time) )
        message.set_validityPeriod(smpp_time);
    }
  }
  {
    char buff[7];
    memset(buff,0,sizeof(buff));
    sms->getEServiceType(buff);
    pdu->message.set_serviceType(buff);
  }

  if ( sms->hasIntProperty(Tag::SMPP_PRIORITY))
    pdu->message.set_priorityFlag(sms->getIntProperty(Tag::SMPP_PRIORITY));

  pdu->message.set_esmClass(sms->getIntProperty(Tag::SMPP_ESM_CLASS));

  if ( sms->hasIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG) )
    pdu->message.set_replaceIfPresentFlag(sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG));
  else
    pdu->message.set_replaceIfPresentFlag(0);


  fillOptional(pdu->optional,sms);
  return true;
}

inline void fetchOptionals(SmppOptional& optional,SMS* sms)
{
  if ( optional.has_userMessageReference() )
    //sms->setMessageReference(optional.get_userMessageReference());
    sms->setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
                        optional.get_userMessageReference());
  else
    //sms->setMessageReference(0);
    sms->setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,0);

  if ( optional.has_receiptedMessageId() )
  {
    smsc::sms::SMSId id = 0;
    if ( !sscanf(optional.get_receiptedMessageId(),"%lld",&id) )
    {
      __warning__("error when processing receiptedMessageId");
      __watch__(optional.get_receiptedMessageId());
    }
    //sms->setReceiptSmsId(id);
    char buffer[64];
    snprintf(buffer,64,"%lld",id);
    sms->setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,buffer);
  }
  else
  {
    // nothing
    //sms->setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,0);
  }
    //sms->setReceiptSmsId(0);
  if ( optional.has_ussdServiceOp() )
    sms->setIntProperty(Tag::SMPP_USSD_SERVICE_OP,optional.get_ussdServiceOp());
  if ( optional.has_destAddrSubunit() )
    sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,optional.get_destAddrSubunit());
  if ( optional.has_payloadType() )
    sms->setIntProperty(Tag::SMPP_PAYLOAD_TYPE,optional.get_payloadType());
  if ( optional.has_msMsgWaitFacilities() )
    sms->setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,optional.get_msMsgWaitFacilities());
  if ( optional.has_userResponseCode() )
    sms->setIntProperty(Tag::SMPP_USER_RESPONSE_CODE,optional.get_userResponseCode());
  if ( optional.has_sarMsgRefNum() )
    sms->setIntProperty(Tag::SMPP_SAR_MSG_REF_NUM,optional.get_sarMsgRefNum());
  if ( optional.has_languageIndicator() )
    sms->setIntProperty(Tag::SMPP_LANGUAGE_INDICATOR,optional.get_languageIndicator());
  if ( optional.has_sarTotalSegments() )
    sms->setIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS,optional.get_sarTotalSegments());
  if ( optional.has_numberOfMessages() )
    sms->setIntProperty(Tag::SMPP_NUMBER_OF_MESSAGES,optional.get_numberOfMessages());
  if ( optional.has_messagePayload() )
    sms->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,
                               optional.get_messagePayload(),
                               optional.size_messagePayload());

  if(optional.has_msValidity())
    sms->setIntProperty(Tag::SMPP_MS_VALIDITY,optional.get_msValidity());
  if ( optional.has_messageState() )
  {
    switch(optional.get_messageState())
    {
      case 0:sms->setIntProperty(Tag::SMPP_MSG_STATE,ENROUTE);break;
      case 1:sms->setIntProperty(Tag::SMPP_MSG_STATE,DELIVERED);break;
      case 2:sms->setIntProperty(Tag::SMPP_MSG_STATE,EXPIRED);break;
      case 3:sms->setIntProperty(Tag::SMPP_MSG_STATE,DELETED);break;
      case 4:sms->setIntProperty(Tag::SMPP_MSG_STATE,UNDELIVERABLE);break;
      default: __warning__("UNKNOWN MESSAGE STATE");break;
    }
  }
}

inline bool fetchSmsFromSmppPdu(PduXSm* pdu,SMS* sms)
{
  __require__ ( pdu != NULL );
  __require__ ( sms != NULL );
  __require__ (smppPduHasSms((SmppHeader*)pdu));
 /* if ( pdu->header.comandId == DATA_SM )
  {
    // ..... ????
  }
  else*/
  {
    PduPartSm& message = pdu->message;
    { // fill address
      PduAddress& source = message.source;
      PduAddress& dest  = message.dest;
      Address originatingAddr(source.value.cstr()?strlen(source.value.cstr()):1,
                            source.typeOfNumber,
                            source.numberingPlan,
                            source.value.cstr()?source.value.cstr():"0");
      Address destinationAddr(dest.value.cstr()?strlen(dest.value.cstr()):1,
                            dest.typeOfNumber,
                            dest.numberingPlan,
                            dest.value.cstr()?dest.value.cstr():"0");/**/
  /*    Address originatingAddr((int)strlen(source.value.cstr()),
                            (unsigned char)source.typeOfNumber,
                            (unsigned char)source.numberingPlan,
                            (const uint8_t*)source.value.cstr());
      Address destinationAddr((int)strlen(dest.value.cstr()),
                            (unsigned char)dest.typeOfNumber,
                            (unsigned char)dest.numberingPlan,
                            (const uint8_t*)dest.value.cstr());*/
      sms->setOriginatingAddress(originatingAddr);
      sms->setDestinationAddress(destinationAddr);
    }

    //__require__ ( message.shortMessage.size() == message.smLength );
    //sms->setMessageBody(message.smLength, message.dataCoding, false, message.shortMessage.cstr());
    //sms->setMessageBody((unsigned char)message.shortMessage.size(), (unsigned char)message.dataCoding, false, (uint8_t*)message.shortMessage.cstr());
    sms->setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE,
                        message.shortMessage.cstr()?message.shortMessage.cstr():"",message.shortMessage.length);
    sms->setIntProperty(Tag::SMPP_SM_LENGTH,(uint32_t)message.shortMessage.size());
    sms->setIntProperty(Tag::SMPP_DATA_CODING,(uint32_t)message.dataCoding);
    sms->setIntProperty(Tag::SMPP_PRIORITY,(uint32_t)message.priorityFlag);
    sms->setIntProperty(Tag::SMPP_PROTOCOL_ID,(uint32_t)message.protocolId);

    if ( message.scheduleDeliveryTime.cstr() )
      sms->setNextTime(smppTime2CTime(message.scheduleDeliveryTime));
    else
      sms->setNextTime(0);

    if ( message.validityPeriod.cstr() )
      sms->setValidTime(smppTime2CTime(message.validityPeriod));
    else
      sms->setValidTime(0);

    //sms->setDeliveryReport(message.registredDelivery);
    sms->setIntProperty(Tag::SMPP_REGISTRED_DELIVERY,(uint32_t)message.registredDelivery);
    sms->setIntProperty(Tag::SMPP_PRIORITY,pdu->message.get_priorityFlag());
    sms->setIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG,pdu->message.get_replaceIfPresentFlag());
  }
  sms->setEServiceType(pdu->message.get_serviceType());
  sms->setIntProperty(Tag::SMPP_ESM_CLASS,(uint32_t)pdu->message.get_esmClass());
  fetchOptionals(pdu->optional,sms);
  return true;
}

inline bool fetchSmsFromDataSmPdu(PduDataSm* pdu,SMS* sms)
{
  PduDataPartSm& data = pdu->get_data();
  { // fill address
    PduAddress& source = data.source;
    PduAddress& dest  = data.dest;
    Address originatingAddr(source.value.cstr()?strlen(source.value.cstr()):1,
                          source.typeOfNumber,
                          source.numberingPlan,
                          source.value.cstr()?source.value.cstr():"0");
    Address destinationAddr(dest.value.cstr()?strlen(dest.value.cstr()):1,
                          dest.typeOfNumber,
                          dest.numberingPlan,
                          dest.value.cstr()?dest.value.cstr():"0");
    sms->setOriginatingAddress(originatingAddr);
    sms->setDestinationAddress(destinationAddr);
  }
  sms->setIntProperty(Tag::SMPP_ESM_CLASS,(uint32_t)data.get_esmClass());
  sms->setIntProperty(Tag::SMPP_REGISTRED_DELIVERY,data.get_registredDelivery());
  sms->setIntProperty(Tag::SMPP_DATA_CODING,data.get_dataCoding());
  fetchOptionals(pdu->optional,sms);
  sms->setValidTime(time(NULL)+pdu->optional.get_qosTimeToLive());
  sms->setIntProperty(Tag::SMPP_PRIORITY,0);
  sms->setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
  sms->setIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG,0);
  sms->setNextTime(0);
  return true;
}

inline bool fillDataSmFromSms(PduDataSm* pdu,SMS* sms)
{
  PduDataPartSm& data = pdu->get_data();
  PduAddress& src = data.get_source();
  {
    char val[21];
    Address& addr = sms->getOriginatingAddress();
    int val_length = addr.getValue(val);
    __require__ ( val_length <= (signed int)sizeof(val) );   // fatal if out of range !!!!
    src.set_value(val);
    src.set_typeOfNumber(addr.getTypeOfNumber());
    src.set_numberingPlan(addr.getNumberingPlan());
  }
  PduAddress& dest = data.get_dest();
  {
    char val[21];
    Address& addr = sms->getDestinationAddress();
    int val_length = addr.getValue(val);
    __require__ ( val_length <= (signed int)sizeof(val) );   // fatal if out of range !!!!
    dest.set_value(val);
    dest.set_typeOfNumber(addr.getTypeOfNumber());
    dest.set_numberingPlan(addr.getNumberingPlan());
  }
  fillOptional(pdu->optional,sms);
  data.set_dataCoding((uint8_t)sms->getIntProperty(Tag::SMPP_DATA_CODING));
  data.set_registredDelivery(sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY));
  return true;
}

};
};

#endif
