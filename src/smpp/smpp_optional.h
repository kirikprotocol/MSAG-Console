#include "util/debug.h"
#include "smpp_memory.h"
#include "smpp_structures.h"
#include "smpp_stream.h"

#if !defined __Cxx_Header__smpp_optional_h__
#define __Cxx_Header__smpp_optional_h__

namespace smsc{
namespace smpp{

/**
  ƒостаем опциональные пол€
*/
inline void fetchSmppOptional(SmppStream* stream,SmppOptional* opt)
{
  __check_smpp_stream_invariant__ ( stream );
  try
  {
    while ( stream->dataOffset<stream->dataLength )
    {
      uint16_t tag;
      uint16_t length;
      int nextDataOffset;
      fetchX(stream,tag);
      fetchX(stream,length);
      
      //__require__ ( length >= 0 ); // not need because length is unsigned
      nextDataOffset = stream->dataOffset+length;
    
    #define SMPP_HAS_FIELD(x,y)
      
      switch ( tag )
      {
    #define macroFetchField(field) \
    fetchX(stream,opt->##field);\
    SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_##field);
        
    #define macroFetchCOctetStr(field,maxlen) \
    fetchCOctetString(stream,opt->##field,maxlen);\
    SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_##field);
      
    #define macroFetchOctetStr(field,len) \
    fetchOctetStringStr(stream,opt->##field,len);\
    SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_##field);
      
      case /*dest_addr_subunit(5.3.2.1)*/            0x05:
        macroFetchField(destAddrSubunit);
        break;
      case /*dest_network_type(5.3.2.3)*/            0x06:
        macroFetchField(destNetworkType);
        break;
      case /*dest_bearer_type(5.3.2.5)*/             0x07:
        macroFetchField(dsetBearerType);
        break;
      case /*dest_telematics_id(5.3.2.7)*/           0x08:
        macroFetchField(destTelematicsId);
        break;
      case /*source_addr_subunit(5.3.2.2)*/          0x0d:
        macroFetchField(sourceAddrSubunit);
        break;
      case /*source_network_type(5.3.2.4)*/          0x0e:
        macroFetchField(sourceNetworkType);
        break;
      case /*source_bearer_type(5.3.2.6)*/           0x0f:
        macroFetchField(sourceBearerType);
        break;
      case /*source_telematics_id(5.3.2.8)*/         0x010:
        macroFetchField(sourceTelematicsId);
        break;
      case /*qos_time_to_live(5.3.2.9)*/             0x017:
        macroFetchField(qosTimeToLive);
        break;
      case /*payload_type(5.3.2.10)*/                0x019:
        macroFetchField(pyloadType);
        break;
      case /*additional_status_info_text(5.3.2.11)*/ 0x01d:
        macroFetchCOctetStr(additionalStatisInfoText,256);
        break;
      case /*receipted_message_id(5.3.2.12)*/        0x01:
        macroFetchCOctetStr(receiptedMessageId,65);
        break;
      case /*ms_msg_wait_facilities(5.3.2.13)*/      0x030:
        macroFetchField(msMsgWaitFacilities);
        break;
      case /*privacy_indicator(5.3.2.14)*/           0x0201:
        macroFetchField(privacyIndicator);
        break;
      case /*source_subaddress(5.3.2.15)*/           0x0202:
        __goto_if_fail__ ( length <= 23 , trap );
        macroFetchOctetStr(sourceSubaddress,length);
        break;
      case /*dest_subaddress(5.3.2.16)*/             0x0203:
        __goto_if_fail__ ( length <= 23 , trap );
        macroFetchOctetStr(destSubaddress,length);
        break;
      case /*user_message_reference(5.3.2.17)*/      0x0204:
        macroFetchField(userMessageReference);
        break;
      case /*user_response_code(5.3.2.18)*/          0x0205:
        macroFetchField(userResponseCode);
        break;
      case /*source_port(5.3.2.20)*/                 0x020a:
        macroFetchField(sourcePort);
        break;
      case /*destination_port(5.3.2.21)*/            0x020b:
        macroFetchField(destinationPort);
        break;
      case /*sar_msg_ref_num(5.3.2.22)*/             0x020c:
        macroFetchField(sarMsgRefNum);
        break;
      case /*language_indecator(5.3.2.19)*/          0x020d:
        macroFetchField(languageIndicator);
        break;
      case /*sar_total_segments(5.3.2.23)*/          0x020e:
        macroFetchField(sarTotalSegments);
        break;
      case /*sar_segment_seqnum(5.3.2.24)*/          0x020f:
        macroFetchField(sarSegmentSegnum);
        break;
      case /*sc_interface_version(5.3.2.25)*/        0x0210:
        macroFetchField(scInterfaceVersion);
        break;
      case /*callback_num_pres_ind(5.3.2.37)*/       0x0302:
        macroFetchField(callback_num_press_ind);
        break;
      case /*callback_num_atag(5.3.2.38)*/           0x0303:
        __goto_if_fail__ ( length <= 65 , trap );
        macroFetchOctetStr(callbackNumAtag,length);
        break;
      case /*number_of_messages(5.3.2.39)*/          0x0304:
        macroFetchField(numberOfMessages);
        break;
      case /*callback_num(5.3.2.36)*/                0x0381:
        __goto_if_fail__ ( length <= 19 , trap );
        macroFetchOctetStr(callbackNum,length);
        break;
      case /*dpf_result(5.3.2.28)*/                  0x0420:
        macroFetchField(dpfRequit);
        break;
      case /*set_dpf(5.3.2.29)*/                     0x0421:
        macroFetchField(setDpf);
        break;
      case /*ms_available_status(5.3.2.30)*/         0x0422:
        macroFetchField(msAvilableStatus);
        break;
      case /*network_error_code(5.3.2.31)*/          0x0423:
        __goto_if_fail__ ( length == 3 , trap );
        fetchX(stream,opt->networkErrCode[0]);
        fetchX(stream,opt->networkErrCode[1]);
        fetchX(stream,opt->networkErrCode[2]);
        SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_networkErrCode);
        break;
      case /*message_payload(5.3.2.32)*/             0x0424:
        macroFetchOctetStr(messagePyload,length);
        break;
      case /*delivery_failure_reason(5.3.2.33)*/     0x0425:
        macroFetchField(deliveryFailureReason);
        break;
      case /*more_messages_to_send(5.3.2.34)*/       0x0426:
        macroFetchField(moreMessagesToSend);
        break;
      case /*message_state(5.3.2.35)*/               0x0427:
        macroFetchField(messageState);
        break;
      case /*ussd_service_op(5.3.2.44)*/             0x0501:
        /*  зачитываем и храним как 1 октет ,а не как строку */
        macroFetchField(ussdServiceOp);
        break;
      case /*display_time(5.3.2.26)*/                0x1201:
        macroFetchField(displayTime);
        break;
      case /*sms_signal(5.3.2.40)*/                  0x1203:
        macroFetchField(smsSignal);
        break;
      case /*ms_validity(5.3.2.27)*/                 0x1204:
        macroFetchField(msVakidity);
        break;
      case /*alert_on_message_delivery(5.3.2.41)*/   0x130c:
        alertOnMessageDelivery = true;
        SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_alertOnMessageDelivery);
        break;
      case /*its_reply_type(5.3.2.42)*/              0x1380:
        macroFetchField(itsReplayType);
        break;
      case /*its_session_info(5.3.2.43)*/            0x1383:
        fetchX(stream,opt->itsSessionInfo[0]);
        fetchX(stream,opt->itsSessionInfo[1]);
        SMPP_HAS_FIELD(opt,SMPP_HAS_FIELD_itsSessionInfo);
        break;
      }
      __require__ ( nextDataOffset >= stream->dataOffset );
      if ( nextDataOffset > stream->dataOffset )
      {
        warning("length (%d) of field with tag %x is great field type size, fixed!\n",
                (unsigned int)length,
                (unsigned int)tag);
        while( stream->dataOffset<nextDataOffset) {int8_t octet, fetchX(stream,octet);};
      }
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
    throw BadStreamException();
}

};
};

#endif /* __Cxx_Header__smpp_optional_h__ */
