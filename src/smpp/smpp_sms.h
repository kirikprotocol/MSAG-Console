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
#include <string>
#include "smeman/smetypes.h"

namespace smsc{
namespace smpp{

//using smsc::sms::SMS;
//using smsc::sms::Address;
using std::string;
using namespace smsc::sms;

inline Address PduAddress2Address(const PduAddress& source)
{
  return Address(source.value.cstr()?(uint8_t)strlen(source.value.cstr()):1,
                 source.typeOfNumber,
                 source.numberingPlan,
                 source.value.cstr()?source.value.cstr():"0");
}

inline PduAddress Address2PduAddress(const Address& addr)
{
  PduAddress src;
  char val[21];
  int val_length = addr.getValue(val);
  __require__ ( val_length <= (signed int)sizeof(val) );   // fatal if out of range !!!!
  src.set_value(val);
  src.set_typeOfNumber(addr.getTypeOfNumber());
  src.set_numberingPlan(addr.getNumberingPlan());
  return src;
}

inline void fillOptional(SmppOptional& optional,SMS* sms,uint32_t smeFlags=0)
{
  using namespace smsc::smeman;
  if ( sms->hasStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID) )
  {
    optional.set_receiptedMessageId(sms->getStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID).c_str());
  }
  if ( sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP) )
    optional.set_ussdServiceOp(sms->getIntProperty(Tag::SMPP_USSD_SERVICE_OP));
  if ( sms->hasIntProperty(Tag::SMPP_PAYLOAD_TYPE) )
    optional.set_payloadType(sms->getIntProperty(Tag::SMPP_PAYLOAD_TYPE));
  if ( sms->hasIntProperty(Tag::SMPP_USER_RESPONSE_CODE) )
    optional.set_userResponseCode(sms->getIntProperty(Tag::SMPP_USER_RESPONSE_CODE));
  if ( sms->hasIntProperty(Tag::SMPP_SAR_MSG_REF_NUM) )
    optional.set_sarMsgRefNum(sms->getIntProperty(Tag::SMPP_SAR_MSG_REF_NUM));
  if ( sms->hasIntProperty(Tag::SMPP_LANGUAGE_INDICATOR) )
    optional.set_languageIndicator(sms->getIntProperty(Tag::SMPP_LANGUAGE_INDICATOR));
  if ( sms->hasIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS) )
    optional.set_sarTotalSegments(sms->getIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS));
  if ( sms->hasIntProperty(Tag::SMPP_NUMBER_OF_MESSAGES) )
    optional.set_numberOfMessages(sms->getIntProperty(Tag::SMPP_NUMBER_OF_MESSAGES));

  if(sms->hasIntProperty(Tag::SMPP_SOURCE_PORT))
    optional.set_sourcePort(sms->getIntProperty(Tag::SMPP_SOURCE_PORT));
  if(sms->hasIntProperty(Tag::SMPP_DESTINATION_PORT))
    optional.set_destinationPort(sms->getIntProperty(Tag::SMPP_DESTINATION_PORT));
  if(sms->hasIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM))
    optional.set_sarSegmentSegnum(sms->getIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM));
  if(sms->hasIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND))
    optional.set_moreMessagesToSend(sms->getIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND));
  if(sms->hasIntProperty(Tag::SMPP_DEST_NETWORK_TYPE))
    optional.set_destNetworkType(sms->getIntProperty(Tag::SMPP_DEST_NETWORK_TYPE));
  if(sms->hasIntProperty(Tag::SMPP_DEST_BEARER_TYPE))
    optional.set_destBearerType(sms->getIntProperty(Tag::SMPP_DEST_BEARER_TYPE));
  if(sms->hasIntProperty(Tag::SMPP_QOS_TIME_TO_LIVE))
    optional.set_qosTimeToLive(sms->getIntProperty(Tag::SMPP_QOS_TIME_TO_LIVE));
  if(sms->hasIntProperty(Tag::SMPP_SET_DPF))
    optional.set_setDpf(sms->getIntProperty(Tag::SMPP_SET_DPF));
  if(sms->hasIntProperty(Tag::SMPP_SOURCE_NETWORK_TYPE))
    optional.set_sourceNetworkType(sms->getIntProperty(Tag::SMPP_SOURCE_NETWORK_TYPE));
  if(sms->hasIntProperty(Tag::SMPP_SOURCE_BEARER_TYPE))
    optional.set_sourceBearerType(sms->getIntProperty(Tag::SMPP_SOURCE_BEARER_TYPE));

  if(sms->hasIntProperty(Tag::SMPP_PRIVACYINDICATOR))
  {
    optional.set_privacyIndicator(sms->getIntProperty(Tag::SMPP_PRIVACYINDICATOR));
  }

  if(sms->hasIntProperty(Tag::SMPP_NETWORK_ERROR_CODE))
  {
    uint32_t nec = htonl(sms->getIntProperty(Tag::SMPP_NETWORK_ERROR_CODE));
    optional.set_networkErrorCode((uint8_t*)&nec + 1);
  }

  if ( sms->hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) )
  {
    unsigned len;
    const char * data = sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
    if(!HSNS_isEqual() && sms->getIntProperty(Tag::SMPP_DATA_CODING)==8)
    {
      optional.set_messagePayload("",0);
      UCS_htons(optional.messagePayload.alloc(len), data, len, sms->getIntProperty(Tag::SMPP_ESM_CLASS));
    }else
    {
      optional.set_messagePayload(data,len);
    }
  }
  if(!(smeFlags&sfForceGsmDatacoding))
  {
    if( sms->hasIntProperty(Tag::SMPP_MS_VALIDITY) )
      optional.set_msValidity(sms->getIntProperty(Tag::SMPP_MS_VALIDITY));
    if ( sms->hasIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES) )
      optional.set_msMsgWaitFacilities(sms->getIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES));
    if ( sms->hasIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT) )
      optional.set_destAddrSubunit(sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT));
  }


  if ( sms->hasIntProperty(Tag::SMPP_MSG_STATE) )
  {
    /*
    switch(sms->getIntProperty(Tag::SMPP_MSG_STATE))
    {
      case ENROUTE: optional.set_messageState(1);break;
      case DELIVERED: optional.set_messageState(2);break;
      case EXPIRED:optional.set_messageState(3);break;
      case DELETED:optional.set_messageState(4);break;
      case UNDELIVERABLE:optional.set_messageState(5);break;
    };
    */
    optional.set_messageState(sms->getIntProperty(Tag::SMPP_MSG_STATE));
  }

  if ( sms->hasStrProperty(Tag::SMSC_SUPPORTED_LOCALE) )
    optional.set_supported_locale(sms->getStrProperty(Tag::SMSC_SUPPORTED_LOCALE).c_str());
  if ( sms->hasIntProperty(Tag::SMSC_SUPPORTED_CODESET) )
    optional.set_supported_codeset( sms->getIntProperty(Tag::SMSC_SUPPORTED_CODESET) );
  if( sms->hasStrProperty(Tag::SMSC_SCCP_DA)  && (smeFlags&sfCarrySccpInfo))
  {
    optional.set_sccp_da(sms->getStrProperty(Tag::SMSC_SCCP_DA).c_str());
  }

  if(smeFlags&sfSmppPlus && sms->hasIntProperty(Tag::SMPP_USSD_SERVICE_OP))
  {
    optional.set_ussd_session_id(sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
    if(sms->hasStrProperty(Tag::SMSC_IMSI_ADDRESS)  && (smeFlags&sfCarryOrgDescriptor))
    {
      optional.set_imsi(sms->getStrProperty(Tag::SMSC_IMSI_ADDRESS).c_str());
    }
    if(sms->hasStrProperty(Tag::SMSC_MSC_ADDRESS)  && (smeFlags&sfCarryOrgDescriptor))
    {
      optional.set_vlr_number_ton(1);
      optional.set_vlr_number_npi(1);
      optional.set_vlr_number(sms->getStrProperty(Tag::SMSC_MSC_ADDRESS).c_str());
    }
    if(sms->hasStrProperty(Tag::SMSC_SCCP_OA) && (smeFlags&sfCarrySccpInfo))
    {
      optional.set_hlr_address_ton(1);
      optional.set_hlr_address_npi(1);
      optional.set_hlr_address(sms->getStrProperty(Tag::SMSC_SCCP_OA).c_str());
    }
  } else {
    if ( sms->hasIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE) )
    {
      optional.set_userMessageReference(sms->getIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE));
    }
    if ( sms->hasStrProperty(Tag::SMSC_IMSI_ADDRESS) && (smeFlags&sfCarryOrgDescriptor))
      optional.set_imsi_address(sms->getStrProperty(Tag::SMSC_IMSI_ADDRESS).c_str());

    if ( sms->hasStrProperty(Tag::SMSC_MSC_ADDRESS)  && (smeFlags&sfCarryOrgDescriptor))
      optional.set_msc_address(sms->getStrProperty(Tag::SMSC_MSC_ADDRESS).c_str());

    if( sms->hasStrProperty(Tag::SMSC_SCCP_OA) && (smeFlags&sfCarrySccpInfo))
    {
      optional.set_sccp_oa(sms->getStrProperty(Tag::SMSC_SCCP_OA).c_str());
    }

  }

  if(sms->hasIntProperty(Tag::SMPP_ITS_SESSION_INFO))
  {
    uint16_t val=sms->getIntProperty(Tag::SMPP_ITS_SESSION_INFO);
    uint8_t arr[2];
    arr[0]=val&0xff;
    arr[1]=(val&0xff00u)>>8;
    optional.set_itsSessionInfo(arr);
  }

  if(sms->hasBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS))
  {
    unsigned len;
    const char* opt=sms->getBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS,&len);
    optional.set_unknownFields(opt,len);
  }

  if(sms->hasBinProperty(Tag::SMPP_CALLBACK_NUM))
  {
    unsigned len;
    const char* val=sms->getBinProperty(Tag::SMPP_CALLBACK_NUM,&len);
    optional.set_callbackNum(val,len);
  }
}

