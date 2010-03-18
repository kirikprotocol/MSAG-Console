/*
  $Id$
*/


#if !defined  __Cxx_Header__smpp_structures_h__
#define  __Cxx_Header__smpp_structures_h__

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif
#include "util/debug.h"
#include "smpp_memory.h"
#include "smpp_strings.h"

namespace smsc{
namespace smpp{


namespace SmppOptionalTags{
  static const uint16_t destAddrSubunit  /*dest_addr_subunit(5.3.2.1)*/ = 0x05;
  static const uint16_t destNetworkType  /*dest_network_type(5.3.2.3)*/ = 0x06;
  static const uint16_t destBearerType   /*dest_bearer_type(5.3.2.5)*/ = 0x07;
  static const uint16_t destTelematicsId /*dest_telematics_id(5.3.2.7)*/ = 0x08;
  static const uint16_t sourceAddrSubunit /*source_addr_subunit(5.3.2.2)*/ = 0x0d;
  static const uint16_t sourceNetworkType /*source_network_type(5.3.2.4)*/ = 0x0e;
  static const uint16_t sourceBearerType  /*source_bearer_type(5.3.2.6)*/  = 0x0f;
  static const uint16_t sourceTelematicsId/*source_telematics_id(5.3.2.8)*/ = 0x010;
  static const uint16_t qosTimeToLive     /*qos_time_to_live(5.3.2.9)*/ = 0x017;
  static const uint16_t payloadType        /*payload_type(5.3.2.10)*/ = 0x019;
  static const uint16_t additionalStatusInfoText /*additional_status_info_text(5.3.2.11)*/ = 0x01d;
  static const uint16_t receiptedMessageId /*receipted_message_id(5.3.2.12)*/ = 0x01e;
  static const uint16_t msMsgWaitFacilities/*ms_msg_wait_facilities(5.3.2.13)*/ = 0x030;
  static const uint16_t privacyIndicator   /*privacy_indicator(5.3.2.14)*/ = 0x0201;
  static const uint16_t sourceSubaddress   /*source_subaddress(5.3.2.15)*/ = 0x0202;
  static const uint16_t destSubaddress     /*dest_subaddress(5.3.2.16)*/ = 0x0203;
  static const uint16_t userMessageReference /*user_message_reference(5.3.2.17)*/ = 0x0204;
  static const uint16_t userResponseCode   /*user_response_code(5.3.2.18)*/  = 0x0205;
  static const uint16_t sourcePort         /*source_port(5.3.2.20)*/ = 0x020a;
  static const uint16_t destinationPort    /*destination_port(5.3.2.21)*/ = 0x020b;
  static const uint16_t sarMsgRefNum       /*sar_msg_ref_num(5.3.2.22)*/ = 0x020c;
  static const uint16_t languageIndicator  /*language_indecator(5.3.2.19)*/ = 0x020d;
  static const uint16_t sarTotalSegments   /*sar_total_segments(5.3.2.23)*/ = 0x020e;
  static const uint16_t sarSegmentSegnum   /*sar_segment_seqnum(5.3.2.24)*/ = 0x020f;
  static const uint16_t scInterfaceVersion /*sc_interface_version(5.3.2.25)*/ = 0x0210;
  static const uint16_t callbackNumPresInd  /*callback_num_pres_ind(5.3.2.37)*/ = 0x0302;
  static const uint16_t callbackNumAtag     /*callback_num_atag(5.3.2.38)*/ = 0x0303;
  static const uint16_t numberOfMessages    /*number_of_messages(5.3.2.39)*/ = 0x0304;
  static const uint16_t callbackNum         /*callback_num(5.3.2.36)*/  = 0x0381;
  static const uint16_t dpfResult           /*dpf_result(5.3.2.28)*/  = 0x0420;
  static const uint16_t setDpf              /*set_dpf(5.3.2.29)*/ = 0x0421;
  static const uint16_t msAvailableStatus   /*ms_available_status(5.3.2.30)*/ = 0x0422;
  static const uint16_t networkErrorCode    /*network_error_code(5.3.2.31)*/ = 0x0423;
  static const uint16_t messagePayload       /*message_payload(5.3.2.32)*/ = 0x0424;
  static const uint16_t deliveryFailureReason /*delivery_failure_reason(5.3.2.33)*/ = 0x0425;
  static const uint16_t moreMessagesToSend  /*more_messages_to_send(5.3.2.34)*/ = 0x0426;
  static const uint16_t messageState        /*message_state(5.3.2.35)*/ = 0x0427;
  static const uint16_t ussdServiceOp       /*ussd_service_op(5.3.2.44)*/ = 0x0501;
  static const uint16_t displayTime         /*display_time(5.3.2.26)*/ = 0x1201;
  static const uint16_t smsSignal           /*sms_signal(5.3.2.40)*/ = 0x1203;
  static const uint16_t msValidity          /*ms_validity(5.3.2.27)*/ = 0x1204;
  static const uint16_t alertOnMessageDelivery /*alert_on_message_delivery(5.3.2.41)*/ = 0x130c;
  static const uint16_t itsReplyType        /*its_reply_type(5.3.2.42)*/ = 0x1380;
  static const uint16_t itsSessionInfo    /*its_session_info(5.3.2.43)*/ = 0x1383;
  /* SMSC specific */
  static const uint16_t supported_locale    = 0x4100;
  static const uint16_t supported_codeset   = 0x4101;
  static const uint16_t imsi_address        = 0x4102;
  static const uint16_t msc_address         = 0x4103;
  static const uint16_t protocol_id         = 0x4104;
  static const uint16_t sccp_oa             = 0x4105;
  static const uint16_t sccp_da             = 0x4106;
  /* SMPP+ specific*/
  static const uint16_t ussd_session_id     = 0x1501;
  static const uint16_t imsi                = 0x1502;
  static const uint16_t hlr_address_ton     = 0x1800;
  static const uint16_t hlr_address_npi     = 0x1801;
  static const uint16_t hlr_address         = 0x1802;
  static const uint16_t vlr_number_ton      = 0x1503;
  static const uint16_t vlr_number_npi      = 0x1504;
  static const uint16_t vlr_number          = 0x1505;
}

namespace SmppOptionalLength{
  static const uint16_t destAddrSubunit  /*dest_addr_subunit(5.3.2.1)*/ = 1;
  static const uint16_t destNetworkType  /*dest_network_type(5.3.2.3)*/ = 1;
  static const uint16_t destBearerType   /*dest_bearer_type(5.3.2.5)*/ = 1;
  static const uint16_t destTelematicsId /*dest_telematics_id(5.3.2.7)*/ = 2;
  static const uint16_t sourceAddrSubunit /*source_addr_subunit(5.3.2.2)*/ = 1;
  static const uint16_t sourceNetworkType /*source_network_type(5.3.2.4)*/ = 1;
  static const uint16_t sourceBearerType  /*source_bearer_type(5.3.2.6)*/  = 1;
  static const uint16_t sourceTelematicsId/*source_telematics_id(5.3.2.8)*/ = 1;
  static const uint16_t qosTimeToLive     /*qos_time_to_live(5.3.2.9)*/ = 4;
  static const uint16_t payloadType       /*payload_type(5.3.2.10)*/ = 1;
  static const uint16_t additionalStatusInfoText /*additional_status_info_text(5.3.2.11)*/ = 256;
  static const uint16_t receiptedMessageId /*receipted_message_id(5.3.2.12)*/ = 65;
  static const uint16_t msMsgWaitFacilities/*ms_msg_wait_facilities(5.3.2.13)*/ = 1;
  static const uint16_t privacyIndicator   /*privacy_indicator(5.3.2.14)*/ = 1;
  static const uint16_t sourceSubaddress   /*source_subaddress(5.3.2.15)*/ = 23;
  static const uint16_t destSubaddress     /*dest_subaddress(5.3.2.16)*/ = 23;
  static const uint16_t userMessageReference /*user_message_reference(5.3.2.17)*/ = 2;
  static const uint16_t userResponseCode   /*user_response_code(5.3.2.18)*/  = 1;
  static const uint16_t sourcePort         /*source_port(5.3.2.20)*/ = 2;
  static const uint16_t destinationPort    /*destination_port(5.3.2.21)*/ = 2;
  static const uint16_t sarMsgRefNum       /*sar_msg_ref_num(5.3.2.22)*/ = 2;
  static const uint16_t languageIndicator  /*language_indecator(5.3.2.19)*/ = 1;
  static const uint16_t sarTotalSegments   /*sar_total_segments(5.3.2.23)*/ = 1;
  static const uint16_t sarSegmentSegnum   /*sar_segment_seqnum(5.3.2.24)*/ = 1;
  static const uint16_t scInterfaceVersion /*sc_interface_version(5.3.2.25)*/ = 1;
  static const uint16_t callbackNumPresInd /*callback_num_pres_ind(5.3.2.37)*/ = 1;
  static const uint16_t callbackNumAtag     /*callback_num_atag(5.3.2.38)*/ = 65;
  static const uint16_t numberOfMessages    /*number_of_messages(5.3.2.39)*/ = 1;
  static const uint16_t callbackNum         /*callback_num(5.3.2.36)*/  = 19;
  static const uint16_t dpfResult           /*dpf_result(5.3.2.28)*/  = 1;
  static const uint16_t setDpf              /*set_dpf(5.3.2.29)*/ = 1;
  static const uint16_t msAvailableStatus   /*ms_available_status(5.3.2.30)*/ = 1;
  static const uint16_t networkErrorCode    /*network_error_code(5.3.2.31)*/ = 3;
  static const uint16_t messagePayload       /*message_payload(5.3.2.32)*/ = 0xffff;
  static const uint16_t deliveryFailureReason /*delivery_failure_reason(5.3.2.33)*/ = 1;
  static const uint16_t moreMessagesToSend  /*more_messages_to_send(5.3.2.34)*/ = 1;
  static const uint16_t messageState        /*message_state(5.3.2.35)*/ = 1;
  static const uint16_t ussdServiceOp       /*ussd_service_op(5.3.2.44)*/ = 1;
  static const uint16_t displayTime         /*display_time(5.3.2.26)*/ = 1;
  static const uint16_t smsSignal           /*sms_signal(5.3.2.40)*/ = 2;
  static const uint16_t msValidity          /*ms_validity(5.3.2.27)*/ = 1;
  static const uint16_t alertOnMessageDelivery /*alert_on_message_delivery(5.3.2.41)*/ = 0;
  static const uint16_t itsReplyType        /*its_reply_type(5.3.2.42)*/ = 1;
  static const uint16_t itsSessionInfo    /*its_session_info(5.3.2.43)*/ = 2;
  /* SMSC_specific  */
  static const uint16_t supported_locale    = 31;
  static const uint16_t supported_codeset   = 2;
  static const uint16_t imsi_address        = 31;
  static const uint16_t msc_address         = 31;
  static const uint16_t protocol_id         = 1;
  static const uint16_t sccp_oa             =21;
  static const uint16_t sccp_da             =21;
  /* SMPP+ specific*/
  static const uint16_t ussd_session_id     = 4;
  static const uint16_t imsi                = 15;
  static const uint16_t hlr_address_ton     = 1;
  static const uint16_t hlr_address_npi     = 1;
  static const uint16_t hlr_address         = 21;
  static const uint16_t vlr_number_ton      = 1;
  static const uint16_t vlr_number_npi      = 1;
  static const uint16_t vlr_number          = 21;

}

namespace SmppOptionalFields{
#define BIT(x) ((uint64_t)1<<(x))
  static const uint64_t destAddrSubunit  /*dest_addr_subunit(5.3.2.1)*/ = BIT(0);
  static const uint64_t destNetworkType  /*dest_network_type(5.3.2.3)*/ = BIT(1);
  static const uint64_t destBearerType   /*dest_bearer_type(5.3.2.5)*/ = BIT(2);
  static const uint64_t destTelematicsId /*dest_telematics_id(5.3.2.7)*/ = BIT(3);
  static const uint64_t sourceAddrSubunit /*source_addr_subunit(5.3.2.2)*/ = BIT(4);
  static const uint64_t sourceNetworkType /*source_network_type(5.3.2.4)*/ = BIT(5);
  static const uint64_t sourceBearerType  /*source_bearer_type(5.3.2.6)*/  = BIT(6);
  static const uint64_t sourceTelematicsId/*source_telematics_id(5.3.2.8)*/ = BIT(7);
  static const uint64_t qosTimeToLive     /*qos_time_to_live(5.3.2.9)*/ = BIT(8);
  static const uint64_t payloadType       /*payload_type(5.3.2.10)*/ = BIT(9);
  static const uint64_t additionalStatusInfoText /*additional_status_info_text(5.3.2.11)*/ = BIT(10);
  static const uint64_t receiptedMessageId /*receipted_message_id(5.3.2.12)*/ = BIT(11);
  static const uint64_t msMsgWaitFacilities/*ms_msg_wait_facilities(5.3.2.13)*/ = BIT(12);
  static const uint64_t privacyIndicator   /*privacy_indicator(5.3.2.14)*/ = BIT(13);
  static const uint64_t sourceSubaddress   /*source_subaddress(5.3.2.15)*/ = BIT(14);
  static const uint64_t destSubaddress     /*dest_subaddress(5.3.2.16)*/ = BIT(15);
  static const uint64_t userMessageReference /*user_message_reference(5.3.2.17)*/ = BIT(16);
  static const uint64_t userResponseCode   /*user_response_code(5.3.2.18)*/  = BIT(17);
  static const uint64_t sourcePort         /*source_port(5.3.2.20)*/ = BIT(18);
  static const uint64_t destinationPort    /*destination_port(5.3.2.21)*/ = BIT(19);
  static const uint64_t sarMsgRefNum       /*sar_msg_ref_num(5.3.2.22)*/ = BIT(20);
  static const uint64_t languageIndicator  /*language_indecator(5.3.2.19)*/ = BIT(21);
  static const uint64_t sarTotalSegments   /*sar_total_segments(5.3.2.23)*/ = BIT(22);
  static const uint64_t sarSegmentSegnum   /*sar_segment_seqnum(5.3.2.24)*/ = BIT(23);
  static const uint64_t scInterfaceVersion /*sc_interface_version(5.3.2.25)*/ = BIT(24);
  static const uint64_t callbackNumPresInd /*callback_num_pres_ind(5.3.2.37)*/ = BIT(25);
  static const uint64_t callbackNumAtag     /*callback_num_atag(5.3.2.38)*/ = BIT(26);
  static const uint64_t numberOfMessages    /*number_of_messages(5.3.2.39)*/ = BIT(27);
  static const uint64_t callbackNum         /*callback_num(5.3.2.36)*/  = BIT(28);
  static const uint64_t dpfResult           /*dpf_result(5.3.2.28)*/  = BIT(29);
  static const uint64_t setDpf              /*set_dpf(5.3.2.29)*/ = BIT(30);
  static const uint64_t msAvailableStatus   /*ms_available_status(5.3.2.30)*/ = BIT(31);
  static const uint64_t networkErrorCode      /*network_error_code(5.3.2.31)*/ = BIT(32);
  static const uint64_t messagePayload       /*message_payload(5.3.2.32)*/ = BIT(33);
  static const uint64_t deliveryFailureReason /*delivery_failure_reason(5.3.2.33)*/ = BIT(34);
  static const uint64_t moreMessagesToSend  /*more_messages_to_send(5.3.2.34)*/ = BIT(35);
  static const uint64_t messageState        /*message_state(5.3.2.35)*/ = BIT(36);
  static const uint64_t ussdServiceOp       /*ussd_service_op(5.3.2.44)*/ = BIT(37);
  static const uint64_t displayTime         /*display_time(5.3.2.26)*/ = BIT(38);
  static const uint64_t smsSignal           /*sms_signal(5.3.2.40)*/ = BIT(39);
  static const uint64_t msValidity          /*ms_validity(5.3.2.27)*/ = BIT(40);
  static const uint64_t alertOnMessageDelivery /*alert_on_message_delivery(5.3.2.41)*/ = BIT(41);
  static const uint64_t itsReplyType       /*its_reply_type(5.3.2.42)*/ = BIT(42);
  static const uint64_t itsSessionInfo     /*its_session_info(5.3.2.43)*/ = BIT(43);
  /* SMSC_specific  */
  static const uint64_t supported_locale    = BIT(44);
  static const uint64_t supported_codeset   = BIT(45);
  static const uint64_t imsi_address        = BIT(46);
  static const uint64_t msc_address         = BIT(47);

