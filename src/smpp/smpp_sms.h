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
      /*Address originatingAddr(strlen(source.value.cstr()),
                            source.typeOfNumber,
                            source.numberingPlan,
                            source.value.cstr());
      Address destinationAddr(strlen(dest.value.cstr()),
                            dest.typeOfNumber,
                            dest.numberingPlan,
                            dest.value.cstr());*/
      Address originatingAddr((int)strlen(source.value.cstr()),
                            (unsigned char)source.typeOfNumber,
                            (unsigned char)source.numberingPlan,
                            (const uint8_t*)source.value.cstr());
      Address destinationAddr((int)strlen(dest.value.cstr()),
                            (unsigned char)dest.typeOfNumber,
                            (unsigned char)dest.numberingPlan,
                            (const uint8_t*)dest.value.cstr());
      sms->setOriginatingAddress(originatingAddr);
      sms->setDestinationAddress(destinationAddr);
    }
    
    __require__ ( message.shortMessage.size() == message.smLength );
//    sms->setMessageBody(message.smLength, message.dataCoding, false, message.shortMessage.cstr());
    sms->setMessageBody((unsigned char)message.smLength, (unsigned char)message.dataCoding, false, (uint8_t*)message.shortMessage.cstr());
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

    sms->setStatusReportRequested(message.registredDelivery);
  }
}

};
};

#endif