inline bool fillSmppPduFromSms(PduXSm* pdu,SMS* sms,uint32_t smeFlags=0)
{
  using namespace smsc::smeman;
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
      __require__ ( val_length <= (signed int)sizeof(val) );   // fatal if out of range !!!!
      src.set_value(val);
      src.set_typeOfNumber(addr.getTypeOfNumber());
      src.set_numberingPlan(addr.getNumberingPlan());
    }
    PduAddress& dest = message.get_dest();
    {
      char val[21];
      Address& addr = sms->getDestinationAddress();
      int val_length = addr.getValue(val);
      __require__ ( val_length <= (signed int)sizeof(val) );   // fatal if out of range !!!!
      dest.set_value(val);
      dest.set_typeOfNumber(addr.getTypeOfNumber());
      dest.set_numberingPlan(addr.getNumberingPlan());
    }
    {
      //char msg[256];
      //const smsc::sms::Body& sms_body = sms->getMessageBody();
      unsigned len = 0;
      const char* short_msg = sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
      unsigned msg_length = sms->getIntProperty(Tag::SMPP_SM_LENGTH);
      __require__(msg_length <= len);
      if(msg_length>0)
      {
        if(!HSNS_isEqual() && sms->getIntProperty(Tag::SMPP_DATA_CODING)==8)
        {
          message.set_shortMessage("",0);
          UCS_htons(message.shortMessage.alloc(len),short_msg,msg_length,sms->getIntProperty(Tag::SMPP_ESM_CLASS));
        }else
        {
          message.set_shortMessage(short_msg,msg_length);
        }
      }
      //message.set_smLength((uint8_t)msg_length);
      //message.set_dataCoding((uint8_t)sms_body.getCodingScheme());
      if((smeFlags&sfForceGsmDatacoding) && sms->hasIntProperty(Tag::SMSC_ORIGINAL_DC))
      {
        message.set_dataCoding((uint8_t)sms->getIntProperty(Tag::SMSC_ORIGINAL_DC));
      }else
      {
        message.set_dataCoding((uint8_t)sms->getIntProperty(Tag::SMPP_DATA_CODING));
      }
    }
    //message.set_protocolId(sms->getProtocolIdentifier());
    message.set_protocolId(sms->getIntProperty(Tag::SMPP_PROTOCOL_ID));
    //message.set_priorityFlag(sms->getPriority());
    message.set_priorityFlag(sms->getIntProperty(Tag::SMPP_PRIORITY));
    //message.set_registredDelivery(/*sms->isStatusReportRequested()*/
    //    sms->getDeliveryReport());
    message.set_registredDelivery(sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY));
    {
      char smpp_time[SMPP_TIME_BUFFER_LENGTH];
      if ( cTime2SmppTime(sms->getNextTime(),smpp_time) )
        message.set_scheduleDeliveryTime(smpp_time);
      if ( cTime2SmppTime(sms->getValidTime(),smpp_time) )
        message.set_validityPeriod(smpp_time);
    }
  }
  {
    char buff[7];
    memset(buff,0,sizeof(buff));
    sms->getEServiceType(buff);
    pdu->message.set_serviceType(buff);
  }

  if ( sms->hasIntProperty(Tag::SMPP_PRIORITY))
    pdu->message.set_priorityFlag(sms->getIntProperty(Tag::SMPP_PRIORITY));

  pdu->message.set_esmClass(sms->getIntProperty(Tag::SMPP_ESM_CLASS));

  if ( sms->hasIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG) )
    pdu->message.set_replaceIfPresentFlag(sms->getIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG));
  else
    pdu->message.set_replaceIfPresentFlag(0);


  fillOptional(pdu->optional,sms,smeFlags);
  return true;
}

