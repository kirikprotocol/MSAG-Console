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

namespace smsc{
namespace smpp{

using smsc::sms::SMS;
using smsc::sms::Address;

inline bool fillSmppPduFromSms(PduXsm* pdu,SMS* sms)
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

inline bool fetchSmsFromSmppPdu(PduXsm* pdu,SMS* sms)
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
    PduPartSm message& pdu->message;
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
                            dest.value.cstr());
      sms->setOriginatingAddress(oridginatingAddr);
      sms->setDestinationAddress(destinationAddr);
    }
    
    __require__ ( message.shortMessage.size() == message.smLength );
    sms->setMessageBody(message.smLenght, message.dataCoding, false, message.shortMessage.cstr());
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
