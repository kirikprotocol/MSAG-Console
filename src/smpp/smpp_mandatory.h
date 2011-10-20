/*
  $Id$
*/


///
/// ���� ���� �������� ��� ������������ ����� �� ������ �
/// ��� ������������ ����� � �����
///

#if !defined __Cxx_Header__smpp_mandatory_h__
#define __Cxx_Header__smpp_mandatory_h__

#include <memory>
#include "util/debug.h"
#include "smpp_structures.h"
#include "smpp_optional.h"
#include "smpp_stream.h"

namespace smsc{
namespace smpp{

using std::auto_ptr;

class BadDataException {};

static inline void fetchPduAddress(SmppStream* stream,PduAddress& addr)
{
  fetchX(stream,addr.typeOfNumber);
  fetchX(stream,addr.numberingPlan);
  fetchCOctetStr(stream,addr.value,21);
}

static inline void fillPduAddress(SmppStream* stream,PduAddress& addr)
{
  fillX(stream,addr.typeOfNumber);
  fillX(stream,addr.numberingPlan);
  fillCOctetStr(stream,addr.value);
}

static inline bool fillSmppPdu(SmppStream* stream,SmppHeader* _pdu)
{
  using namespace SmppCommandSet;
  uint32_t cmdid = _pdu->commandId;
  try
  {
    _pdu->set_commandLength(calcSmppPacketLength(_pdu));

    switch( cmdid )
    {
    case BIND_RECIEVER:
    case BIND_TRANSMITTER:
    case BIND_TRANCIEVER:
    {
      PduBindTRX* pdu = reinterpret_cast<PduBindTRX*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->systemId);
      fillCOctetStr(stream,pdu->password);
      fillCOctetStr(stream,pdu->systemType);
      fillX(stream,pdu->interfaceVersion);
      fillPduAddress(stream,pdu->addressRange);
      return true;
    }
    case BIND_TRANSMITTER_RESP:
    case BIND_RECIEVER_RESP:
    case BIND_TRANCIEVER_RESP:
    {
      PduBindTRXResp* pdu = reinterpret_cast<PduBindTRXResp*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->systemId);
      fillX(stream,SmppOptionalTags::scInterfaceVersion);
      fillX(stream,(uint16_t)1);
      fillX(stream,pdu->scInterfaceVersion);
      return true;
    }
    case SUBMIT_SM:
    case DELIVERY_SM:
    case SUBMIT_MULTI:
    {
      PduXSm* pdu = reinterpret_cast<PduXSm*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      PduPartSm& sm = pdu->message;
      fillCOctetStr(stream,sm.serviceType);
      fillPduAddress(stream,sm.source);
      if ( cmdid == SUBMIT_MULTI )
      {
        fillX(stream, sm.numberOfDests);
        if ( sm.numberOfDests ) {
          for ( int i=0; i < sm.numberOfDests; ++i )
          {
            fillX(stream,sm.dests[i].flag);
            if ( sm.dests[i].flag == 0x01 )
            {
              fillPduAddress(stream,sm.dests[i]);
            }
            else if ( sm.dests[i].flag == 0x02 )
            {
              fillCOctetStr(stream,sm.dests[i].value);
            }
            else
            {
              __warning2__ ("flag value %x for dest address is unknown ",sm.dests[i].flag);
              //goto trap;
              throw BadDataException();
            }
          }
        }
      }
      else
      {
        fillPduAddress(stream,sm.dest);
      }
      fillX(stream,sm.esmClass);
      fillX(stream,sm.protocolId);
      fillX(stream,sm.priorityFlag);
      fillX(stream,sm.scheduleDeliveryTime);
      fillX(stream,sm.validityPeriod);
      fillX(stream,sm.registredDelivery);
      fillX(stream,sm.replaceIfPresentFlag);
      fillX(stream,sm.dataCoding);
      fillX(stream,sm.smDefaultMsgId);
      fillX(stream,((uint8_t)sm.shortMessage.size()));
      fillX(stream,sm.shortMessage);
      fillSmppOptional(stream,&pdu->optional);
      return true;
    }
    case SUBMIT_SM_RESP:
    case DELIVERY_SM_RESP:
    {
      PduXSmResp* pdu = reinterpret_cast<PduXSmResp*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      if(!(cmdid==SUBMIT_SM_RESP && pdu->header.commandStatus!=0))
      {
        fillCOctetStr(stream,pdu->messageId);
#ifdef SMPPRESPHASOPTS
        fillSmppOptional(stream,&pdu->optional);
#endif
      }
      if(pdu->has_ussdSessionId())
      {
        fillX(stream,SmppOptionalTags::ussd_session_id);
        fillX(stream,(uint16_t)4);
        fillX(stream,pdu->get_ussdSessionId());
      }
      return true;
    }
    case SUBMIT_MULTI_RESP:
    {
      PduMultiSmResp* pdu = reinterpret_cast<PduMultiSmResp*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->messageId);
      fillX(stream,pdu->noUnsuccess);
      if ( pdu->noUnsuccess )
      {
        for ( int i=0; i<pdu->noUnsuccess; ++i )
        {
          fillPduAddress(stream,pdu->sme[i].addr);
          fillX(stream,pdu->sme[i].errorStatusCode);
        }
      }
      return true;
    }
    case UNBIND:
    case UNBIND_RESP:
    case GENERIC_NACK:
    case ENQUIRE_LINK:
    case ENQUIRE_LINK_RESP:
    case REPLACE_SM_RESP:
    case CANCEL_SM_RESP:
    {
      fillSmppHeader(stream,*_pdu);
      return true;
    }
    case OUTBIND:
    {
      PduOutBind* pdu = reinterpret_cast<PduOutBind*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->systemId);
      fillCOctetStr(stream,pdu->password);
      return true;
    }
    //case REPLACE_SM:
    //case CANCEL_SM:
    //case ALERT_NOTIFICATION:
    //case DATA_SM:
    //case DATA_SM_RESP:
    //case QUERY_SM:
    //case QUERY_SM_RESP:
    case REPLACE_SM:
    {
      PduReplaceSm* pdu = reinterpret_cast<PduReplaceSm*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->messageId);
      fillPduAddress(stream,pdu->source);
      fillCOctetStr(stream,pdu->scheduleDeliveryTime);
      fillCOctetStr(stream,pdu->validityPeriod);
      fillX(stream,pdu->registredDelivery);
      fillX(stream,pdu->smDefaultMsgId);
      fillX(stream,((uint8_t)pdu->shortMessage.size()));
      fillOctetStr(stream,pdu->shortMessage);
      return true;
    }
    case CANCEL_SM:
    {
      PduCancelSm* pdu = reinterpret_cast<PduCancelSm*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->serviceType);
      fillCOctetStr(stream,pdu->messageId);
      fillPduAddress(stream,pdu->source);
      fillPduAddress(stream,pdu->dest);
      return true;
    }
    case ALERT_NOTIFICATION:
    {
      PduAlertNotification* pdu = reinterpret_cast<PduAlertNotification*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillPduAddress(stream,pdu->source);
      fillPduAddress(stream,pdu->esme);
      fillSmppOptional(stream,&pdu->optional);
      return true;
    }
    case DATA_SM:
    {
      PduDataSm* pdu = reinterpret_cast<PduDataSm*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->data.serviceType);
      fillPduAddress(stream,pdu->data.source);
      fillPduAddress(stream,pdu->data.dest);
      fillX(stream,pdu->data.esmClass);
      fillX(stream,pdu->data.registredDelivery);
      fillX(stream,pdu->data.dataCoding);
      fillSmppOptional(stream,&pdu->optional);
      return true;
    }
    case DATA_SM_RESP:
    {
      PduDataSmResp* pdu = reinterpret_cast<PduDataSmResp*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->messageId);
      fillSmppOptional(stream,&pdu->optional);
      return true;
    }
    case QUERY_SM:
    {
      PduQuerySm* pdu = reinterpret_cast<PduQuerySm*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->messageId);
      fillPduAddress(stream,pdu->source);
      return true;
    }
    case QUERY_SM_RESP:
    {
      PduQuerySmResp* pdu = reinterpret_cast<PduQuerySmResp*>(_pdu);
      fillSmppHeader(stream,pdu->header);
      fillCOctetStr(stream,pdu->messageId);
      fillCOctetStr(stream,pdu->finalDate);
      fillX(stream,pdu->messageState);
      fillX(stream,pdu->errorCode);
      return true;
    }
    //default:
    //  goto trap;
    //  break;
    }
    __warning__("bad smpp pdu");
    return false;
  }
  catch(...)
  {
    __warning__ ("fill pdu error");
    //throw;
    return false;
  }
  return true;
}