inline void fetchOptionals(SmppOptional& optional,SMS* sms,uint32_t smeFlags=0)
{
  using namespace smsc::smeman;
  if ( optional.has_userMessageReference() )
    //sms->setMessageReference(optional.get_userMessageReference());
    sms->setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,
                        optional.get_userMessageReference());
//  else
    //sms->setMessageReference(0);
//    sms->setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,0);

  if ( optional.has_receiptedMessageId() )
  {
    // FIXME: do we still need this check?
    /*
    smsc::sms::SMSId id = 0;
    if ( !sscanf(optional.get_receiptedMessageId(),"%lld",&id) )
    {
      __warning__("error when processing receiptedMessageId");
      __watch__(optional.get_receiptedMessageId());
    }
    //sms->setReceiptSmsId(id);
     */
    const char* ptr = optional.get_receiptedMessageId();
    char buffer[65];
    if (strlen(ptr) >= sizeof(buffer)) {
        // make sure the length is 64+1 bytes
        strncpy(buffer,ptr,sizeof(buffer));
        buffer[sizeof(buffer)-1] = '\0';
        ptr = buffer;
    }
    // snprintf(buffer,64,"%lld",id);
    sms->setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,ptr);
  }
  else
  {
    // nothing
    //sms->setStrProperty(Tag::SMPP_RECEIPTED_MESSAGE_ID,0);
  }
    //sms->setReceiptSmsId(0);
  if ( optional.has_ussdServiceOp() )
    sms->setIntProperty(Tag::SMPP_USSD_SERVICE_OP,optional.get_ussdServiceOp());
  if ( optional.has_payloadType() )
    sms->setIntProperty(Tag::SMPP_PAYLOAD_TYPE,optional.get_payloadType());
  if ( optional.has_userResponseCode() )
    sms->setIntProperty(Tag::SMPP_USER_RESPONSE_CODE,optional.get_userResponseCode());
  if ( optional.has_sarMsgRefNum() )
    sms->setIntProperty(Tag::SMPP_SAR_MSG_REF_NUM,optional.get_sarMsgRefNum());
  if ( optional.has_languageIndicator() )
    sms->setIntProperty(Tag::SMPP_LANGUAGE_INDICATOR,optional.get_languageIndicator());
  if ( optional.has_sarTotalSegments() )
    sms->setIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS,optional.get_sarTotalSegments());
  if ( optional.has_numberOfMessages() )
    sms->setIntProperty(Tag::SMPP_NUMBER_OF_MESSAGES,optional.get_numberOfMessages());

  if(optional.has_sourcePort())
    sms->setIntProperty(Tag::SMPP_SOURCE_PORT,optional.get_sourcePort());
  if(optional.has_destinationPort())
    sms->setIntProperty(Tag::SMPP_DESTINATION_PORT,optional.get_destinationPort());
  if(optional.has_sarSegmentSegnum())
    sms->setIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM,optional.get_sarSegmentSegnum());
  if(optional.has_moreMessagesToSend())
    sms->setIntProperty(Tag::SMPP_MORE_MESSAGES_TO_SEND,optional.get_moreMessagesToSend());
  if(optional.has_destNetworkType())
    sms->setIntProperty(Tag::SMPP_DEST_NETWORK_TYPE,optional.get_destNetworkType());
  if(optional.has_destBearerType())
    sms->setIntProperty(Tag::SMPP_DEST_BEARER_TYPE,optional.get_destBearerType());
  if(optional.has_qosTimeToLive())
    sms->setIntProperty(Tag::SMPP_QOS_TIME_TO_LIVE,optional.get_qosTimeToLive());
  if(optional.has_setDpf())
    sms->setIntProperty(Tag::SMPP_SET_DPF,optional.get_setDpf());
  if(optional.has_sourceNetworkType())
    sms->setIntProperty(Tag::SMPP_SOURCE_NETWORK_TYPE,optional.get_sourceNetworkType());
  if(optional.has_sourceBearerType())
    sms->setIntProperty(Tag::SMPP_SOURCE_BEARER_TYPE,optional.get_sourceBearerType());
  if(optional.has_privacyIndicator())
  {
    sms->setIntProperty(Tag::SMPP_PRIVACYINDICATOR,optional.get_privacyIndicator());
  }

  if(optional.has_networkErrorCode())
  {
    uint32_t nec = 0;
    memcpy((uint8_t*)&nec + 1, optional.get_networkErrorCode(), 3);
    sms->setIntProperty(Tag::SMPP_NETWORK_ERROR_CODE,ntohl(nec));
  }

  if(!(smeFlags&sfForceGsmDatacoding))
  {
    if ( optional.has_destAddrSubunit() )
      sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,optional.get_destAddrSubunit());
    if ( optional.has_msMsgWaitFacilities() )
      sms->setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,optional.get_msMsgWaitFacilities());
    if(optional.has_msValidity())
      sms->setIntProperty(Tag::SMPP_MS_VALIDITY,optional.get_msValidity());
  }


  if ( optional.has_messagePayload() )
  {
    sms->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,optional.get_messagePayload(),optional.size_messagePayload());
    if(!HSNS_isEqual() && sms->getIntProperty(Tag::SMPP_DATA_CODING)==8)
    {
      unsigned len;
      char* msg=(char*)sms->getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,&len);
      UCS_ntohs(msg,msg,len,sms->getIntProperty(Tag::SMPP_ESM_CLASS));
    }
  }else
  {
    if(!sms->hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
    {
      sms->setBinProperty(Tag::SMPP_SHORT_MESSAGE,"",0);
    }
  }

  if ( optional.has_messageState() )
  {
    /*
    switch(optional.get_messageState())
    {
      case 1:sms->setIntProperty(Tag::SMPP_MSG_STATE,ENROUTE);break;
      case 2:sms->setIntProperty(Tag::SMPP_MSG_STATE,DELIVERED);break;
      case 3:sms->setIntProperty(Tag::SMPP_MSG_STATE,EXPIRED);break;
      case 4:sms->setIntProperty(Tag::SMPP_MSG_STATE,DELETED);break;
      case 5:sms->setIntProperty(Tag::SMPP_MSG_STATE,UNDELIVERABLE);break;
      case 6://accepted
      case 7://unknown
      case 8://rejected
        //sms->setIntProperty(Tag::SMPP_MSG_STATE,???);
        break;
      default: __warning__("UNKNOWN OR UNSUPPORTED MESSAGE STATE");break;
    }*/
    sms->setIntProperty(Tag::SMPP_MSG_STATE,optional.get_messageState());
  }

  if ( optional.has_supported_locale() )
    sms->setStrProperty( Tag::SMSC_SUPPORTED_LOCALE, optional.get_supported_locale() );

  if ( optional.has_imsi_address() )
    sms->setStrProperty( Tag::SMSC_IMSI_ADDRESS, optional.get_imsi_address() );

  if ( optional.has_msc_address() )
    sms->setStrProperty( Tag::SMSC_MSC_ADDRESS, optional.get_msc_address() );

  if ( optional.has_supported_codeset() )
    sms->setIntProperty( Tag::SMSC_SUPPORTED_CODESET, optional.get_supported_codeset() );

  if( optional.has_sccp_oa())
  {
    sms->setStrProperty(Tag::SMSC_SCCP_OA,optional.get_sccp_oa());
  }
  if( optional.has_sccp_da())
  {
    sms->setStrProperty(Tag::SMSC_SCCP_DA,optional.get_sccp_da());
  }

  if(optional.has_itsSessionInfo())
  {
    const uint8_t* arr=optional.get_itsSessionInfo();
    uint16_t val=arr[0]|(arr[1]<<8);
    sms->setIntProperty(Tag::SMPP_ITS_SESSION_INFO,val);
  }

  if((smeFlags&sfSmppPlus) && optional.has_ussd_session_id())
  {
    sms->setIntProperty(Tag::SMPP_USER_MESSAGE_REFERENCE,optional.get_ussd_session_id());
  }

  if(optional.has_unknownFields())
  {
    sms->setBinProperty(Tag::SMSC_UNKNOWN_OPTIONALS,optional.get_unknownFields(),optional.size_unknownFields());
  }
  if(optional.has_callbackNum())
  {
    sms->setBinProperty(Tag::SMPP_CALLBACK_NUM,optional.get_callbackNum(),optional.size_callbackNum());
  }
//  if ( optional.has_protocol_id() )
//    sms->setIntProperty(Tag::SMPP_PROTOCOL_ID,(uint32_t)optional.get_protocol_id());
}