  static const uint64_t unknownFields       = BIT(48);
  static const uint64_t protocol_id         = BIT(49);

  static const uint64_t sccp_oa         = BIT(50);
  static const uint64_t sccp_da         = BIT(51);

  /* SMPP+ specific*/
  static const uint64_t ussd_session_id     = BIT(52);
  static const uint64_t imsi                = BIT(53);
  static const uint64_t hlr_address_ton     = BIT(54);
  static const uint64_t hlr_address_npi     = BIT(55);
  static const uint64_t hlr_address         = BIT(56);
  static const uint64_t vlr_number_ton      = BIT(57);
  static const uint64_t vlr_number_npi      = BIT(58);
  static const uint64_t vlr_number          = BIT(59);

#undef BIT
}

namespace SmppCommandSet{ /* SMPP v3.4 (5.1.2.1) */
  static const uint32_t GENERIC_NACK                 = 0x80000000;
  static const uint32_t BIND_RECIEVER                = 0x00000001;
  static const uint32_t BIND_RECIEVER_RESP           = 0x80000001;
  static const uint32_t BIND_TRANSMITTER             = 0x00000002;
  static const uint32_t BIND_TRANSMITTER_RESP        = 0x80000002;
  static const uint32_t QUERY_SM                     = 0x00000003;
  static const uint32_t QUERY_SM_RESP                = 0x80000003;
  static const uint32_t SUBMIT_SM                    = 0x00000004;
  static const uint32_t SUBMIT_SM_RESP               = 0x80000004;
  static const uint32_t DELIVERY_SM                  = 0x00000005;
  static const uint32_t DELIVERY_SM_RESP             = 0x80000005;
  static const uint32_t UNBIND                       = 0x00000006;
  static const uint32_t UNBIND_RESP                  = 0x80000006;
  static const uint32_t REPLACE_SM                   = 0x00000007;
  static const uint32_t REPLACE_SM_RESP              = 0x80000007;
  static const uint32_t CANCEL_SM                    = 0x00000008;
  static const uint32_t CANCEL_SM_RESP               = 0x80000008;
  static const uint32_t BIND_TRANCIEVER              = 0x00000009;
  static const uint32_t BIND_TRANCIEVER_RESP         = 0x80000009;
  static const uint32_t OUTBIND                      = 0x0000000b;
  static const uint32_t ENQUIRE_LINK                 = 0x00000015;
  static const uint32_t ENQUIRE_LINK_RESP            = 0x80000015;
  static const uint32_t SUBMIT_MULTI                 = 0x00000021;
  static const uint32_t SUBMIT_MULTI_RESP            = 0x80000021;
  static const uint32_t ALERT_NOTIFICATION           = 0x00000102;
  static const uint32_t DATA_SM                      = 0x00000103;
  static const uint32_t DATA_SM_RESP                 = 0x80000103;
}

namespace SmppStatusSet
{
  static const uint32_t ESME_ROK                  = 0x00000000;
  static const uint32_t ESME_RINVMSGLEN           = 0x00000001;
  static const uint32_t ESME_RINVCMDLEN           = 0x00000002;
  static const uint32_t ESME_RINVCMDID            = 0x00000003;
  static const uint32_t ESME_RINVBNDSTS           = 0x00000004;
  static const uint32_t ESME_RALYBND              = 0x00000005;
  static const uint32_t ESME_RINVPRTFLG           = 0x00000006;
  static const uint32_t ESME_RINVREGDLVFLG        = 0x00000007;
  static const uint32_t ESME_RSYSERR              = 0x00000008;
  static const uint32_t ESME_RINVSRCADR           = 0x0000000A;
  static const uint32_t ESME_RINVDSTADR           = 0x0000000B;
  static const uint32_t ESME_RINVMSGID            = 0x0000000C;
  static const uint32_t ESME_RBINDFAIL            = 0x0000000D;
  static const uint32_t ESME_RINVPASWD            = 0x0000000E;
  static const uint32_t ESME_RINVSYSID            = 0x0000000F;
  static const uint32_t ESME_RCANCELFAIL          = 0x00000011;
  static const uint32_t ESME_RREPLACEFAIL         = 0x00000013;
  static const uint32_t ESME_RMSGQFUL             = 0x00000014;
  static const uint32_t ESME_RINVSERTYP           = 0x00000015;
  static const uint32_t ESME_RINVNUMDESTS         = 0x00000033;
  static const uint32_t ESME_RINVDLNAME           = 0x00000034;
  static const uint32_t ESME_RINVDESTFLAG         = 0x00000040;
  static const uint32_t ESME_RINVSUBREP           = 0x00000042;
  static const uint32_t ESME_RINVESMCLASS         = 0x00000043;
  static const uint32_t ESME_RCNTSUBDL            = 0x00000044;
  static const uint32_t ESME_RSUBMITFAIL          = 0x00000045;
  static const uint32_t ESME_RINVSRCTON           = 0x00000048;
  static const uint32_t ESME_RINVSRCNPI           = 0x00000049;
  static const uint32_t ESME_RINVDSTTON           = 0x00000050;
  static const uint32_t ESME_RINVDSTNPI           = 0x00000051;
  static const uint32_t ESME_RINVSYSTYP           = 0x00000053;
  static const uint32_t ESME_RINVREPFLAG          = 0x00000054;
  static const uint32_t ESME_RINVNUMMSGS          = 0x00000055;
  static const uint32_t ESME_RTHROTTLED           = 0x00000058;
  static const uint32_t ESME_RINVSCHED            = 0x00000061;
  static const uint32_t ESME_RINVEXPIRY           = 0x00000062;
  static const uint32_t ESME_RINVDFTMSGID         = 0x00000063;
  static const uint32_t ESME_RX_T_APPN            = 0x00000064;
  static const uint32_t ESME_RX_P_APPN            = 0x00000065;
  static const uint32_t ESME_RX_R_APPN            = 0x00000066;
  static const uint32_t ESME_RQUERYFAIL           = 0x00000067;
  static const uint32_t ESME_RINVOPTPARSTREAM     = 0x000000C0;
  static const uint32_t ESME_ROPTPARNOTALLWD      = 0x000000C1;
  static const uint32_t ESME_RINVPARLEN           = 0x000000C2;
  static const uint32_t ESME_RMISSINGOPTPARAM     = 0x000000C3;
  static const uint32_t ESME_RINVOPTPARAMVAL      = 0x000000C4;
  static const uint32_t ESME_RDELIVERYFAILURE     = 0x000000FE;
  static const uint32_t ESME_RUNKNOWNERR          = 0x000000FF;
  static const uint32_t ESME_RINVDCS              = 0x00000104;

}

namespace AddrSubunitValue{ /* SMPP v3.4 (5.3.2.1) */
  static const uint8_t UNKNOWN          = 0;
  static const uint8_t MS_DISPLAY       = 0x1;
  static const uint8_t MOBILE_EQUIPMENT = 0x2;
  static const uint8_t SMART_CARD1      = 0x3;
  static const uint8_t EXTERNAL_UNIT    = 0x4;
}

namespace NetworkTypeValue{ /* SMPP v3.4 (5.3.2.3) */
  static const uint8_t UNKNOWN          = 0;
  static const uint8_t GSM              = 0x1;
  static const uint8_t ANSI136_TDMA     = 0x2;
  static const uint8_t IS95_CDMA         = 0x3;
  static const uint8_t PDC              = 0x4;
  static const uint8_t PHS              = 0x5;
  static const uint8_t IDEN             = 0x6;
  static const uint8_t AMPS             = 0x7;
  static const uint8_t PAGING_NETWORK   = 0x8;
}

namespace BearerTypeValue{ /* SMPP v3.4 (5.3.2.5) */
  static const uint8_t UNKNOWN          = 0;
  static const uint8_t SMS              = 0x1;
  static const uint8_t CSD              = 0x2;
  static const uint8_t PACKET_DATA      = 0x3;
  static const uint8_t USSD             = 0x4;
  static const uint8_t CDPD             = 0x5;
  static const uint8_t DATA_TAC         = 0x6;
  static const uint8_t FLEX             = 0x7;
  static const uint8_t CELLCAST         = 0x8;
}

namespace PyloadTypeValue{ /* SMPP v3.4 (5.3.2.10) */
  static const uint8_t WDP  = 0;
  static const uint8_t WCMP = 1;
}

namespace SmppMessageState{
  static const uint8_t ENROUTE      =1;
  static const uint8_t DELIVERED    =2;
  static const uint8_t EXPIRED      =3;
  static const uint8_t DELETED      =4;
  static const uint8_t UNDELIVERABLE=5;
  static const uint8_t ACCEPTED     =6;
  static const uint8_t UNKNOWN      =7;
  static const uint8_t REJECTED     =8;
}

namespace MsValidityValue{ /* SMPP v3.4 (5.3.2.27) */
  static const uint8_t STORE_INDEFINITELY          = 0;
  static const uint8_t POWER_DOWN                  = 0x1;
  static const uint8_t SID_BASED_REGISTRATION_AREA = 0x2;
  static const uint8_t DISPLAY_ONLY                = 0x3;
}

namespace MsAvailabilityStatus{ /* SMPP v3.4 (5.3.2.30) */
  static const uint8_t AVAILABLE                   = 0;
  static const uint8_t DENIED                      = 0x1;
  static const uint8_t UNAVAILABLE                 = 0x2;
}

namespace UssdServiceOpValue{/* SMPP v3.4 (5.3.2.44) */
  static const int8_t PSSD_INDICATION        = 0;
  static const int8_t PSSR_INDICATION        = 1;
  static const int8_t USSR_REQUEST           = 2;
  static const int8_t USSN_REQUEST           = 3;
  static const int8_t PSSD_RESPONSE          = 16;
  static const int8_t PSSR_RESPONSE          = 17;
  static const int8_t USSR_CONFIRM           = 18;
  static const int8_t USSN_CONFIRM           = 19;
}

namespace NumberingPlanValue{ /* SMPP v3.4 (5.2.6) */
#define BIT(x) (((uint8_t)1)<<((uint8_t)x))
  static const uint8_t UNKNOWN              = 0;
  static const uint8_t ISDN                 = BIT(0);
  static const uint8_t DATA                 = BIT(1)|BIT(0);
  static const uint8_t TELEX                = BIT(2);
  static const uint8_t LAND_MOBILE          = BIT(2) | BIT(1);
  static const uint8_t NATIONAL             = BIT(3);
  static const uint8_t PRIVATE              = BIT(3) | BIT(0);
  static const uint8_t ERMES                = BIT(3) | BIT(1);
  static const uint8_t INTERNET             = BIT(3) | BIT(2) | BIT(1);
  static const uint8_t WAP                  = BIT(4) | BIT(1);
#undef BIT
}

namespace TypeOfNumberValue{ /* SMPP v3.4 (5.2.5) */
#define BIT(x) (((uint8_t)1)<<((uint8_t)x))
  static const uint8_t UNKNOWN              = 0;
  static const uint8_t INTERNATIONAL        = BIT(0);
  static const uint8_t NATIONAL             = BIT(1);
  static const uint8_t NETWORK_SPECIFIC     = BIT(1) | BIT(0);
  static const uint8_t SUBSCRIBER_NUMBER    = BIT(2);
  static const uint8_t ALPHANUMERIC         = BIT(2) | BIT(0);
  static const uint8_t ABBRIVIATED          = BIT(2) | BIT(1);
#undef BIT
}

namespace DataCoding{
#define BIT(x) (((uint8_t)1)<<((uint8_t)x))
  static const uint8_t SMSC7BIT             = 0;
  static const uint8_t LATIN1               = 3;
  static const uint8_t BINARY               = BIT(2);
  static const uint8_t UCS2                 = BIT(3);
#undef BIT
}

#define __int_property__(type,field) \
  type field;\
  inline void set_##field(type __value) {field = __value;} \
  inline type get_##field() {return field;}
#define __ref_property__(type,field) \
  type field;\
  inline void set_##field(const type& __value) {field = __value;} \
  inline type& get_##field() {return field;}
#define __ptr_property__(type,field) \
  type* field;\
  inline void set_##field(type* __value) {field = __value;} \
  inline type* get_##field() {return field;}
#define __ptr2_property__(type,field,counter) \
  type* field;\
  inline void set_##field(type* __value,unsigned count) {\
    if ( field && counter != 0 ) delete[] field; \
    field = new type[count]; \
    for ( unsigned i=0; i<count; ++i ) { field[i] = __value[i]; }\
    counter = count; } \
  inline type* get_##field() {return field;}
#define __intarr_property__(type,field,size) \
  type field[size];\
  inline void set_##field(type* __value) { __require__(__value!=NULL); memcpy(field,__value,size*sizeof(type)); } \
  inline const type* get_##field() { return field; }
#define __cstr_property__(field)\
  COStr field;\
  inline void set_##field(const char* __value) { __require__(__value!=NULL); field.copy(__value); } \
  inline const char* get_##field() { return field.cstr(); }
#define __ostr_property__(field)\
  OStr field;\
  inline void set_##field(const char* __value,int __len) { __require__(__value!=NULL); field.copy(__value,__len); } \
  inline const char* get_##field() { return field.cstr(); } \
  inline int size_##field() { return field.size(); }

template<class T> unsigned CalcSize(T* arr,unsigned count)
{
  unsigned sz = 0;
  for ( unsigned u = 0; u < count; ++u ) {
    sz+=arr[u].size();
  }
  return sz;
}

#define _s_int_property__(type,field) + (uint32_t)sizeof(type)
#define _s_ref_property__(type,field) + (uint32_t)field.size()
#define _s_ptr_property__(type,field,count) + (uint32_t)( field ? CalcSize(field,count) : 0 )
#define _s_intarr_property__(type,field,size) + (uint32_t)(sizeof(type)*size)
#define _s_cstr_property__(field) + (uint32_t)(field.size()+1)
#define _s_ostr_property__(field) + (uint32_t)(field.size())

#ifdef _WIN32

#define __LOG__ void*
#define dump_text
#define dump_text1
#define dump_uint
#define dump_int
#define dump_ostr
#define dump_cstr


#else
#define __LOG__ FILE*
template <class TLOG>
inline void fprintf_tab(TLOG* log, int align) { for( int i=0; i<align; ++i) fprintf(log,"  ");}
#define dump_text1(format) {fprintf_tab(log,align); fprintf(log,format"\n");}
#define dump_text(format,...) {fprintf_tab(log,align); fprintf(log,format"\n",__VA_ARGS__);}
#define dump_uint(field) dump_text("%s = %u",#field,(uint32_t)field);
#define dump_int(field) dump_text("%s = %u",#field,(int32_t)field);
#define dump_ostr(field){ dump_text("%s: length= %u", #field, (uint32_t)field.size()); \
    if ( field.cstr() ){ \
    for ( int i = 0; i < field.size() ; ++i ) fprintf(log,"%02x ",(unsigned char)field.cstr()[i]); fprintf(log,"\n");}}