class StreamGuard
{
  SmppStream* stream;
public:
  StreamGuard(SmppStream* stream):stream(stream){}
  ~StreamGuard()
  {
    if (stream->dataOffset < stream->dataLength)
    {
      __warning2__("packet %08X has left data, dropped",stream->header.commandId);
      dropPdu(stream);
    }
  }
};

/**
  ������� ������������ ����
*/
static inline SmppHeader* fetchSmppPdu(SmppStream* stream)
{
  using namespace SmppCommandSet;
  __check_smpp_stream_invariant__ ( stream );
  StreamGuard guard(stream);
  try
  {
    uint32_t cmdid = smppCommandId(stream);
    switch ( cmdid )
    {
    case BIND_RECIEVER:
    case BIND_TRANSMITTER:
    case BIND_TRANCIEVER:
    {
      auto_ptr<PduBindTRX> pdu(new PduBindTRX());
      fetchSmppHeader(stream,pdu->header);
      fetchCOctetStr(stream,pdu->systemId,16);
      fetchCOctetStr(stream,pdu->password,9);
      fetchCOctetStr(stream,pdu->systemType,13);
      fetchX(stream,pdu->interfaceVersion);
      //fetchPduAddress(stream,pdu->addressRange);
      fetchX(stream,pdu->addressRange.typeOfNumber);
      fetchX(stream,pdu->addressRange.numberingPlan);
      fetchCOctetStr(stream,pdu->addressRange.value,41);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case BIND_TRANSMITTER_RESP:
    case BIND_RECIEVER_RESP:
    case BIND_TRANCIEVER_RESP:
    {
      auto_ptr<PduBindTRXResp> pdu(new PduBindTRXResp());
      fetchSmppHeader(stream,pdu->header);
      fetchCOctetStr(stream,pdu->systemId,16);
      if ( stream->dataOffset < stream->dataLength  )
      {
        uint16_t tag;
        uint16_t length;
        fetchX(stream,tag);
        fetchX(stream,length);
        __goto_if_fail__ ( tag == /*sc_interface_version(5.3.2.25)*/ 0x0210 , trap);
        fetchX(stream,pdu->scInterfaceVersion);
      }
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case SUBMIT_SM:
    case DELIVERY_SM:
    case SUBMIT_MULTI:
    {
      auto_ptr<PduXSm> pdu(new PduXSm());
      fetchSmppHeader(stream,pdu->header);
      PduPartSm& sm = pdu->message;
      fetchCOctetStr(stream,sm.serviceType,6);
      fetchPduAddress(stream,sm.source);
      if ( cmdid == SUBMIT_MULTI )
      {
        fetchX(stream, sm.numberOfDests);
        if ( sm.numberOfDests ) {
          sm.dests = new PduDestAddress[sm.numberOfDests];
          for ( int i=0; i < sm.numberOfDests; ++i )
          {
            fetchX(stream,sm.dests[i].flag);
            if ( sm.dests[i].flag == 0x01 )
            {
              fetchPduAddress(stream,sm.dests[i]);
            }
            else if ( sm.dests[i].flag == 0x02 )
            {
              fetchCOctetStr(stream,sm.dests[i].value,21);
            }
            else
            {
              __warning2__ ("flag value %x for dest address is unknown ",sm.dests[i].flag);
              goto trap;
            }
          }
        }
      }
      else
      {
        fetchPduAddress(stream,sm.dest);
      }
      fetchX(stream,sm.esmClass);
      fetchX(stream,sm.protocolId);
      fetchX(stream,sm.priorityFlag);
      fetchCOctetStr(stream,sm.scheduleDeliveryTime,17);
      fetchCOctetStr(stream,sm.validityPeriod,17);
      fetchX(stream,sm.registredDelivery);
      fetchX(stream,sm.replaceIfPresentFlag);
      fetchX(stream,sm.dataCoding);
      fetchX(stream,sm.smDefaultMsgId);
      uint8_t smLength = 0;
      fetchX(stream,smLength);
      sm.shortMessage.dispose();
      if ( smLength )
        fetchOctetStr(stream,sm.shortMessage,smLength);
      fetchSmppOptional(stream,&pdu->optional);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case SUBMIT_SM_RESP:
    case DELIVERY_SM_RESP:
    {
      auto_ptr<PduXSmResp> pdu(new PduXSmResp());
      fetchSmppHeader(stream,pdu->header);
      if(!(cmdid==SUBMIT_SM_RESP && pdu->header.commandStatus!=0))
      {
        fetchCOctetStr(stream,pdu->messageId,65);
#ifdef SMPPRESPHASOPTS
        fetchSmppOptional(stream,&pdu->optional);
#endif
      }
      // FIXME: add ussdSessionId
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case SUBMIT_MULTI_RESP:
    {
      auto_ptr<PduMultiSmResp> pdu(new PduMultiSmResp());
      fetchSmppHeader(stream,pdu->header);
      fetchCOctetStr(stream,pdu->messageId,65);
      fetchX(stream,pdu->noUnsuccess);
      int no = pdu->noUnsuccess;
      if ( no )
      {
        pdu->sme = new UnsuccessDeliveries[no];
        for ( int i=0; i<no; ++i )
        {
          fetchPduAddress(stream,pdu->sme[i].addr);
          fetchX(stream,pdu->sme[i].errorStatusCode);
        }
      }
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case UNBIND:
    case UNBIND_RESP:
    case GENERIC_NACK:
    case ENQUIRE_LINK:
    case ENQUIRE_LINK_RESP:
    case REPLACE_SM_RESP:
    case CANCEL_SM_RESP:
    {
      auto_ptr<PduWithOnlyHeader> pdu(new PduWithOnlyHeader);
      fetchSmppHeader(stream,pdu->header);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case OUTBIND:
    {
      auto_ptr<PduOutBind> pdu(new PduOutBind);
      fetchSmppHeader(stream,pdu->header);
      fetchCOctetStr(stream,pdu->systemId,16);
      fetchCOctetStr(stream,pdu->password,9);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case REPLACE_SM:
    {
      auto_ptr<PduReplaceSm> pdu(new PduReplaceSm);
      fetchSmppHeader(stream,pdu->header);
      fetchCOctetStr(stream,pdu->messageId,65);
      fetchPduAddress(stream,pdu->source);
      fetchCOctetStr(stream,pdu->scheduleDeliveryTime,17);
      fetchCOctetStr(stream,pdu->validityPeriod,17);
      fetchX(stream,pdu->registredDelivery);
      fetchX(stream,pdu->smDefaultMsgId);
      uint8_t smLength = 0;
      fetchX(stream,smLength);
      pdu->shortMessage.dispose();
      if ( smLength )
        fetchOctetStr(stream,pdu->shortMessage,smLength);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case CANCEL_SM:
    {
      auto_ptr<PduCancelSm> pdu(new PduCancelSm);
      fetchSmppHeader(stream,pdu->header);
      fetchCOctetStr(stream,pdu->serviceType,6);
      fetchCOctetStr(stream,pdu->messageId,65);
      fetchPduAddress(stream,pdu->source);
      fetchPduAddress(stream,pdu->dest);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case ALERT_NOTIFICATION:
    {
      auto_ptr<PduAlertNotification> pdu(new PduAlertNotification);
      fetchSmppHeader(stream,pdu->header);
      fetchPduAddress(stream,pdu->source);
      fetchPduAddress(stream,pdu->esme);
      fetchSmppOptional(stream,&pdu->optional);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case DATA_SM:
    {
      auto_ptr<PduDataSm> pdu(new PduDataSm);
      fetchSmppHeader(stream,pdu->header);
      fetchCOctetStr(stream,pdu->data.serviceType,6);
      fetchPduAddress(stream,pdu->data.source);
      fetchPduAddress(stream,pdu->data.dest);
      fetchX(stream,pdu->data.esmClass);
      fetchX(stream,pdu->data.registredDelivery);
      fetchX(stream,pdu->data.dataCoding);
      fetchSmppOptional(stream,&pdu->optional);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case DATA_SM_RESP:
    {
      auto_ptr<PduDataSmResp> pdu(new PduDataSmResp);
      fetchSmppHeader(stream,pdu->header);
      if(pdu->header.get_commandLength()!=16)
      {
        fetchCOctetStr(stream,pdu->messageId,65);
        fetchSmppOptional(stream,&pdu->optional);
      }
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case QUERY_SM:
    {
      auto_ptr<PduQuerySm> pdu(new PduQuerySm);
      fetchSmppHeader(stream,pdu->header);
      fetchCOctetStr(stream,pdu->messageId,65);
      fetchPduAddress(stream,pdu->source);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    case QUERY_SM_RESP:
    {
      auto_ptr<PduQuerySmResp> pdu(new PduQuerySmResp);
      fetchSmppHeader(stream,pdu->header);
      fetchCOctetStr(stream,pdu->messageId,65);
      fetchCOctetStr(stream,pdu->finalDate,17);
      fetchX(stream,pdu->messageState);
      fetchX(stream,pdu->errorCode);
      return reinterpret_cast<SmppHeader*>(pdu.release());
    }
    //default:
    //  goto trap;
    }
    dropPdu(stream);
    SmppHeader *hdr=new SmppHeader;
    hdr->commandId=cmdid;
    __trace2__("fetchSmppPdu: unknown cmdid:%d",cmdid);
    //__unreachable__("");
    return hdr;
  }
  catch (...)
  {
    dropPdu(stream);
    __warning__("parsing broken, reason: exception");
    //throw;
    return 0;
  }
  /* --- unreachable ---
  __check_smpp_stream_invariant__ ( stream );
  return;*/
  trap:
    //dropPdu(stream); packet will droped by stream guard
    __warning__("parsing broken, reason: parse error");
    //throw BadStreamException();
    return 0;
}

}
}

#endif