inline bool fetchSmsFromSmppPdu(PduXSm* pdu,SMS* sms,uint32_t smeFlags=0)
{
  using namespace smsc::smeman;
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
      Address originatingAddr(source.value.cstr()?(uint8_t)strlen(source.value.cstr()):1,
                            source.typeOfNumber,
                            source.numberingPlan,
                            source.value.cstr()?source.value.cstr():"0");
      Address destinationAddr(dest.value.cstr()?(uint8_t)strlen(dest.value.cstr()):1,
                            dest.typeOfNumber,
                            dest.numberingPlan,
                            dest.value.cstr()?dest.value.cstr():"0");/**/
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
    //sms->setMessageBody((unsigned char)message.shortMessage.size(), (unsigned char)message.dataCoding, false, (uint8_t*)message.shortMessage.cstr());
    if(message.shortMessage.length)
    {
      sms->setBinProperty(Tag::SMPP_SHORT_MESSAGE,
                          message.shortMessage.cstr()?message.shortMessage.cstr():"",message.shortMessage.length);
      sms->setIntProperty(Tag::SMPP_SM_LENGTH,(uint32_t)message.shortMessage.size());
      if(!HSNS_isEqual() && message.dataCoding==8)
      {
        unsigned len;
        char* msg=(char*)sms->getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
        UCS_ntohs(msg,msg,len,message.get_esmClass());
      }
    }
    int dc=(uint32_t)message.dataCoding;
    if(smeFlags&sfForceGsmDatacoding)
    {
      int user_data_coding=dc;
      sms->setIntProperty(Tag::SMSC_ORIGINAL_DC,dc);
      sms->setIntProperty(Tag::SMSC_FORCE_DC,1);
      unsigned encoding = 0;
      if ( (user_data_coding & 0xc0) == 0 ||  // 00xxxxxx
           (user_data_coding & 0xc0) == 0x40 )  // 01xxxxxx
      {
        encoding = user_data_coding&0x0c;
        if(encoding==0)encoding=DataCoding::SMSC7BIT;
        if(encoding==0x0c)encoding=DataCoding::BINARY;
        if ( (user_data_coding & 0xc0) == 0x40 )
        {
          sms->setIntProperty(Tag::SMPP_MS_VALIDITY,0x03);
        }
        int das=(user_data_coding&0x03)+1;
        sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,das);
        if ( das!=3 && (user_data_coding&(1<<5)) )
        {
          __trace__("SmppToSms: required compression");
          return false;
        }
        if(sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)==0x03)
        {
          encoding=DataCoding::BINARY;
        }
      }
      else if ( (user_data_coding & 0xf0) == 0xc0 ) // 1100xxxx
      {
        encoding = DataCoding::SMSC7BIT;
        sms->setIntProperty(Tag::SMPP_MS_VALIDITY,0x3);
        sms->setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                           (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
      }
      else if ( (user_data_coding & 0xf0) == 0xd0 ) // 1101xxxx
      {
        encoding = DataCoding::SMSC7BIT;
        sms->setIntProperty(Tag::SMPP_MS_VALIDITY,0x0);
        sms->setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                           (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
      }
      else if ( (user_data_coding & 0xf0) == 0xe0 ) // 1110xxxx
      {
        encoding = DataCoding::UCS2;
        sms->setIntProperty(Tag::SMPP_MS_VALIDITY,0x0);
        sms->setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                           (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
      }
      else if ( (user_data_coding & 0xf0) == 0xf0 ) // 1111xxxx
      {
        if ( user_data_coding & 0x4 ) encoding = DataCoding::BINARY;
        else encoding = DataCoding::SMSC7BIT;
        sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,(user_data_coding&0x3)+1);
      }
      else{
        encoding=DataCoding::BINARY;
      }
      __trace2__("forceDC: %d->%d",dc,encoding);
      sms->setIntProperty(Tag::SMPP_DATA_CODING,encoding);
    }else
    {
      /*
      if((dc&0xf0)==0xf0 && (dc&0x08)==0)
      {
        sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,(dc&0x03)+1);
        sms->setIntProperty(Tag::SMPP_DATA_CODING,(dc&0x04)?DataCoding::BINARY:DataCoding::SMSC7BIT);
      }else
      {
      */
        sms->setIntProperty(Tag::SMPP_DATA_CODING,dc);
      //}
    }

    sms->setIntProperty(Tag::SMPP_PRIORITY,(uint32_t)message.priorityFlag);
    sms->setIntProperty(Tag::SMPP_PROTOCOL_ID,(uint32_t)message.protocolId);

    if ( message.scheduleDeliveryTime.cstr() )
      sms->setNextTime(smppTime2CTime(message.scheduleDeliveryTime));
    else
      sms->setNextTime(0);

    if ( message.validityPeriod.cstr() )
      sms->setValidTime(smppTime2CTime(message.validityPeriod));
    else
      sms->setValidTime(0);

    //sms->setDeliveryReport(message.registredDelivery);
    sms->setIntProperty(Tag::SMPP_REGISTRED_DELIVERY,(uint32_t)message.registredDelivery);
    sms->setIntProperty(Tag::SMPP_PRIORITY,pdu->message.get_priorityFlag());
    sms->setIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG,pdu->message.get_replaceIfPresentFlag());
  }
  sms->setEServiceType(pdu->message.get_serviceType());
  sms->setIntProperty(Tag::SMPP_ESM_CLASS,(uint32_t)pdu->message.get_esmClass());
  fetchOptionals(pdu->optional,sms,smeFlags);
  return true;
}