#define dump_cstr(field) { if ( field.cstr() ) {dump_text("%s = %s",#field,field.cstr());} \
  else {dump_text("%s = NULL",#field);} }
#endif

struct SmppHeader //: public MemoryManagerUnit
{
  __int_property__(uint32_t,commandLength)
  __int_property__(uint32_t,commandId)
  __int_property__(uint32_t,commandStatus)
  __int_property__(uint32_t,sequenceNumber)
  inline uint32_t size() { return 4*4;}
  SmppHeader() :
    commandLength(0),
    commandId(0),
    commandStatus(0),
    sequenceNumber(0) {}
  template <class TLOG>
  inline void dump( TLOG* log,int align)
  {
    dump_text1("SmppHeader{");
    ++align;
    dump_uint(commandLength);
    dump_uint(commandId);
    dump_uint(commandStatus);
    dump_uint(sequenceNumber);
    --align;
    dump_text1("}");
  }
};

struct PduAddress //: public MemoryManagerUnit
{
  __int_property__(uint8_t,typeOfNumber)
  __int_property__(uint8_t,numberingPlan)
  __cstr_property__(value)
  PduAddress() :
    typeOfNumber(0),
    numberingPlan(0) {};
  inline uint32_t size()
  {
    return (uint32_t)(0
                   _s_int_property__(uint8_t,typeOfNumber)
                   _s_int_property__(uint8_t,numberingPlan)
                   _s_cstr_property__(value));
  }
  template <class TLOG >
  inline void dump( TLOG* log,int align)
  {
    dump_text1("PduAddress{");
    ++align;
    dump_uint(typeOfNumber);
    dump_uint(numberingPlan);
    dump_cstr(value);
    --align;
    dump_text1("} //PduAddress");
  }
};

struct PduDestAddress : public PduAddress
{
  __int_property__(uint8_t,flag)
  PduDestAddress() : flag(0){};
  inline uint32_t size() {
    switch ( flag ) {
    case 0x01:
      return PduAddress::size() + 1;
    case 0x02:
      return PduAddress::size() + 1 - 2;
    default:
      throw runtime_error("invalid PduDestAddress flag");
    }
  }
  template <class TLOG>
  inline void dump( TLOG* log,int align)
  {
    dump_text1("PduDestAddress{");
    ++align;
    dump_uint(flag);
    if ( flag == 0x1 )
    {
      dump_uint(typeOfNumber);
      dump_uint(numberingPlan);
    }
    if ( flag == 0x02 || flag == 0x02)
    {
      dump_cstr(value);
    }
    --align;
    dump_text1("} //PduDestAddress");
  }
};

struct SmppOptional //: public MemoryManagerUnit
{
  SmppOptional() : field_present(0) {}
  uint64_t field_present;
#define _o_int_property__(type,field) \
  type field;\
  inline bool has_##field(){ return (field_present & SmppOptionalFields::field)!=0; } \
  inline void set_##field(type value) {field_present |= SmppOptionalFields::field; field = value; } \
  inline type get_##field() { __ret0_if_fail__(has_##field()); return field; }
#define _o_intarr_property__(type,field,size)\
  type field[size];\
  inline bool has_##field(){ return (field_present & SmppOptionalFields::field)!=0; } \
  inline void set_##field(type* value) { __require__(value!=NULL); field_present |= SmppOptionalFields::field; memcpy(field,value,size); } \
  inline const type* get_##field() { __ret0_if_fail__(has_##field()); return field; }
#define _o_cstr_property__(field)\
  COStr field;\
  inline bool has_##field(){ return (field_present & SmppOptionalFields::field)!=0 && (field.cstr() != 0 ); } \
  inline void set_##field(const char* value) { __require__(value!=NULL); field_present |= SmppOptionalFields::field; field.copy(value); } \
  inline const char* get_##field() { __ret0_if_fail__(has_##field()); return field.cstr(); }
#define _o_ostr_property__(field)\
  OStr field;\
  inline bool has_##field(){ return (field_present & SmppOptionalFields::field)!=0 && (field.cstr()!=0); } \
  inline void set_##field(const char* value,int len) { __require__(value!=NULL); field_present |= SmppOptionalFields::field; field.copy(value,len); } \
  inline const char* get_##field() { __ret0_if_fail__(has_##field()); return field.cstr(); } \
  inline int size_##field() { __ret0_if_fail__(has_##field()); return field.size(); }

  _o_int_property__(uint8_t,destAddrSubunit)
  _o_int_property__(uint8_t,sourceAddrSubunit)
  _o_int_property__(uint8_t,destNetworkType)
  _o_int_property__(uint8_t,sourceNetworkType)
  _o_int_property__(uint8_t,destBearerType)
  _o_int_property__(uint8_t,sourceBearerType)
  _o_int_property__(uint16_t,destTelematicsId)
  _o_int_property__(uint8_t,sourceTelematicsId)
  _o_int_property__(uint32_t,qosTimeToLive)
  _o_int_property__(uint8_t,payloadType)
  _o_cstr_property__(additionalStatusInfoText)
  _o_cstr_property__(receiptedMessageId)
  _o_int_property__(uint8_t,msMsgWaitFacilities)
  _o_int_property__(uint8_t,privacyIndicator)
  _o_ostr_property__(sourceSubaddress)
  _o_ostr_property__(destSubaddress)
  _o_int_property__(uint16_t,userMessageReference)
  _o_int_property__(uint8_t,userResponseCode)
  _o_int_property__(uint8_t,languageIndicator)
  _o_int_property__(uint16_t,sourcePort)
  _o_int_property__(uint16_t,destinationPort)
  _o_int_property__(uint16_t,sarMsgRefNum)
  _o_int_property__(uint8_t,sarTotalSegments)
  _o_int_property__(uint8_t,sarSegmentSegnum)
  _o_int_property__(uint8_t,scInterfaceVersion)
  _o_int_property__(uint8_t,displayTime)
  _o_int_property__(uint8_t,msValidity)
  _o_int_property__(uint8_t,dpfResult)
  _o_int_property__(uint8_t,setDpf)
  _o_int_property__(uint8_t,msAvailableStatus)
  _o_intarr_property__(uint8_t,networkErrorCode,3)
  _o_ostr_property__(messagePayload)
  _o_int_property__(uint8_t,deliveryFailureReason)
  _o_int_property__(uint8_t,moreMessagesToSend)
  _o_int_property__(uint8_t,messageState)
  _o_ostr_property__(callbackNum)
  _o_int_property__(uint8_t,callbackNumPresInd)
  _o_ostr_property__(callbackNumAtag)
  _o_int_property__(uint8_t,numberOfMessages)
  _o_int_property__(uint16_t,smsSignal)
  _o_int_property__(uint8_t,itsReplyType)
  _o_intarr_property__(uint8_t,itsSessionInfo,2)
  _o_int_property__(uint8_t,ussdServiceOp)
  _o_int_property__(bool,alertOnMessageDelivery)

  /* SMSC_specific  */

  _o_cstr_property__(supported_locale)
  _o_cstr_property__(imsi_address)
  _o_cstr_property__(msc_address)
  _o_int_property__(uint16_t,supported_codeset)

  _o_ostr_property__(unknownFields)
  _o_int_property__(uint8_t,protocol_id)

  _o_cstr_property__(sccp_oa)
  _o_cstr_property__(sccp_da)

  _o_int_property__(uint32_t,ussd_session_id)
  _o_cstr_property__(imsi)
  _o_int_property__(uint8_t,hlr_address_ton)
  _o_int_property__(uint8_t,hlr_address_npi)
  _o_cstr_property__(hlr_address)
  _o_int_property__(uint8_t,vlr_number_ton)
  _o_int_property__(uint8_t,vlr_number_npi)
  _o_cstr_property__(vlr_number)


#undef _o_int_property__
#undef _o_intarr_property__
#undef _o_cstr_property__
#undef _o_ostr_property__

#define _o_int_property__(type,field) + (uint32_t)( has_##field()  ?  sizeof(type)+4 : 0 )
#define _o_intarr_property__(type,field,size) + (uint32_t)( has_##field()  ?  sizeof(type)*size+4 : 0 )
#define _o_cstr_property__(field) + (uint32_t)( has_##field()  ?  field.size()+1+4  : 0 )
#define _o_ostr_property__(field) + (uint32_t)( has_##field()  ?  field.size()+4  : 0 )
  inline uint32_t size() {
  return (uint32_t) (
  0
  _o_int_property__(uint8_t,destAddrSubunit)
  _o_int_property__(uint8_t,sourceAddrSubunit)
  _o_int_property__(uint8_t,destNetworkType)
  _o_int_property__(uint8_t,sourceNetworkType)
  _o_int_property__(uint8_t,destBearerType)
  _o_int_property__(uint8_t,sourceBearerType)
  _o_int_property__(uint16_t,destTelematicsId)
  _o_int_property__(uint8_t,sourceTelematicsId)
  _o_int_property__(uint32_t,qosTimeToLive)
  _o_int_property__(uint8_t,payloadType)
  _o_cstr_property__(additionalStatusInfoText)
  _o_cstr_property__(receiptedMessageId)
  _o_int_property__(uint8_t,msMsgWaitFacilities)
  _o_int_property__(uint8_t,privacyIndicator)
  _o_ostr_property__(sourceSubaddress)
  _o_ostr_property__(destSubaddress)
  _o_int_property__(uint16_t,userMessageReference)
  _o_int_property__(uint8_t,userResponseCode)
  _o_int_property__(uint8_t,languageIndicator)
  _o_int_property__(uint16_t,sourcePort)
  _o_int_property__(uint16_t,destinationPort)
  _o_int_property__(uint16_t,sarMsgRefNum)
  _o_int_property__(uint8_t,sarTotalSegments)
  _o_int_property__(uint8_t,sarSegmentSegnum)
  _o_int_property__(uint8_t,scInterfaceVersion)
  _o_int_property__(uint8_t,displayTime)
  _o_int_property__(uint8_t,msValidity)
  _o_int_property__(uint8_t,dpfResult)
  _o_int_property__(uint8_t,setDpf)
  _o_int_property__(uint8_t,msAvailableStatus)
  _o_intarr_property__(uint8_t,networkErrorCode,3)
  _o_ostr_property__(messagePayload)
  _o_int_property__(uint8_t,deliveryFailureReason)
  _o_int_property__(uint8_t,moreMessagesToSend)
  _o_int_property__(uint8_t,messageState)
  _o_ostr_property__(callbackNum)
  _o_int_property__(uint8_t,callbackNumPresInd)
  _o_ostr_property__(callbackNumAtag)
  _o_int_property__(uint8_t,numberOfMessages)
  _o_int_property__(uint16_t,smsSignal)
  _o_int_property__(uint8_t,itsReplyType)
  _o_intarr_property__(uint8_t,itsSessionInfo,2)
  _o_int_property__(uint8_t,ussdServiceOp)
  _o_int_property__(bool,alertOnMessageDelivery)

  /* SMSC_specific  */

  _o_cstr_property__(supported_locale)
  _o_cstr_property__(imsi_address)
  _o_cstr_property__(msc_address)
  _o_int_property__(uint16_t,supported_codeset)
  _o_int_property__(uint8_t,protocol_id)
  _o_cstr_property__(sccp_oa)
  _o_cstr_property__(sccp_da)
  _o_int_property__(uint32_t,ussd_session_id)
  _o_cstr_property__(imsi)
  _o_int_property__(uint8_t,hlr_address_ton)
  _o_int_property__(uint8_t,hlr_address_npi)
  _o_cstr_property__(hlr_address)
  _o_int_property__(uint8_t,vlr_number_ton)
  _o_int_property__(uint8_t,vlr_number_npi)
  _o_cstr_property__(vlr_number)

  + (uint32_t)( has_unknownFields()  ?  unknownFields.size()  : 0 )


  ); }

#undef _o_int_property__
#undef _o_intarr_property__
#undef _o_cstr_property__
#undef _o_ostr_property__

#define _o_int_property__(type,field) if ( has_##field() ) {dump_uint(field);}
#define _o_intarr_property__(type,field,size) if ( has_##field() ) {for ( int i=0; i < size; ++i) {dump_uint(field[i]);}}
#define _o_cstr_property__(field) if ( has_##field() ) { dump_cstr(field); }
#define _o_ostr_property__(field) if ( has_##field() ) { dump_ostr(field); }
  template <class TLOG>
  inline void dump( TLOG* log,int align)
  {
    dump_text1("SmppOptional{");
    ++align;
    _o_int_property__(uint8_t,destAddrSubunit)
    _o_int_property__(uint8_t,sourceAddrSubunit)
    _o_int_property__(uint8_t,destNetworkType)
    _o_int_property__(uint8_t,sourceNetworkType)
    _o_int_property__(uint8_t,destBearerType)
    _o_int_property__(uint8_t,sourceBearerType)
    _o_int_property__(uint16_t,destTelematicsId)
    _o_int_property__(uint8_t,sourceTelematicsId)
    _o_int_property__(uint32_t,qosTimeToLive)
    _o_int_property__(uint8_t,payloadType)
    _o_cstr_property__(additionalStatusInfoText)
    _o_cstr_property__(receiptedMessageId)
    _o_int_property__(uint8_t,msMsgWaitFacilities)
    _o_int_property__(uint8_t,privacyIndicator)
    _o_ostr_property__(sourceSubaddress)
    _o_ostr_property__(destSubaddress)
    _o_int_property__(uint16_t,userMessageReference)
    _o_int_property__(uint8_t,userResponseCode)
    _o_int_property__(uint8_t,languageIndicator)
    _o_int_property__(uint16_t,sourcePort)
    _o_int_property__(uint16_t,destinationPort)
    _o_int_property__(uint16_t,sarMsgRefNum)
    _o_int_property__(uint8_t,sarTotalSegments)
    _o_int_property__(uint8_t,sarSegmentSegnum)
    _o_int_property__(uint8_t,scInterfaceVersion)
    _o_int_property__(uint8_t,displayTime)
    _o_int_property__(uint8_t,msValidity)
    _o_int_property__(uint8_t,dpfResult)
    _o_int_property__(uint8_t,setDpf)
    _o_int_property__(uint8_t,msAvailableStatus)
    _o_intarr_property__(uint8_t,networkErrorCode,3)
    _o_ostr_property__(messagePayload)
    _o_int_property__(uint8_t,deliveryFailureReason)
    _o_int_property__(uint8_t,moreMessagesToSend)
    _o_int_property__(uint8_t,messageState)
    _o_ostr_property__(callbackNum)
    _o_int_property__(uint8_t,callbackNumPresInd)
    _o_ostr_property__(callbackNumAtag)
    _o_int_property__(uint8_t,numberOfMessages)
    _o_int_property__(uint16_t,smsSignal)
    _o_int_property__(uint8_t,itsReplyType)
    _o_intarr_property__(uint8_t,itsSessionInfo,2)
    _o_int_property__(uint8_t,ussdServiceOp)
    _o_int_property__(bool,alertOnMessageDelivery)

  /* SMSC_specific  */

    _o_cstr_property__(supported_locale)
    _o_cstr_property__(imsi_address)
    _o_cstr_property__(msc_address)
    _o_int_property__(uint16_t,supported_codeset)

    _o_ostr_property__(unknownFields)
    _o_int_property__(uint8_t,protocol_id)

    _o_int_property__(uint32_t,ussd_session_id)
    _o_cstr_property__(imsi)
    _o_int_property__(uint8_t,hlr_address_ton)
    _o_int_property__(uint8_t,hlr_address_npi)
    _o_cstr_property__(hlr_address)
    _o_int_property__(uint8_t,vlr_number_ton)
    _o_int_property__(uint8_t,vlr_number_npi)
    _o_cstr_property__(vlr_number)

    --align;
    dump_text1("} //SmppOptional");
  }
#undef _o_int_property__
#undef _o_intarr_property__
#undef _o_cstr_property__
#undef _o_ostr_property__
};


struct PduOutBind //: public SmppHeader//MemoryManagerUnit
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(systemId)
  __cstr_property__(password)
  inline uint32_t size()
  {
    return (uint32_t)(0 //SmppHeader::size()
                      _s_ref_property__(SmppHeader,header)
                      _s_cstr_property__(systemId)
                      _s_cstr_property__(password));
  }
  template <class TLOG>
  inline void dump( TLOG* log,int align = 0)
  {
    dump_text1("SmppOutBind{");
    header.dump(log,align+1);
    ++align;
    dump_cstr(systemId);
    dump_cstr(password);
    --align;
    dump_text1("} //SmppOutBind");
  }

};

struct PduWithOnlyHeader //: public SmppHeader//MemoryManagerUnit
{
  __ref_property__(SmppHeader,header);
  inline uint32_t size() { return _s_ref_property__(SmppHeader,header); }
  template <class TLOG>
  inline void dump( TLOG* log,int align = 0)
  {
    dump_text1("PduWithOnlyHeader{");
    header.dump(log,align+1);
    dump_text1("} //PduWithOnlyHeader");
  }
};

typedef PduWithOnlyHeader PduUnbind;
typedef PduWithOnlyHeader PduUnbindResp;
typedef PduWithOnlyHeader PduGenericNack;
typedef PduWithOnlyHeader PduEnquireLink;
typedef PduWithOnlyHeader PduEnquireLinkResp;
typedef PduWithOnlyHeader PduReplaceSmResp;
typedef PduWithOnlyHeader PduCancelSmResp;

struct PduPartSm //: public MemoryManagerUnit
{
  __cstr_property__(serviceType)
  __ref_property__(PduAddress,source)
  __ref_property__(PduAddress,dest)
  __int_property__(uint8_t,numberOfDests)
  __ptr2_property__(PduDestAddress,dests,numberOfDests)
  __int_property__(uint8_t,esmClass)
  __int_property__(uint8_t,protocolId)
  __int_property__(uint8_t,priorityFlag)
  __cstr_property__(scheduleDeliveryTime)
  __cstr_property__(validityPeriod)
  __int_property__(uint8_t,registredDelivery)
  __int_property__(uint8_t,replaceIfPresentFlag)
  __int_property__(uint8_t,dataCoding)
  __int_property__(uint8_t,smDefaultMsgId)
  //__int_property__(uint8_t,smLength)
  //__ostr_property__(shortMessage)
  OStr shortMessage;
  inline void set_shortMessage(const char* __value,int __len)
  {
    //__require__(__value!=NULL);
    if ( !__value ) shortMessage.dispose();
    else shortMessage.copy(__value,__len);
  }
  inline const char* get_shortMessage() { return shortMessage.cstr(); }
  inline int size_shortMessage(){ return shortMessage.size(); }
  inline uint8_t get_smLength(){ return (uint8_t)shortMessage.size(); }
  ~PduPartSm()
  {
    //__trace2__("::%s:",__FUNCTION__);
    if (dests && numberOfDests ) delete[] dests;
    dests = 0;
  }
  PduPartSm() :
    numberOfDests(0),
    dests(0),
    esmClass(0),
    protocolId(0),
    priorityFlag(0),
    registredDelivery(0),
    replaceIfPresentFlag(0),
    dataCoding(0),
    smDefaultMsgId(0)
  {
    //__trace2__("::%s:",__FUNCTION__);
  }
  inline uint32_t size(bool multi)
  {
    return (uint32_t)(0 + 1 // sizeof(smLength)
                      _s_cstr_property__(serviceType)
                      _s_ref_property__(PduAddress,source)
                      + ( !multi ? 0 _s_ref_property__(PduAddress,dest) : 0 )
                      + ( multi ? 0 _s_int_property__(uint8_t,numberOfDests) : 0 )
                      + ( multi ? 0 _s_ptr_property__(PduDestAddress,dests,numberOfDests) : 0 )
                      _s_int_property__(uint8_t,esmClass)
                      _s_int_property__(uint8_t,protocolId)
                      _s_int_property__(uint8_t,priorityFlag)
                      _s_cstr_property__(scheduleDeliveryTime)
                      _s_cstr_property__(validityPeriod)
                      _s_int_property__(uint8_t,registredDelivery)
                      _s_int_property__(uint8_t,replaceIfPresentFlag)
                      _s_int_property__(uint8_t,dataCoding)
                      _s_int_property__(uint8_t,smDefaultMsgId)
                      _s_ostr_property__(shortMessage));
  }
  template <class TLOG>
  inline void dump( TLOG* log,int align = 0)
  {
    dump_text1("PduPartSm{");
    ++align;
    dump_cstr(serviceType);
    dump_text1("source = "); source.dump(log,align+1);
    dump_text1("dest = "); dest.dump(log,align+1);
    dump_uint(numberOfDests);
    //_s_ptr_property__(PduDestAddress,dests)
    dump_uint(esmClass);
    dump_uint(protocolId);
    dump_uint(priorityFlag);
    dump_cstr(scheduleDeliveryTime);
    dump_cstr(validityPeriod);
    dump_uint(registredDelivery);
    dump_uint(replaceIfPresentFlag);
    dump_uint(dataCoding);
    dump_uint(smDefaultMsgId);
    //dump_uint(get_smLength());
    dump_ostr(shortMessage);
    --align;
    dump_text1("} //PduPartSm");
  }
};

struct PduXSm //: public SmppHeader//public MemoryManagerUnit
{
  __ref_property__(SmppHeader,header)
  __ref_property__(PduPartSm,message)
  __ref_property__(SmppOptional,optional)
  inline uint32_t size( bool multi )
  {
    return (uint32_t)(0
                      _s_ref_property__(SmppHeader,header)
                      //_s_ref_property__(PduPartSm,message)
                      + message.size(multi)
                      _s_ref_property__(SmppOptional,optional)
                      );
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduXSm{");
    header.dump(log,align+1);
    message.dump(log,align+1);
    optional.dump(log,align+1);
    dump_text1("} //PduXSm");
  }
  PduXSm()
  {
    //__trace2__("::%s:",__FUNCTION__);
  }
  ~PduXSm()
  {
    //__trace2__("::%s:",__FUNCTION__);
  }
};

struct PduXSmResp //: public SmppHeader //MemoryManagerUnit
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(messageId)
  int ussdSessionId;
  bool haveUssdSessionId;
  void set_ussdSessionId(int val)
  {
    haveUssdSessionId=true;
    ussdSessionId=val;
  }
  int get_ussdSessionId()const
  {
    return ussdSessionId;
  }
  bool has_ussdSessionId()const
  {
    return haveUssdSessionId;
  }
  PduXSmResp():haveUssdSessionId(false){}
  inline uint32_t size()
  {
    return (header.get_commandStatus()==0 || header.get_commandId()!=SmppCommandSet::SUBMIT_SM_RESP?
       (uint32_t)(0 _s_ref_property__(SmppHeader,header)
                    _s_cstr_property__(messageId))
                    :(uint32_t)(0 _s_ref_property__(SmppHeader,header)))+(haveUssdSessionId?8:0);
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduXSmResp{");
    header.dump(log,align+1);
    ++align;
    dump_cstr(messageId);
    --align;
    dump_text1("} //PduXSm");
  }
};

typedef PduXSm  PduSubmitSm;
typedef PduXSm  PduDeliverySm;
typedef PduXSm  PduMultiSm;

typedef PduXSmResp  PduSubmitSmResp;
typedef PduXSmResp  PduDeliverySmResp;

struct PduDataSmResp //: public SmppHeader //MemoryManagerUnit
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(messageId)
  __ref_property__(SmppOptional,optional)
  inline uint32_t size()
  {
    return (uint32_t)(0
                      _s_ref_property__(SmppHeader,header)
                      _s_cstr_property__(messageId)
                      _s_ref_property__(SmppOptional,optional));

  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduXSmResp{");
    header.dump(log,align+1);
    ++align;
    dump_cstr(messageId);
    optional.dump(log,align);
    --align;
    dump_text1("} //PduXSm");
  }
};


struct UnsuccessDeliveries //: public MemoryManagerUnit
{
  __ref_property__(PduAddress,addr)
  __int_property__(uint32_t,errorStatusCode)
  inline uint32_t size()
  {
    return (uint32_t)(0
                      _s_ref_property__(PduAddress,addr)
                      _s_int_property__(uint32_t,errorStatusCode));
  }
};

struct PduMultiSmResp //: public SmppHeader//MemoryManagerUnit
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(messageId)
  __int_property__(uint8_t,noUnsuccess)
  __ptr2_property__(UnsuccessDeliveries,sme,noUnsuccess)
  inline uint32_t size()
  {
    return (uint32_t)(0
    _s_ref_property__(SmppHeader,header)
    _s_cstr_property__(messageId)
    _s_int_property__(uint8_t,noUnsuccess)
    _s_ptr_property__(UnsuccessDeliveries,sme,noUnsuccess));
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduMultiResp{");
    header.dump(log,align+1);
    ++align;
    dump_cstr(messageId);
    --align;
    dump_text1("} //PduMultiResp");
  }
  PduMultiSmResp() : noUnsuccess(0),sme(0) {}
  ~PduMultiSmResp()
  {
//    __trace2__("::%s:",__FUNCTION__);
    if (sme && noUnsuccess) delete[] sme;
    sme = 0;
  }
};



struct PduBindTRX //: public SmppHeader//MemoryManagerUnit
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(systemId)
  __cstr_property__(password)
  __cstr_property__(systemType)
  __int_property__(uint8_t,interfaceVersion)
  __ref_property__(PduAddress,addressRange)
  inline uint32_t size()
  {
    return (uint32_t)(0
                      _s_ref_property__(SmppHeader,header)
                      _s_cstr_property__(systemId)
                      _s_cstr_property__(password)
                      _s_cstr_property__(systemType)
                      _s_int_property__(uint8_t,interfaceVersion)
                      _s_ref_property__(PduAddress,addressRange));
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduBindTRX{");
    header.dump(log,align+1);
    ++align;
    dump_cstr(systemId);
    dump_cstr(password);
    dump_cstr(systemType);
    dump_uint(interfaceVersion);
    dump_text1("address_range = "); addressRange.dump(log,align+1);
    --align;
    dump_text1("} //PduBindTRX");
  }
};

struct PduBindTRXResp //: public SmppHeader//MemoryManagerUnit
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(systemId)
//optional
  __int_property__(int8_t,scInterfaceVersion)
  PduBindTRXResp() :
    scInterfaceVersion(0) {};
  inline uint32_t size()
  {
    return (uint32_t)(0
                      _s_ref_property__(SmppHeader,header)
                      _s_cstr_property__(systemId)
                    //optional
                      _s_int_property__(int8_t,scInterfaceVersion)+4);
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduBindTRXResp{");
    header.dump(log,align+1);
    ++align;
    dump_cstr(systemId);
    dump_uint(scInterfaceVersion);
    --align;
    dump_text1("} //PduBindTRXResp");
  }
};

struct PduDataPartSm //: public MemoryManagerUnit
{
  __cstr_property__(serviceType)
  __ref_property__(PduAddress,source)
  __ref_property__(PduAddress,dest)
  __int_property__(uint8_t,esmClass)
  __int_property__(uint8_t,registredDelivery)
  __int_property__(uint8_t,dataCoding)

  PduDataPartSm() :
    esmClass(0),
    registredDelivery(0),
    dataCoding(0) {}

  inline uint32_t size()
  {
    return (uint32_t)(0 /*+ 1 // sizeof(smLength)*/
                      _s_cstr_property__(serviceType)
                      _s_ref_property__(PduAddress,source)
                      _s_ref_property__(PduAddress,dest)
                      _s_int_property__(uint8_t,esmClass)
                      _s_int_property__(uint8_t,registredDelivery)
                      _s_int_property__(uint8_t,dataCoding));
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduPartSm{");
    ++align;
    dump_cstr(serviceType);
    dump_text1("source = "); source.dump(log,align+1);
    dump_text1("dest = "); dest.dump(log,align+1);
    //_s_ptr_property__(PduDestAddress,dests)
    dump_uint(esmClass);
    dump_uint(registredDelivery);
    dump_uint(dataCoding);
    --align;
    dump_text1("} //PduPartSm");
  }
};

struct PduDataSm
{
  __ref_property__(SmppHeader,header)
  __ref_property__(PduDataPartSm,data)
  __ref_property__(SmppOptional,optional)
  inline uint32_t size()
  {
    return (uint32_t)(0
                      _s_ref_property__(SmppHeader,header)
                      _s_ref_property__(PduDataPartSm,data)
                      _s_ref_property__(SmppOptional,optional));
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduDataSm{");
    header.dump(log,align+1);
    ++align;
    data.dump(log,align);
    optional.dump(log,align);
    --align;
    dump_text1("} //PduDataSm");
  }
};

struct PduQuerySm
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(messageId)
  __ref_property__(PduAddress,source)
  inline uint32_t size()
  {
    return (uint32_t)(0
                      _s_ref_property__(SmppHeader,header)
                      _s_cstr_property__(messageId)
                      _s_ref_property__(PduAddress,source));
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduQuerySm{");
    header.dump(log,align+1);
    ++align;
    dump_cstr(messageId);
    dump_text1("source = "); source.dump(log,align);
    --align;
    dump_text1("} //PduQuerySm");
  }
};

struct PduQuerySmResp
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(messageId)
  __cstr_property__(finalDate)
  __int_property__(uint8_t,messageState)
  __int_property__(uint8_t,errorCode)
  inline uint32_t size()
  {
    return (uint32_t)(0
                      _s_ref_property__(SmppHeader,header)
                      _s_cstr_property__(messageId)
                      _s_cstr_property__(finalDate)
                      _s_int_property__(uint8_t,messageState)
                      _s_int_property__(uint8_t,errorCode));
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduQuerySmResp{");
    header.dump(log,align+1);
    ++align;
    dump_cstr(messageId);
    dump_cstr(finalDate);
    dump_uint(messageState);
    dump_uint(errorCode);
    --align;
    dump_text1("} //PduQuerySmResp");
  }
};

struct PduCancelSm
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(serviceType)
  __cstr_property__(messageId)
  __ref_property__(PduAddress,source)
  __ref_property__(PduAddress,dest)

  inline uint32_t size()
  {
    return (uint32_t)(0
                      _s_ref_property__(SmppHeader,header)
                      _s_cstr_property__(serviceType)
                      _s_cstr_property__(messageId)
                      _s_ref_property__(PduAddress,source)
                      _s_ref_property__(PduAddress,dest));
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduCancelSm{");
    header.dump(log,align+1);
    ++align;
    --align;
    dump_text1("} //PduCancelSm");
  }
};

struct PduReplaceSm
{
  __ref_property__(SmppHeader,header)
  __cstr_property__(messageId)
  __ref_property__(PduAddress,source)
  __cstr_property__(scheduleDeliveryTime)
  __cstr_property__(validityPeriod)
  __int_property__(uint8_t,registredDelivery)
  __int_property__(uint8_t,smDefaultMsgId)
  OStr shortMessage;
  inline void set_shortMessage(const char* __value,int __len)
  {
    //__require__(__value!=NULL);
    if ( !__value ) shortMessage.dispose();
    else shortMessage.copy(__value,__len);
  }
  inline const char* get_shortMessage() { return shortMessage.cstr(); }
  inline int size_shortMessage(){ return shortMessage.size(); }
  inline uint8_t get_smLength(){ return (uint8_t)shortMessage.size(); }
  PduReplaceSm() :
    registredDelivery(0),
    smDefaultMsgId(0){}

  inline uint32_t size()
  {
    return (uint32_t)(0 + 1 // sizeof(smLength)
                    _s_ref_property__(SmppHeader,header)
                    _s_cstr_property__(messageId)
                    _s_ref_property__(PduAddress,source)
                    _s_cstr_property__(scheduleDeliveryTime)
                    _s_cstr_property__(validityPeriod)
                    _s_int_property__(uint8_t,registredDelivery)
                    _s_int_property__(uint8_t,smDefaultMsgId)
                    _s_ostr_property__(shortMessage));
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduReplaceSm{");
    header.dump(log,align+1);
    ++align;
    dump_cstr(messageId);
    dump_text1("source = "); source.dump(log,align+1);
    dump_cstr(scheduleDeliveryTime);
    dump_cstr(validityPeriod);
    dump_uint(registredDelivery);
    dump_uint(smDefaultMsgId);
    dump_ostr(shortMessage);
    --align;
    dump_text1("} //PduReplaceSm");
  }
};

struct PduAlertNotification
{
  __ref_property__(SmppHeader,header)
  __ref_property__(PduAddress,source)
  __ref_property__(PduAddress,esme)
  __ref_property__(SmppOptional,optional)
  inline uint32_t size()
  {
    return (uint32_t)(0
            _s_ref_property__(SmppHeader,header)
            _s_ref_property__(PduAddress,source)
            _s_ref_property__(PduAddress,esme)
            _s_ref_property__(SmppOptional,optional));
  }
  template <class TLOG>
  inline void dump(TLOG* log,int align = 0)
  {
    dump_text1("PduAlertNotification{");
    header.dump(log,align+1);
    ++align;
    source.dump(log,align);
    esme.dump(log,align);
    optional.dump(log,align);
    --align;
    dump_text1("} //PduAlertNotification");
  }
};

inline bool smppPduHasSms(SmppHeader* pdu)
{
  switch(pdu->commandId)
  {
  case SmppCommandSet::SUBMIT_SM:
  case SmppCommandSet::DELIVERY_SM:
  case SmppCommandSet::SUBMIT_MULTI:
  case SmppCommandSet::DATA_SM:
    return true;
  }
  return false;
}

#undef __int_property__
#undef __ref_property__
#undef __ptr_property__
#undef __intarr_property__
#undef __cstr_property__
#undef __ostr_property__

inline uint32_t calcSmppPacketLength(const SmppHeader* __pdu)
{
  using namespace SmppCommandSet;
  SmppHeader* _pdu = const_cast<SmppHeader*>(__pdu);
  switch ( _pdu->commandId )
  {
  case GENERIC_NACK:  return reinterpret_cast<PduGenericNack*>(_pdu)->size();
  case BIND_RECIEVER: return reinterpret_cast<PduBindTRX*>(_pdu)->size();
  case BIND_RECIEVER_RESP: return reinterpret_cast<PduBindTRXResp*>(_pdu)->size();
  case BIND_TRANSMITTER: return reinterpret_cast<PduBindTRX*>(_pdu)->size();
  case BIND_TRANSMITTER_RESP: return reinterpret_cast<PduBindTRXResp*>(_pdu)->size();
  case QUERY_SM: return reinterpret_cast<PduQuerySm*>(_pdu)->size();
  case QUERY_SM_RESP: return reinterpret_cast<PduQuerySmResp*>(_pdu)->size();
  case SUBMIT_SM: return reinterpret_cast<PduXSm*>(_pdu)->size(false);
  case SUBMIT_SM_RESP: return reinterpret_cast<PduSubmitSmResp*>(_pdu)->size();
  case DELIVERY_SM: return reinterpret_cast<PduXSm*>(_pdu)->size(false);
  case DELIVERY_SM_RESP: return reinterpret_cast<PduDeliverySmResp*>(_pdu)->size();
  case UNBIND: return reinterpret_cast<PduUnbind*>(_pdu)->size();
  case UNBIND_RESP: return reinterpret_cast<PduUnbindResp*>(_pdu)->size();
  case REPLACE_SM: return reinterpret_cast<PduReplaceSm*>(_pdu)->size();
  case REPLACE_SM_RESP: return reinterpret_cast<PduReplaceSmResp*>(_pdu)->size();
  case CANCEL_SM: return reinterpret_cast<PduCancelSm*>(_pdu)->size();
  case CANCEL_SM_RESP: return reinterpret_cast<PduCancelSmResp*>(_pdu)->size();
  case BIND_TRANCIEVER: return reinterpret_cast<PduBindTRX*>(_pdu)->size();
  case BIND_TRANCIEVER_RESP: return reinterpret_cast<PduBindTRXResp*>(_pdu)->size();
  case OUTBIND: return reinterpret_cast<PduOutBind*>(_pdu)->size();
  case ENQUIRE_LINK: return reinterpret_cast<PduEnquireLink*>(_pdu)->size();
  case ENQUIRE_LINK_RESP: return reinterpret_cast<PduEnquireLinkResp*>(_pdu)->size();
  case SUBMIT_MULTI: return reinterpret_cast<PduXSm*>(_pdu)->size(true);
  case SUBMIT_MULTI_RESP: return reinterpret_cast<PduMultiSmResp*>(_pdu)->size();
  case ALERT_NOTIFICATION: return reinterpret_cast<PduAlertNotification*>(_pdu)->size();
  case DATA_SM: return reinterpret_cast<PduDataSm*>(_pdu)->size();
  case DATA_SM_RESP: return reinterpret_cast<PduDataSmResp*>(_pdu)->size();
  default:
    __unreachable__("this code is not reachable");
  }
  return 0;
}

inline bool dump_pdu(SmppHeader* _pdu,FILE* log = stderr)
{
  using namespace SmppCommandSet;
  //uint32_t length = 4*4; // header
  _pdu->commandLength = calcSmppPacketLength(_pdu);
  switch ( _pdu->commandId )
  {
  case GENERIC_NACK:  reinterpret_cast<PduGenericNack*>(_pdu)->dump(log); break;
  case BIND_RECIEVER: reinterpret_cast<PduBindTRX*>(_pdu)->dump(log); break;
  case BIND_RECIEVER_RESP: reinterpret_cast<PduBindTRXResp*>(_pdu)->dump(log); break;
  case BIND_TRANSMITTER: reinterpret_cast<PduBindTRX*>(_pdu)->dump(log); break;
  case BIND_TRANSMITTER_RESP: reinterpret_cast<PduBindTRXResp*>(_pdu)->dump(log); break;
  case QUERY_SM: reinterpret_cast<PduQuerySm*>(_pdu)->dump(log); break;
  case QUERY_SM_RESP: reinterpret_cast<PduQuerySmResp*>(_pdu)->dump(log); break;
  case SUBMIT_SM: reinterpret_cast<PduSubmitSm*>(_pdu)->dump(log);  break;
  case SUBMIT_SM_RESP: reinterpret_cast<PduSubmitSmResp*>(_pdu)->dump(log);  break;
  case DELIVERY_SM: reinterpret_cast<PduDeliverySm*>(_pdu)->dump(log); break;
  case DELIVERY_SM_RESP: reinterpret_cast<PduDeliverySmResp*>(_pdu)->dump(log); break;
  case UNBIND: reinterpret_cast<PduUnbind*>(_pdu)->dump(log); break;
  case UNBIND_RESP: reinterpret_cast<PduUnbindResp*>(_pdu)->dump(log); break;
  case REPLACE_SM: reinterpret_cast<PduReplaceSm*>(_pdu)->dump(log); break;
  case REPLACE_SM_RESP: reinterpret_cast<PduReplaceSmResp*>(_pdu)->dump(log);break;
  case CANCEL_SM: reinterpret_cast<PduCancelSm*>(_pdu)->dump(log);break;
  case CANCEL_SM_RESP: reinterpret_cast<PduCancelSmResp*>(_pdu)->dump(log);break;
  case BIND_TRANCIEVER: reinterpret_cast<PduBindTRX*>(_pdu)->dump(log); break;
  case BIND_TRANCIEVER_RESP: reinterpret_cast<PduBindTRXResp*>(_pdu)->dump(log); break;
  case OUTBIND: reinterpret_cast<PduOutBind*>(_pdu)->dump(log); break;
  case ENQUIRE_LINK: reinterpret_cast<PduEnquireLink*>(_pdu)->dump(log); break;
  case ENQUIRE_LINK_RESP: reinterpret_cast<PduEnquireLinkResp*>(_pdu)->dump(log); break;
  case SUBMIT_MULTI: reinterpret_cast<PduMultiSm*>(_pdu)->dump(log); break;
  case SUBMIT_MULTI_RESP: reinterpret_cast<PduMultiSmResp*>(_pdu)->dump(log); break;
  case ALERT_NOTIFICATION: reinterpret_cast<PduAlertNotification*>(_pdu)->dump(log); break;
  case DATA_SM: reinterpret_cast<PduDataSm*>(_pdu)->dump(log); break;
  case DATA_SM_RESP: reinterpret_cast<PduDataSmResp*>(_pdu)->dump(log); break;
  default:
    __unreachable__("this code is not reachable");
  }
  return true;
}

static inline void disposePdu(SmppHeader* _pdu)
{
  using namespace SmppCommandSet;
  if ( !_pdu ) return;
  switch ( _pdu->commandId )
  {
  case GENERIC_NACK:
    delete(reinterpret_cast<PduGenericNack*>(_pdu)); break;
  case BIND_RECIEVER:
    delete(reinterpret_cast<PduBindTRX*>(_pdu)); break;
  case BIND_RECIEVER_RESP:
    delete(reinterpret_cast<PduBindTRXResp*>(_pdu)); break;
  case BIND_TRANSMITTER:
    delete(reinterpret_cast<PduBindTRX*>(_pdu)); break;
  case BIND_TRANSMITTER_RESP:
    delete(reinterpret_cast<PduBindTRXResp*>(_pdu)); break;
  case QUERY_SM:
    delete(reinterpret_cast<PduQuerySm*>(_pdu)); break;
  case QUERY_SM_RESP:
    delete(reinterpret_cast<PduQuerySmResp*>(_pdu));break;
  case SUBMIT_SM:
    delete(reinterpret_cast<PduSubmitSm*>(_pdu));  break;
  case SUBMIT_SM_RESP:
    delete(reinterpret_cast<PduSubmitSmResp*>(_pdu));  break;
  case DELIVERY_SM:
    delete(reinterpret_cast<PduDeliverySm*>(_pdu)); break;
  case DELIVERY_SM_RESP:
    delete(reinterpret_cast<PduDeliverySmResp*>(_pdu)); break;
  case UNBIND:
    delete(reinterpret_cast<PduUnbind*>(_pdu)); break;
  case UNBIND_RESP:
    delete(reinterpret_cast<PduUnbindResp*>(_pdu)); break;
  case REPLACE_SM:
    delete(reinterpret_cast<PduReplaceSm*>(_pdu)); break;
  case REPLACE_SM_RESP:
    delete(reinterpret_cast<PduReplaceSmResp*>(_pdu)); break;
  case CANCEL_SM:
    delete(reinterpret_cast<PduCancelSm*>(_pdu)); break;
  case CANCEL_SM_RESP:
    delete(reinterpret_cast<PduCancelSmResp*>(_pdu));break;
  case BIND_TRANCIEVER:
    delete(reinterpret_cast<PduBindTRX*>(_pdu)); break;
  case BIND_TRANCIEVER_RESP:
    delete(reinterpret_cast<PduBindTRXResp*>(_pdu)); break;
  case OUTBIND:
    delete(reinterpret_cast<PduOutBind*>(_pdu)); break;
  case ENQUIRE_LINK:
    delete(reinterpret_cast<PduEnquireLink*>(_pdu)); break;
  case ENQUIRE_LINK_RESP:
    delete(reinterpret_cast<PduEnquireLinkResp*>(_pdu)); break;
  case SUBMIT_MULTI:
    delete(reinterpret_cast<PduMultiSm*>(_pdu)); break;
  case SUBMIT_MULTI_RESP:
    delete(reinterpret_cast<PduMultiSmResp*>(_pdu)); break;
  case ALERT_NOTIFICATION:
    delete(reinterpret_cast<PduAlertNotification*>(_pdu)); break;
  case DATA_SM:
    delete(reinterpret_cast<PduDataSm*>(_pdu)); break;
  case DATA_SM_RESP:
    delete(reinterpret_cast<PduDataSmResp*>(_pdu)); break;
  default:
    delete _pdu;
    //__unreachable__("this code is not reachable");
  }
}

}
}
#endif
