/*
  $Id$
*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "smpp.h"

using namespace smsc::smpp;

int main(void)
{
  fprintf(stderr,"begin test\n");
  try
  {
    {
      int fd = open("/tmp/smpp_pdu_test1",O_RDWR|O_CREAT|O_TRUNC);
      SmppStream stream;
      //assignStreamWith(&stream,fd,false);
      PduMultiSm pdu;
      SmppHeader& pduHeader = pdu.get_header();
      pduHeader.set_commandId(SmppCommandSet::SUBMIT_MULTI);
      PduPartSm& sm = pdu.get_message();
      const char* msg = "short message";
      sm.set_shortMessage(msg,strlen(msg));
      PduAddress& source = sm.get_source();
      source.set_typeOfNumber(TypeOfNumberValue::NETWORK_SPECIFIC);
      source.set_numberingPlan(NumberingPlanValue::INTERNET);
      source.set_value("1.1.1.1");
      PduAddress& dest = sm.get_dest();
      dest.set_typeOfNumber(TypeOfNumberValue::NETWORK_SPECIFIC);
      dest.set_numberingPlan(NumberingPlanValue::INTERNET);
      dest.set_value("1.1.1.1");
      {
        char smpp_time[SMPP_TIME_BUFFER_LENGTH];
        if ( cTime2SmppTime(time(0),smpp_time) )
          sm.set_scheduleDeliveryTime(smpp_time);
        if ( cTime2SmppTime(time(0),smpp_time) )
          sm.set_validityPeriod(smpp_time);
      }
      pdu.get_optional().set_receiptedMessageId("mes");
      pdu.get_optional().set_additionalStatusInfoText("add");
      dump_pdu(reinterpret_cast<SmppHeader*>(&pdu));
      fillSmppPdu(&stream,reinterpret_cast<SmppHeader*>(&pdu));
      close(fd);
    }
    {
      int fd = open("/tmp/smpp_pdu_test1",O_RDWR);
      SmppStream stream;
      auto_ptr<SmppHeader> pdu;
      //assignStreamWith(&stream,fd,true);
      pdu = auto_ptr<SmppHeader>(fetchSmppPdu(&stream));
      __require__( pdu.get() != 0 );
      dump_pdu(pdu.get());
      close(fd);
    }
  }
  catch(...)
  {
    __warning__("catch excpetion");
  }
  //unlink("/tmp/smpp_pdu_test1");
  fprintf(stderr,"end test\n"); */
}
