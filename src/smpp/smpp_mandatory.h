/*
  $Id$
*/

///
/// Этот файл содержит код вычитывающий пакет из потока и 
/// код записывающий пакет в поток
///

#if !defined __Cxx_Header__smpp_mandatory_h__
#define __Cxx_header__smpp_mandatory_h__

#include <memory>
#include "util/debug.h"
#include "smpp_structures.h"
#include "smpp_optional.h"
#include "smpp_stream.h"

namespace smsc{
namespace smpp{

using std::auto_ptr;

class BadDataException {};

inline void fetchPduAddress(SmppStream* stream,PduAddress& addr)
{
  fetchX(stream,addr.typeOfNumber);
  fetchX(stream,addr.numberingPlan);
  fetchCOctetStr(stream,addr.value,21);
}

inline void fillPduAddress(SmppStream* stream,PduAddress& addr)
{
  fillX(stream,addr.typeOfNumber);
  fillX(stream,addr.numberingPlan);
  fillCOctetStr(stream,addr.value);
}

inline bool fillSmppPdu(SmppStream* stream,SmppHeader* _pdu)
{
  using namespace SmppCommandSet;
  int cmdid = _pdu->commandId;
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
        if ( sm.numberOfDests )
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
      fillCOctetStr(stream,pdu->messageId);
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
    case REPLACE_SM:
    case REPLACE_SM_RESP:
    case CANCEL_SM:
    case CANCEL_SM_RESP:
    case ENQUIRE_LINK:
    case ENQUIRE_LINK_RESP:
    case ALERT_NOTIFICATION:
    case DATA_SM:
    case DATA_SM_RESP:
    case QUERY_SM:
    case QUERY_SM_RESP:
      break;
    }
    __warning__("bad smpp pdu");
    return false;
  }
  catch(...)
  {
    __warning__ ("fill pdu error");
    throw;
  }
}

/**
  Достаем обязательные поля
*/
inline SmppHeader* fetchSmppPdu(SmppStream* stream)
{
  using namespace SmppCommandSet;
  class StreamGuard
  { 
    SmppStream* stream;
  public:
    StreamGuard(SmppStream* stream):stream(stream){}
    ~StreamGuard()
		{
			if (stream->dataOffset < stream->dataLength) 
			{
				__warning__("packet has left data, dropped");
				dropPdu(stream);
			}
		}
  };
  __check_smpp_stream_invariant__ ( stream );
  StreamGuard guard(stream);
  try
  {
    int32_t cmdid = smppCommandId(stream);
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
      fetchPduAddress(stream,pdu->addressRange);
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
        if ( sm.numberOfDests )
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
      fetchCOctetStr(stream,pdu->messageId,65);
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
    case REPLACE_SM_RESP:
    case CANCEL_SM:
    case CANCEL_SM_RESP:
    case ENQUIRE_LINK:
    case ENQUIRE_LINK_RESP:
    case ALERT_NOTIFICATION:
    case DATA_SM:
    case DATA_SM_RESP:
    case QUERY_SM:
    case QUERY_SM_RESP:
      goto trap;
    }
    __watch__(cmdid);
    __unreachable__("");
  }
  catch (...)
  {
    dropPdu(stream);
    __warning__("parsing broken, reason: exception");
    throw;
  }
  /* --- unreachable ---
  __check_smpp_stream_invariant__ ( stream );
  return;*/
  trap:
    //dropPdu(stream); packet will droped by stream guard
    __warning__("parsing broken, reason: parse error");
    throw BadStreamException();
}

};
};

#endif