inline bool fetchSmsFromDataSmPdu(PduDataSm* pdu,SMS* sms,uint32_t smeFlags=0)
{
  using namespace smsc::smeman;
  PduDataPartSm& data = pdu->get_data();
  { // fill address
    PduAddress& source = data.source;
    PduAddress& dest  = data.dest;
    Address originatingAddr(source.value.cstr()?(uint8_t)strlen(source.value.cstr()):1,
                          source.typeOfNumber,
                          source.numberingPlan,
                          source.value.cstr()?source.value.cstr():"0");
    Address destinationAddr(dest.value.cstr()?(uint8_t)strlen(dest.value.cstr()):1,
                          dest.typeOfNumber,
                          dest.numberingPlan,
                          dest.value.cstr()?dest.value.cstr():"0");
    sms->setOriginatingAddress(originatingAddr);
    sms->setDestinationAddress(destinationAddr);
  }
  sms->setIntProperty(Tag::SMPP_ESM_CLASS,(uint32_t)data.get_esmClass());
  sms->setIntProperty(Tag::SMPP_REGISTRED_DELIVERY,data.get_registredDelivery());

  int dc=(uint32_t)data.dataCoding;
  if(smeFlags&sfForceGsmDatacoding)
  {
    int user_data_coding=dc;
    sms->setIntProperty(Tag::SMSC_ORIGINAL_DC,dc);
    sms->setIntProperty(Tag::SMSC_FORCE_DC,1);
    unsigned encoding = 0;
    if ( (user_data_coding & 0xc0) == 0 ||  // 00xxxxxx
         (user_data_coding & 0xc0) == 0x40 )  // 01xxxxxx
    {
      encoding = user_data_coding&0x0c;
      if(encoding==0)encoding=DataCoding::SMSC7BIT;
      if(encoding==0x0c)encoding=DataCoding::BINARY;
      if ( (user_data_coding & 0xc0) == 0x40 )
      {
        sms->setIntProperty(Tag::SMPP_MS_VALIDITY,0x03);
      }
      int das=(user_data_coding&0x03)+1;
      sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,das);
      if ( das!=3 && (user_data_coding&(1<<5)) )
      {
        __trace__("SmppDataToSms: required compression");
        return false;
      }
      if(sms->getIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT)==0x03)
      {
        encoding=DataCoding::BINARY;
      }
    }
    else if ( (user_data_coding & 0xf0) == 0xc0 ) // 1100xxxx
    {
      encoding = DataCoding::SMSC7BIT;
      sms->setIntProperty(Tag::SMPP_MS_VALIDITY,0x3);
      sms->setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                         (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
    }
    else if ( (user_data_coding & 0xf0) == 0xd0 ) // 1101xxxx
    {
      encoding = DataCoding::SMSC7BIT;
      sms->setIntProperty(Tag::SMPP_MS_VALIDITY,0x0);
      sms->setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                         (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
    }
    else if ( (user_data_coding & 0xf0) == 0xe0 ) // 1110xxxx
    {
      encoding = DataCoding::UCS2;
      sms->setIntProperty(Tag::SMPP_MS_VALIDITY,0x0);
      sms->setIntProperty(Tag::SMPP_MS_MSG_WAIT_FACILITIES,
                         (user_data_coding&0x3)|((user_data_coding&0x8)<<4));
    }
    else if ( (user_data_coding & 0xf0) == 0xf0 ) // 1111xxxx
    {
      if ( user_data_coding & 0x4 ) encoding = DataCoding::BINARY;
      else encoding = DataCoding::SMSC7BIT;
      sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,(user_data_coding&0x3)+1);
    }
    else{
      encoding=DataCoding::BINARY;
    }
    __trace2__("forceDC: %d->%d",dc,encoding);
    sms->setIntProperty(Tag::SMPP_DATA_CODING,encoding);
  }else
  {
    /*
    if((dc&0xf0)==0xf0 && (dc&0x08)==0)
    {
      sms->setIntProperty(Tag::SMPP_DEST_ADDR_SUBUNIT,(dc&0x03)+1);
      sms->setIntProperty(Tag::SMPP_DATA_CODING,(dc&0x04)?DataCoding::BINARY:DataCoding::SMSC7BIT);
    }else
    {
    */
      sms->setIntProperty(Tag::SMPP_DATA_CODING,dc);
    //}
  }

  fetchOptionals(pdu->optional,sms,smeFlags);
  sms->setEServiceType(pdu->data.get_serviceType());
  if(pdu->optional.has_qosTimeToLive())
  {
    sms->setValidTime(time(NULL)+pdu->optional.get_qosTimeToLive());
  }else
  {
    sms->setValidTime(0);
  }
  sms->setIntProperty(Tag::SMPP_PRIORITY,0);
  sms->setIntProperty(Tag::SMPP_PROTOCOL_ID,0);
  if ( pdu->optional.has_protocol_id() )
    sms->setIntProperty(Tag::SMPP_PROTOCOL_ID,(uint32_t)pdu->optional.get_protocol_id());
  sms->setIntProperty(Tag::SMPP_REPLACE_IF_PRESENT_FLAG,0);
  sms->setNextTime(0);
  return true;
}

