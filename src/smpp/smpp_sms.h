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
			string short_msg = sms->getStrProperty(Tag::SMPP_SHORT_MESSAGE);
      int msg_length = sms->getIntProperty(Tag::SMPP_SM_LENGTH);
      __require__(msg_length <= (signed)short_msg.length());
      message.set_shortMessage(short_msg.c_str(),msg_length);
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
  //pdu->optional.set_userMessageReference(sms->getMessageReference());
  pdu->optional.set_userMessageReference(
		sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
  //{
    //char buff[64];
    //memset(buff,0,sizeof(buff));
    //sprintf(buff,"%lld",sms->getReceiptSmsId());
    //pdu->optional.set_receiptedMessageId(buff);
  //}
	pdu->optional.set_receiptedMessageId(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
  {
    char buff[7];
    memset(buff,0,sizeof(buff));
    sms->getEServiceType(buff);
    pdu->message.set_serviceType(buff);
  }
  pdu->message.set_esmClass(sms->getIntProperty(Tag::SMPP_ESM_CLASS));
  return true;
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
      Address originatingAddr(strlen(source.value.cstr()),
                            source.typeOfNumber,
                            source.numberingPlan,
                            source.value.cstr());
      Address destinationAddr(strlen(dest.value.cstr()),
                            dest.typeOfNumber,
                            dest.numberingPlan,
                            dest.value.cstr());/**/
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
		sms->setStrProperty(Tag::SMPP_SHORT_MESSAGE,
												message.shortMessage.cstr()?message.shortMessage.cstr():"");
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
  }
  if ( pdu->optional.has_userMessageReference() )
    //sms->setMessageReference(pdu->optional.get_userMessageReference());
		sms->setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
												pdu->optional.get_userMessageReference());
  else
    //sms->setMessageReference(0);
		sms->setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,0);
  
	if ( pdu->optional.has_receiptedMessageId() )
  {
    smsc::sms::SMSId id = 0;
    if ( !sscanf(pdu->optional.get_receiptedMessageId(),"%lld",&id) )
    {
      __warning__("error when processing receiptedMessageId");
      __watch__(pdu->optional.get_receiptedMessageId());
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
  sms->setEServiceType(pdu->message.get_serviceType());
  //sms->setEsmClass(pdu->message.get_esmClass());
	sms->setIntProperty(Tag::SMPP_ESM_CLASS,(uint32_t)pdu->message.get_esmClass());
  return true;
}

};
};

#endif

