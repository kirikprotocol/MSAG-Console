/*
	$id$
*/


#include "util/debug.h"
#include "smpp_memory.h"
#include "smpp_structures.h"
#include "smpp_stream.h"

#if !defined __Cxx_Header__smpp_optional_h__
#define __Cxx_Header__smpp_optional_h__

namespace smsc{
namespace smpp{

inline void fillSmppOptional(SmppStream* stream,SmppOptional* opt)
{
  __check_smpp_stream_invariant__ ( stream );
	__require__(opt != NULL);

#define macroFillField(field) \
	if ( opt->has_##field() ){ \
		__require__ (sizeof(opt->##field)==SmppOptionalLength::##field); \
		fillX(stream,SmppOptionalTags::##field); \
		fillX(stream,SmppOptionalLength::##field); \
		fillX(stream,opt->##field);\
	}

#define macroFillOctetStr(field,maxlen) \
	if ( opt->has_##field()&& opt->get_##field() != NULL){ \
		int str_length = opt->size_##field(); \
		__throw_if_fail__(((str_length<=maxlen)||(maxlen==-1))),VeryLargOctetStringException);\
		fillX(stream,SmppOptionalTags::##field); \
		fillX(stream,str_length); \
		const char* text = opt->get_##field();\
		for ( int k=0; k<str_length; ++k )\
			fillX(stream,text[k]);\
	}

#define macroFillCOctetStr(field,maxlen) \
	if ( opt->has_##field() && opt->get_##field() != NULL) \
		const char* text = opt->get_##field();\
		int str_length = strlen(text);\
		__throw_if_fail__(((str_length<=maxlen)||(maxlen==-1))),VeryLargOctetStringException);\
		fillX(stream,SmppOptionalTags::##field); \
		fillX(stream,str_length+1); \
		for ( int k=0; k<=str_length; ++k )\
			fillX(stream,text[k]);\
	}
  
	/*dest_addr_subunit(5.3.2.1)*/      macroFillField(destAddrSubunit);
  /*dest_network_type(5.3.2.3)*/      macroFillField(destNetworkType);
  /*dest_bearer_type(5.3.2.5)*/       macroFillField(dsetBearerType);
  /*dest_telematics_id(5.3.2.7)*/     macroFillField(destTelematicsId);
  /*source_addr_subunit(5.3.2.2)*/    macroFillField(sourceAddrSubunit);
  /*source_network_type(5.3.2.4)*/    macroFillField(sourceNetworkType);
  /*source_bearer_type(5.3.2.6)*/     macroFillField(sourceBearerType);
  /*source_telematics_id(5.3.2.8)*/   macroFillField(sourceTelematicsId);
  /*qos_time_to_live(5.3.2.9)*/       macroFillField(qosTimeToLive);
  /*payload_type(5.3.2.10)*/          macroFillField(pyloadType);
  /*additional_status_info_text(5.3.2.11)*/ 
	                                    macroFillCOctetStr(additionalStatisInfoText,256);
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
  /*callback_num_pres_ind(5.3.2.37)*/ macroFillField(callbackNumPressInd);
  /*callback_num_atag(5.3.2.38)*/     macroFillOctetStr(callbackNumAtag,65);
	/*number_of_messages(5.3.2.39)*/    macroFillField(numberOfMessages);
	/*callback_num(5.3.2.36)*/          macroFillOctetStr(callbackNum,19);
  /*dpf_result(5.3.2.28)*/            macroFillField(dpfRequit);
  /*set_dpf(5.3.2.29)*/               macroFillField(setDpf);
	/*ms_available_status(5.3.2.30)*/   macroFillField(msAvilableStatus);
  /*network_error_code(5.3.2.31)*/    
	if ( opt->has_networkErrCode() ) 
	{
		fillX(stream,SmppOptionalTags::networkErrCode); // tag
		fillX(stream,3);																// length
		fillX(stream,opt->networkErrCode[0]);
		fillX(stream,opt->networkErrCode[1]);
		fillX(stream,opt->networkErrCode[2]);
	}
  /*message_payload(5.3.2.32)*/       macroFillOctetStr(messagePyload,-1);
	/*delivery_failure_reason(5.3.2.33)*/macroFillField(deliveryFailureReason);
	/*more_messages_to_send(5.3.2.34)*/ macroFillField(moreMessagesToSend);
	/*message_state(5.3.2.35)*/         macroFillField(messageState);
  /*ussd_service_op(5.3.2.44)*/       macroFillField(ussdServiceOp);
  /*display_time(5.3.2.26)*/          macroFillField(displayTime);
  /*sms_signal(5.3.2.40)*/            macroFillField(smsSignal);
  /*ms_validity(5.3.2.27)*/           macroFillField(msVakidity);
  /*alert_on_message_delivery(5.3.2.41)*/
	if ( opt->has_alertOnMessageDelivery() )
	{
		fillX(stream,SmppOptionalTags::alertOnMessageDelivery); // tag
		fillX(stream,0);																        // length
	}
  /*its_reply_type(5.3.2.42)*/        macroFillField(itsReplayType);
	/*its_session_info(5.3.2.43)*/ 
	if ( opt->has_itsSessionInfo() ) 
	{
		fillX(stream,SmppOptionalTags::itsSessionInfo); // tag
		fillX(stream,2);																// length
		fillX(stream,opt->itsSessionInfo[0]);
		fillX(stream,opt->itsSessionInfo[1]);
	}

#undef macroFillField
#undef macroFillOctetStr
#undef macroFillCOctetStr

}

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
    
      switch ( tag )
      {
    #define macroFetchField(field) \
    case SmppOptionalTags::##field :
			fetchX(stream,opt->##field);\
			opt->fields_present |= SmppOptionalFields::##field; \
			break
        
    #define macroFetchCOctetStr(field,maxlen) \
		case SmppOptionalTags::##field :
			fetchCOctetString(stream,opt->##field,maxlen);\
			opt->fields_present |= SmppOptionalFields::##field;	 \
			break
      
    #define macroFetchOctetStr(field,len) \
		case SmppOptionalTags::##field :
			fetchOctetStringStr(stream,opt->##field,len);\
			opt->fields_present |= SmppOptionalFields::##field; \
			break
      
      /*dest_addr_subunit(5.3.2.1)*/     macroFetchField(destAddrSubunit);
      /*dest_network_type(5.3.2.3)*/     macroFetchField(destNetworkType);
      /*dest_bearer_type(5.3.2.5)*/      macroFetchField(dsetBearerType);
      /*dest_telematics_id(5.3.2.7)*/    macroFetchField(destTelematicsId);
      /*source_addr_subunit(5.3.2.2)*/   macroFetchField(sourceAddrSubunit);
			/*source_network_type(5.3.2.4)*/   macroFetchField(sourceNetworkType);
			/*source_bearer_type(5.3.2.6)*/    macroFetchField(sourceBearerType);
			/*source_telematics_id(5.3.2.8)*/  macroFetchField(sourceTelematicsId);
			/*qos_time_to_live(5.3.2.9)*/      macroFetchField(qosTimeToLive);
			/*payload_type(5.3.2.10)*/         macroFetchField(pyloadType);
			/*additional_status_info_text(5.3.2.11)*/ macroFetchCOctetStr(additionalStatisInfoText,256);
			/*receipted_message_id(5.3.2.12)*/ macroFetchCOctetStr(receiptedMessageId,65);
			/*ms_msg_wait_facilities(5.3.2.13)*/macroFetchField(msMsgWaitFacilities);
			/*privacy_indicator(5.3.2.14)*/    macroFetchField(privacyIndicator);
      /*source_subaddress(5.3.2.15)*/           
			case SmppOptionalTags::sourceSubaddress :
        __goto_if_fail__ ( length <= 23 , trap );
				fetchOctetStringStr(stream,opt->sourceSubaddress,length);
				opt->fields_present |= SmppOptionalFields::sourceSubaddress; 
        break;
      /*dest_subaddress(5.3.2.16)*/
			case SmppOptionalTags::destSubaddress :
        __goto_if_fail__ ( length <= 23 , trap );
				fetchOctetStringStr(stream,opt->destSubaddress,length);
				opt->fields_present |= SmppOptionalFields::destSubaddress; 
				break;
      /*user_message_reference(5.3.2.17)*/macroFetchField(userMessageReference);
      /*user_response_code(5.3.2.18)*/    macroFetchField(userResponseCode);
      /*source_port(5.3.2.20)*/           macroFetchField(sourcePort);
			/*destination_port(5.3.2.21)*/      macroFetchField(destinationPort);
      /*sar_msg_ref_num(5.3.2.22)*/       macroFetchField(sarMsgRefNum);
			/*language_indecator(5.3.2.19)*/    macroFetchField(languageIndicator);
			/*sar_total_segments(5.3.2.23)*/    macroFetchField(sarTotalSegments);
			/*sar_segment_seqnum(5.3.2.24)*/    macroFetchField(sarSegmentSegnum);
			/*sc_interface_version(5.3.2.25)*/  macroFetchField(scInterfaceVersion);
			/*callback_num_pres_ind(5.3.2.37)*/ macroFetchField(callbackNumPressInd);
      /*callback_num_atag(5.3.2.38)*/
			case SmppOptionalTags::callbackNumAtag :
        __goto_if_fail__ ( length <= 65 , trap );
				fetchOctetStringStr(stream,opt->callbackNumAtag,length);
				opt->fields_present |= SmppOptionalFields::callbackNumAtag; 
        break;
      /*number_of_messages(5.3.2.39)*/    macroFetchField(numberOfMessages);
      /*callback_num(5.3.2.36)*/   
			case SmppOptionalTags::callbackNum :
        __goto_if_fail__ ( length <= 19 , trap );
				fetchOctetStringStr(stream,opt->callbackNum,length);
				opt->fields_present |= SmppOptionalFields::callbackNum; 
        break;
      /*dpf_result(5.3.2.28)*/            macroFetchField(dpfRequit);
			/*set_dpf(5.3.2.29)*/               macroFetchField(setDpf);
			/*ms_available_status(5.3.2.30)*/   macroFetchField(msAvilableStatus);
			/*network_error_code(5.3.2.31)*/          
			case SmppOptionalTags::networkErrCode :
        __goto_if_fail__ ( length == 3 , trap );
        fetchX(stream,opt->networkErrCode[0]);
        fetchX(stream,opt->networkErrCode[1]);
        fetchX(stream,opt->networkErrCode[2]);
        opt->fields_present |= SmppOptionalFields::networkErrCode;
        break;
      /*message_payload(5.3.2.32)*/       macroFetchOctetStr(messagePyload,length);
      /*delivery_failure_reason(5.3.2.33)*/macroFetchField(deliveryFailureReason);
      /*more_messages_to_send(5.3.2.34)*/ macroFetchField(moreMessagesToSend);
			/*message_state(5.3.2.35)*/         macroFetchField(messageState);
			/*ussd_service_op(5.3.2.44)*/             
        /*  зачитываем и храним как 1 октет ,а не как строку */
        macroFetchField(ussdServiceOp);
      /*display_time(5.3.2.26)*/          macroFetchField(displayTime);
			/*sms_signal(5.3.2.40)*/            macroFetchField(smsSignal);
			/*ms_validity(5.3.2.27)*/           macroFetchField(msVakidity);
      /*alert_on_message_delivery(5.3.2.41)*/  
			case SmppOptionalTags::alertOnMessageDelivery :
        __goto_if_fail__ ( length == 0 , trap );
        opt->set_alertOnMessageDelivery(true);
        break;
      /*its_reply_type(5.3.2.42)*/        macroFetchField(itsReplayType);
			/*its_session_info(5.3.2.43)*/      
			case SmppOptionalTags::itsSessionInfo :
        __goto_if_fail__ ( length == 2 , trap );
        fetchX(stream,opt->itsSessionInfo[0]);
        fetchX(stream,opt->itsSessionInfo[1]);
        opt->fields_present |= SmppOptionalFields::itsSessionInfo;
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
