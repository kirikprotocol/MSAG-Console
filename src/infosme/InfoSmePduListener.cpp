#include "system/status.h"
#include "InfoSmePduListener.h"
#include "SmscConnector.h"
#include "TaskTypes.hpp"

namespace smsc {
namespace infosme {

// using namespace smsc::system;

void InfoSmePduListener::setSyncTransmitter(SmppTransmitter *transmitter) {
  syncTransmitter = transmitter;
}

void InfoSmePduListener::setAsyncTransmitter(SmppTransmitter *transmitter) {
  asyncTransmitter = transmitter;
}

void InfoSmePduListener::processReceipt (SmppHeader *pdu) {
  if (!pdu || !asyncTransmitter) return;
  bool bNeedResponce = true;

  SMS sms;
  fetchSmsFromSmppPdu((PduXSm*)pdu, &sms);
  bool isReceipt = (sms.hasIntProperty(Tag::SMPP_ESM_CLASS)) ?
      ((sms.getIntProperty(Tag::SMPP_ESM_CLASS)&0x3C) == 0x4) : false;

  if (isReceipt && ((PduXSm*)pdu)->get_optional().has_receiptedMessageId())
  {
      const char* msgid = ((PduXSm*)pdu)->get_optional().get_receiptedMessageId();
      if (msgid && msgid[0] != '\0')
      {
          bool delivered = false;
          bool retry = false;

          if (sms.hasIntProperty(Tag::SMPP_MSG_STATE))
          {
              int msgState = sms.getIntProperty(Tag::SMPP_MSG_STATE);
              switch (msgState)
              {
              case SmppMessageState::DELIVERED:
                  delivered = true;
                  break;
              case SmppMessageState::EXPIRED:
              case SmppMessageState::DELETED:
                  retry = true;
                  break;
              case SmppMessageState::ENROUTE:
              case SmppMessageState::UNKNOWN:
              case SmppMessageState::ACCEPTED:
              case SmppMessageState::REJECTED:
              case SmppMessageState::UNDELIVERABLE:
                  break;
              default:
                  smsc_log_warn(logger, "Invalid state=%d received in reciept !", msgState);
                  break;
              }
          }
          ResponseData rd(delivered?0:smsc::system::Status::UNKNOWNERR,0,msgid);
          // ResponseData rd(delivered?0:Status::UNKNOWNERR, ConnectorSeqNum(), msgid);
          rd.retry=retry;

          bNeedResponce = processor.invokeProcessReceipt(rd);
      }
  }

  if (bNeedResponce)
  {
      PduDeliverySmResp smResp;
      smResp.get_header().set_commandId(SmppCommandSet::DELIVERY_SM_RESP);
      smResp.set_messageId("");
      smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
      asyncTransmitter->sendDeliverySmResp(smResp);
  }
}

void InfoSmePduListener::processResponce(SmppHeader *pdu) {
  if (!pdu) return;

  int seqNum = pdu->get_sequenceNumber();
  int status = pdu->get_commandStatus();

  bool accepted  = (status == smsc::system::Status::OK);

  const char* msgid = ((PduXSmResp*)pdu)->get_messageId();
  std::string msgId = "";
  if (!msgid || msgid[0] == '\0') accepted = false;
  else msgId = msgid;

  if (!accepted)
      smsc_log_info(logger, "SMS #%s seqNum=%d wasn't accepted, errcode=%d",
                    msgId.c_str(), seqNum, status);
  processor.invokeProcessResponse(ResponseData(status, seqNum, msgId));
}

void InfoSmePduListener::handleEvent(SmppHeader *pdu) {
  //TDOD: 
  /*
  if (bInfoSmeIsConnecting) {
      infoSmeReady.Wait(infoSmeReadyTimeout);
      if (bInfoSmeIsConnecting) {
          disposePdu(pdu);
          return;
      }
  }*/

  switch (pdu->get_commandId())
  {
  case SmppCommandSet::DELIVERY_SM:
      processReceipt(pdu);
      break;
  case SmppCommandSet::SUBMIT_SM_RESP:
      processResponce(pdu);
      break;
  case SmppCommandSet::ENQUIRE_LINK: case SmppCommandSet::ENQUIRE_LINK_RESP:
      break;
  default:
      smsc_log_debug(logger, "Received unsupported Pdu !");
      break;
  }

  disposePdu(pdu);
}

void InfoSmePduListener::handleError(int errorCode) {
  smsc_log_error(logger, "Transport error handled! Code is: %d", errorCode);
  //TODO:
  //setNeedReconnect();
}

}//infosme
}//smsc