inline bool fillDataSmFromSms(PduDataSm* pdu,SMS* sms,uint32_t smeFlags=0)
{
  using namespace smsc::smeman;
  PduDataPartSm& data = pdu->get_data();
  PduAddress& src = data.get_source();
  {
    char val[21];
    Address& addr = sms->getOriginatingAddress();
    int val_length = addr.getValue(val);
    __require__ ( val_length <= (signed int)sizeof(val) );   // fatal if out of range !!!!
    src.set_value(val);
    src.set_typeOfNumber(addr.getTypeOfNumber());
    src.set_numberingPlan(addr.getNumberingPlan());
  }
  PduAddress& dest = data.get_dest();
  {
    char val[21];
    Address& addr = sms->getDestinationAddress();
    int val_length = addr.getValue(val);
    __require__ ( val_length <= (signed int)sizeof(val) );   // fatal if out of range !!!!
    dest.set_value(val);
    dest.set_typeOfNumber(addr.getTypeOfNumber());
    dest.set_numberingPlan(addr.getNumberingPlan());
  }
  {
    char buff[7];
    memset(buff,0,sizeof(buff));
    sms->getEServiceType(buff);
    pdu->data.set_serviceType(buff);
  }

  fillOptional(pdu->optional,sms,smeFlags);
  data.set_esmClass(sms->getIntProperty(Tag::SMPP_ESM_CLASS));
  if((smeFlags&sfForceGsmDatacoding) && sms->hasIntProperty(Tag::SMSC_ORIGINAL_DC))
  {
    data.set_dataCoding((uint8_t)sms->getIntProperty(Tag::SMSC_ORIGINAL_DC));
  }else
  {
    data.set_dataCoding((uint8_t)sms->getIntProperty(Tag::SMPP_DATA_CODING));
  }
  data.set_registredDelivery(sms->getIntProperty(Tag::SMPP_REGISTRED_DELIVERY));
  return true;
}

}
}

#endif
