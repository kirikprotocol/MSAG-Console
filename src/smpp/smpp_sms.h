/*
  $Id$
*/

//
// ���� ���� �������� ��� ��� ������� � SMS ������������� � ����� SMPP
//

#if !defined __Cxx_Header__smpp_sms_h__
#define __Cxx_Header__smpp_sms_h__

#include "util/debug.h"
#include "smpp_structures.h"
#include "sms/sms.h"
#include "smpp_time.h"

namespace smsc{
namespace smpp{

using smsc::sms::SMS;
using smsc::sms::Address;

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
      __require__ ( val_length <= sizeof(val) );   // fatal if out of range !!!!
      src.set_value(val);
      src.set_typeOfNumber(addr.getTypeOfNumber());
      src.set_numberingPlan(addr.getNumberingPlan());
    }
    PduAddress& dest = message.get_dest();
    {
      char val[21];
      Address& addr = sms->getDestinationAddress();
      int val_length = addr.getValue(val);
      __require__ ( val_length <= sizeof(val) );   // fatal if out of range !!!!
      dest.set_value(val);
      dest.set_typeOfNumber(addr.getTypeOfNumber());
      dest.set_numberingPlan(addr.getNumberingPlan());
    }
    {
      char msg[256];
      const smsc::sms::Body& sms_body = sms->getMessageBody();
      int msg_length = sms_body.getData((uint8_t*)msg);
      __require__(msg_length <= sizeof(msg));
      message.set_shortMessage(msg,msg_length);
      //message.set_smLength((uint8_t)msg_length);
      message.set_dataCoding((uint8_t)sms_body.getCodingScheme());
    }
    message.set_protocolId(sms->getProtocolIdentifier());
    message.set_priorityFlag(sms->getPriority());
    message.set_registredDelivery(/*sms->isStatusReportRequested()*/
				sms->getDeliveryReport());
    {
      char smpp_time[SMPP_TIME_BUFFER_LENGTH];
      if ( cTime2SmppTime(sms->getWaitTime(),smpp_time) )
        message.set_scheduleDeliveryTime(smpp_time);
      if ( cTime2SmppTime(sms->getValidTime(),smpp_time) )
        message.set_validityPeriod(smpp_time);
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
    sms->setMessageBody((unsigned char)message.shortMessage.size(), (unsigned char)message.dataCoding, false, (uint8_t*)message.shortMessage.cstr());
    sms->setProtocolIdentifier(message.protocolId);
    sms->setPriority(message.priorityFlag);
    
    if ( message.scheduleDeliveryTime.cstr() )
      sms->setWaitTime(smppTime2CTime(message.scheduleDeliveryTime));
    else
      sms->setWaitTime(0);
    
    if ( message.validityPeriod.cstr() )
      sms->setValidTime(smppTime2CTime(message.validityPeriod));
    else
      sms->setValidTime(0);

    sms->setDeliveryReport(message.registredDelivery);
  }
}

};
};

#endif
