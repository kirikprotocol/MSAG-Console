/*
  $Id$
*/

#include <algorithm>
#include "util/debug.h"
#include "smpp_memory.h"
#include "smpp_structures.h"
#include "smpp_stream.h"
#include "util/int.h"
#include "util/Exception.hpp"
#include "core/buffers/TmpBuf.hpp"

#if !defined __Cxx_Header__smpp_optional_h__
#define __Cxx_Header__smpp_optional_h__

namespace smsc{
namespace smpp{

class VeryLargeOctetStringException : public smsc::util::Exception
{
public:
    VeryLargeOctetStringException( const char* fmt, ... ) {
        SMSC_UTIL_EX_FILL(fmt);
    }
};

inline void fillSmppOptional(SmppStream* stream,SmppOptional* opt)
{
  __check_smpp_stream_invariant__ ( stream );
  __require__(opt != NULL);

#define macroFillField(field) \
  if ( opt->has_##field() ){ \
    __require__ (sizeof(opt->field)==SmppOptionalLength::field); \
    fillX(stream,SmppOptionalTags::field); \
    fillX(stream,SmppOptionalLength::field); \
    fillX(stream,opt->field);\
  }

#define macroFillOctetStr(field,maxlen) \
  if ( opt->has_##field() && opt->get_##field() != NULL ) {\
      int str_length = opt->size_##field(); \
      if ( ! ((str_length<=maxlen)||(maxlen==-1)) ) { \
          throw VeryLargeOctetStringException("verylargestr: strlen=%u maxlen=%u",unsigned(str_length),unsigned(maxlen)); \
      } \
      fillX(stream,SmppOptionalTags::field); \
      fillX(stream,(uint16_t)str_length); \
      const char* text = opt->get_##field();\
      for ( int k=0; k<str_length; ++k )\
        fillX(stream,(uint8_t)text[k]);\
  }

#define macroFillCOctetStr(field,maxlen) \
  if ( opt->has_##field() && opt->get_##field() != NULL) {\
    const char* text = opt->get_##field();\
    int str_length = (int)strlen(opt->get_##field());\
    if ( ! ((str_length<=maxlen)||(maxlen==-1)) ) { \
        throw VeryLargeOctetStringException("verylargeCstr: strlen=%u maxlen=%u",unsigned(str_length),unsigned(maxlen)); \
    } \
    fillX(stream,SmppOptionalTags::field); \
    fillX(stream,(uint16_t)(str_length+1)); \
    for ( int k=0; k<=str_length; ++k )\
      fillX(stream,(uint8_t)text[k]);\
  }

  /*dest_addr_subunit(5.3.2.1)*/      macroFillField(destAddrSubunit);
  /*dest_network_type(5.3.2.3)*/      macroFillField(destNetworkType);
  /*dest_bearer_type(5.3.2.5)*/       macroFillField(destBearerType);
  /*dest_telematics_id(5.3.2.7)*/     macroFillField(destTelematicsId);
  /*source_addr_subunit(5.3.2.2)*/    macroFillField(sourceAddrSubunit);
  /*source_network_type(5.3.2.4)*/    macroFillField(sourceNetworkType);
  /*source_bearer_type(5.3.2.6)*/     macroFillField(sourceBearerType);
  /*source_telematics_id(5.3.2.8)*/   macroFillField(sourceTelematicsId);
  /*qos_time_to_live(5.3.2.9)*/       macroFillField(qosTimeToLive);
  /*payload_type(5.3.2.10)*/          macroFillField(payloadType);
  /*additional_status_info_text(5.3.2.11)*/
                                      macroFillCOctetStr(additionalStatusInfoText,256);
  /*receipted_message_id(5.3.2.12)*/  macroFillCOctetStr(receiptedMessageId,65);
  /*ms_msg_wait_facilities(5.3.2.13)*/macroFillField(msMsgWaitFacilities);
  /*privacy_indicator(5.3.2.14)*/     macroFillField(privacyIndicator);
  /*source_subaddress(5.3.2.15)*/     macroFillOctetStr(sourceSubaddress,23);
  /*dest_subaddress(5.3.2.16)*/       macroFillOctetStr(destSubaddress,23);
  /*user_message_reference(5.3.2.17)*/macroFillField(userMessageReference);
  /*user_response_code(5.3.2.18)*/    macroFillField(userResponseCode);
  /*source_port(5.3.2.20)*/           macroFillField(sourcePort);
  /*destination_port(5.3.2.21)*/      macroFillField(destinationPort);
  /*sar_msg_ref_num(5.3.2.22)*/       macroFillField(sarMsgRefNum);
  /*language_indecator(5.3.2.19)*/    macroFillField(languageIndicator);
  /*sar_total_segments(5.3.2.23)*/    macroFillField(sarTotalSegments);
  /*sar_segment_seqnum(5.3.2.24)*/    macroFillField(sarSegmentSegnum);
  /*sc_interface_version(5.3.2.25)*/  macroFillField(scInterfaceVersion);
  /*callback_num_pres_ind(5.3.2.37)*/ macroFillField(callbackNumPresInd);
  /*callback_num_atag(5.3.2.38)*/     macroFillOctetStr(callbackNumAtag,65);
  /*number_of_messages(5.3.2.39)*/    macroFillField(numberOfMessages);
  /*callback_num(5.3.2.36)*/          macroFillOctetStr(callbackNum,19);
  /*dpf_result(5.3.2.28)*/            macroFillField(dpfResult);
  /*set_dpf(5.3.2.29)*/               macroFillField(setDpf);
  /*ms_available_status(5.3.2.30)*/   macroFillField(msAvailableStatus);
  /*network_error_code(5.3.2.31)*/
  if ( opt->has_networkErrorCode() )
  {
    fillX(stream,SmppOptionalTags::networkErrorCode);  // tag
    fillX(stream,SmppOptionalLength::networkErrorCode);// length
    fillX(stream,opt->networkErrorCode[0]);
    fillX(stream,opt->networkErrorCode[1]);
    fillX(stream,opt->networkErrorCode[2]);
  }
  /*message_payload(5.3.2.32)*/       macroFillOctetStr(messagePayload,-1);
  /*delivery_failure_reason(5.3.2.33)*/macroFillField(deliveryFailureReason);
  /*more_messages_to_send(5.3.2.34)*/ macroFillField(moreMessagesToSend);
  /*message_state(5.3.2.35)*/         macroFillField(messageState);
  /*ussd_service_op(5.3.2.44)*/       macroFillField(ussdServiceOp);
  /*display_time(5.3.2.26)*/          macroFillField(displayTime);
  /*sms_signal(5.3.2.40)*/            macroFillField(smsSignal);
  /*ms_validity(5.3.2.27)*/           macroFillField(msValidity);
  /*alert_on_message_delivery(5.3.2.41)*/
  if ( opt->has_alertOnMessageDelivery() )
  {
    fillX(stream,SmppOptionalTags::alertOnMessageDelivery);  // tag
    fillX(stream,SmppOptionalLength::alertOnMessageDelivery);// length
  }
  /*its_reply_type(5.3.2.42)*/        macroFillField(itsReplyType);
  /*its_session_info(5.3.2.43)*/
  if ( opt->has_itsSessionInfo() )
  {
    fillX(stream,SmppOptionalTags::itsSessionInfo);  // tag
    fillX(stream,SmppOptionalLength::itsSessionInfo);// length
    fillX(stream,opt->itsSessionInfo[0]);
    fillX(stream,opt->itsSessionInfo[1]);
  }

  /* SMSC specific */
  macroFillCOctetStr( supported_locale, -1 );
  macroFillCOctetStr( imsi_address, -1 );
  macroFillCOctetStr( msc_address, -1 );
  macroFillField( supported_codeset );
  macroFillCOctetStr( sccp_oa, -1 );
  macroFillCOctetStr( sccp_da, -1 );

  /* smpp+ specific*/
  macroFillField(ussd_session_id);
  macroFillCOctetStr(imsi,-1);
  macroFillField(hlr_address_ton);
  macroFillField(hlr_address_npi);
  macroFillCOctetStr(hlr_address,-1);
  macroFillField(vlr_number_ton);
  macroFillField(vlr_number_npi);
  macroFillCOctetStr(vlr_number,-1);


  if(opt->has_unknownFields())
  {
    const char* val=opt->get_unknownFields();
    int len=opt->size_unknownFields();
    for ( int k=0; k<len; ++k )fillX(stream,(uint8_t)val[k]);
  }

  /*protocol_id*/        macroFillField(protocol_id);

#undef macroFillField
#undef macroFillOctetStr
#undef macroFillCOctetStr

}

/**
  ������� ������������ ����
*/
inline void fetchSmppOptional(SmppStream* stream,SmppOptional* opt)
{
  __check_smpp_stream_invariant__ ( stream );
  try
  {
    while ( stream->dataOffset+4 < stream->dataLength )
    {
      uint16_t tag;
      uint16_t length;
      uint32_t nextDataOffset;
      //uint32_t __pos = stream->dataOffset;
      fetchX(stream,tag);
      fetchX(stream,length);
      //__trace2__("T:%hx,L:%hd STR->OFFS:%d,STR->LEN:%d",
      //           tag,length,__pos,stream->dataLength);
      //__require__ ( length >= 0 ); // not need because length is unsigned
      nextDataOffset = stream->dataOffset+length;

      switch ( tag )
      {
#define macroFetchField(field) \
    case SmppOptionalTags::field :{\
      if(length!=SmppOptionalLength::field)\
      {\
        if(tag>=0x1400 && tag<=0x3fff) \
          goto unknown; \
        else \
          throw BadStreamException("fetch optional %s: invalid size expected=%u found=%u", \
            #field, unsigned(SmppOptionalLength::field),unsigned(length));\
      }\
      fetchX(stream,opt->field);\
      opt->field_present |= SmppOptionalFields::field; \
      }break

    #define macroFetchCOctetStr(field,maxlen) \
    case SmppOptionalTags::field :{\
      fetchCOctetStr(stream,opt->field,std::min((uint16_t)maxlen,(uint16_t)length));\
      opt->field_present |= SmppOptionalFields::field;   \
      }break

    #define macroFetchOctetStr(field,len) \
    case SmppOptionalTags::field :{\
      fetchOctetStr(stream,opt->field,length);\
      opt->field_present |= SmppOptionalFields::field; \
      }break

      /*dest_addr_subunit(5.3.2.1)*/     macroFetchField(destAddrSubunit);
      /*dest_network_type(5.3.2.3)*/     macroFetchField(destNetworkType);
      /*dest_bearer_type(5.3.2.5)*/      macroFetchField(destBearerType);
      /*dest_telematics_id(5.3.2.7)*/    macroFetchField(destTelematicsId);
      /*source_addr_subunit(5.3.2.2)*/   macroFetchField(sourceAddrSubunit);
      /*source_network_type(5.3.2.4)*/   macroFetchField(sourceNetworkType);
      /*source_bearer_type(5.3.2.6)*/    macroFetchField(sourceBearerType);
      /*source_telematics_id(5.3.2.8)*/  macroFetchField(sourceTelematicsId);
      /*qos_time_to_live(5.3.2.9)*/      macroFetchField(qosTimeToLive);
      /*payload_type(5.3.2.10)*/         macroFetchField(payloadType);
      /*additional_status_info_text(5.3.2.11)*/
                                         macroFetchCOctetStr(additionalStatusInfoText,256);
      /*receipted_message_id(5.3.2.12)*/ macroFetchCOctetStr(receiptedMessageId,65);
      /*ms_msg_wait_facilities(5.3.2.13)*/macroFetchField(msMsgWaitFacilities);
      /*privacy_indicator(5.3.2.14)*/    macroFetchField(privacyIndicator);
      /*source_subaddress(5.3.2.15)*/
      case SmppOptionalTags::sourceSubaddress :{
        __goto_if_fail__ ( length <= 23 , trap );
        fetchOctetStr(stream,opt->sourceSubaddress,length);
        opt->field_present |= SmppOptionalFields::sourceSubaddress;
        }break;
      /*dest_subaddress(5.3.2.16)*/
      case SmppOptionalTags::destSubaddress :{
        __goto_if_fail__ ( length <= 23 , trap );
        fetchOctetStr(stream,opt->destSubaddress,length);
        opt->field_present |= SmppOptionalFields::destSubaddress;
        }break;
      /*user_message_reference(5.3.2.17)*/macroFetchField(userMessageReference);
      /*user_response_code(5.3.2.18)*/    macroFetchField(userResponseCode);
      /*source_port(5.3.2.20)*/           macroFetchField(sourcePort);
      /*destination_port(5.3.2.21)*/      macroFetchField(destinationPort);
      /*sar_msg_ref_num(5.3.2.22)*/       macroFetchField(sarMsgRefNum);
      /*language_indecator(5.3.2.19)*/    macroFetchField(languageIndicator);
      /*sar_total_segments(5.3.2.23)*/    macroFetchField(sarTotalSegments);
      /*sar_segment_seqnum(5.3.2.24)*/    macroFetchField(sarSegmentSegnum);
      /*sc_interface_version(5.3.2.25)*/  macroFetchField(scInterfaceVersion);
      /*callback_num_pres_ind(5.3.2.37)*/ macroFetchField(callbackNumPresInd);
      /*callback_num_atag(5.3.2.38)*/
      case SmppOptionalTags::callbackNumAtag :{
        __goto_if_fail__ ( length <= 65 , trap );
        fetchOctetStr(stream,opt->callbackNumAtag,length);
        opt->field_present |= SmppOptionalFields::callbackNumAtag;
        }break;
      /*number_of_messages(5.3.2.39)*/    macroFetchField(numberOfMessages);
      /*callback_num(5.3.2.36)*/
      case SmppOptionalTags::callbackNum :{
        __goto_if_fail__ ( length <= 19 , trap );
        fetchOctetStr(stream,opt->callbackNum,length);
        opt->field_present |= SmppOptionalFields::callbackNum;
        }break;
      /*dpf_result(5.3.2.28)*/            macroFetchField(dpfResult);
      /*set_dpf(5.3.2.29)*/               macroFetchField(setDpf);
      /*ms_available_status(5.3.2.30)*/   macroFetchField(msAvailableStatus);
      /*network_error_code(5.3.2.31)*/
      case SmppOptionalTags::networkErrorCode :{
        __goto_if_fail__ ( length == 3 , trap );
        fetchX(stream,opt->networkErrorCode[0]);
        fetchX(stream,opt->networkErrorCode[1]);
        fetchX(stream,opt->networkErrorCode[2]);
        opt->field_present |= SmppOptionalFields::networkErrorCode;
        }break;
      /*message_payload(5.3.2.32)*/       macroFetchOctetStr(messagePayload,length);
      /*delivery_failure_reason(5.3.2.33)*/macroFetchField(deliveryFailureReason);
      /*more_messages_to_send(5.3.2.34)*/ macroFetchField(moreMessagesToSend);
      /*message_state(5.3.2.35)*/         macroFetchField(messageState);
      /*ussd_service_op(5.3.2.44)*/
      /*  ���������� � ������ ��� 1 ����� ,� �� ��� ������ */
                                          macroFetchField(ussdServiceOp);
      /*display_time(5.3.2.26)*/          macroFetchField(displayTime);
      /*sms_signal(5.3.2.40)*/            macroFetchField(smsSignal);
      /*ms_validity(5.3.2.27)*/           macroFetchField(msValidity);
      /*alert_on_message_delivery(5.3.2.41)*/
      case SmppOptionalTags::alertOnMessageDelivery :{
        __goto_if_fail__ ( length == 0 , trap );
        opt->set_alertOnMessageDelivery(true);
        opt->field_present |= SmppOptionalFields::alertOnMessageDelivery;
        }break;
      /*its_reply_type(5.3.2.42)*/        macroFetchField(itsReplyType);
      /*its_session_info(5.3.2.43)*/
      case SmppOptionalTags::itsSessionInfo :
        __goto_if_fail__ ( length == 2 , trap );
        fetchX(stream,opt->itsSessionInfo[0]);
        fetchX(stream,opt->itsSessionInfo[1]);
        opt->field_present |= SmppOptionalFields::itsSessionInfo;
        break;
      /* SMSC specific */
      macroFetchCOctetStr( supported_locale, length );
      macroFetchCOctetStr( imsi_address, length );
      macroFetchCOctetStr( msc_address, length );
      macroFetchField( supported_codeset );
      /*protocol_id*/        macroFetchField(protocol_id);
      macroFetchCOctetStr( sccp_oa, length );
      macroFetchCOctetStr( sccp_da, length );

      /* smpp+ specific*/
      macroFetchField(ussd_session_id);
      macroFetchCOctetStr( imsi, length );
      macroFetchField(hlr_address_ton);
      macroFetchField(hlr_address_npi);
      macroFetchCOctetStr( hlr_address, length );
      macroFetchField(vlr_number_ton);
      macroFetchField(vlr_number_npi);
      macroFetchCOctetStr( vlr_number, length );


      default:
        {
          unknown:
          opt->field_present |= SmppOptionalFields::unknownFields;
#ifndef DISABLE_TRACING
          smsc::logger::Logger* log=smsc::logger::Logger::getInstance("smpp");
#endif
          debug2(log,"Unknown optional tag:%x(%d), length=%d",tag,tag,length);
          smsc::core::buffers::TmpBuf<uint8_t,128> buf(length);
          uint16_t tmp=htons(tag);
          buf.Append((uint8_t*)&tmp,2);
          tmp=htons(length);
          buf.Append((uint8_t*)&tmp,2);
          uint8_t x;
          for(int i=0;i<length;i++)
          {
            fetchX(stream,x);
            buf.Append(&x,1);
          }
          debug2(log,"added %d bytes to unknownFields",buf.GetPos());
          opt->unknownFields.append((char*)buf.get(),(int)buf.GetPos());
          break;
        }
      }
      if ( ! ( nextDataOffset == stream->dataOffset ) ) {
          throw BadStreamException("fetch optional: nextData=%u dataoffset=%u",
                                   unsigned(nextDataOffset), unsigned(stream->dataOffset));
      }
      /*
      if ( nextDataOffset > stream->dataOffset )
      {
        __warning2__("length (%d) of field with tag %x is great field type size, fixed!\n",
                (unsigned int)length,
                (unsigned int)tag);
        while( stream->dataOffset<nextDataOffset) {int8_t octet; fetchX(stream,octet);};
      }
      */

    #undef macroFetchField
    #undef macroFetchOctetStr
    #undef macroFetchCOctetStr
    #undef SMPP_HAS_FIELD
    }
  }
  catch (...)
  {
    dropPdu(stream);
    __warning__("packet is dropped, reason: exception");
    throw;
  }
   __check_smpp_stream_invariant__ ( stream );
  return;
  trap:
    dropPdu(stream);
    __warning__("packet is dropped, reason: parse error");
  throw BadStreamException("packet is dropped: parse error");
}

}
}

#endif /* __Cxx_Header__smpp_optional_h__ */
