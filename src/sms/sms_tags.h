#ifndef __SMSC_SMS_SMS_TAGS_H__
#define __SMSC_SMS_SMS_TAGS_H__

#ifndef SMSDEFTAG
#define SMSDEFTAG(type,idx,name) const int name=idx | ((type)<<8);
#endif

#define SMS_INT_TAG 0
#define SMS_STR_TAG 1
#define SMS_BIN_TAG 2

#define ISMSDEFTAG(idx,name) SMSDEFTAG(SMS_INT_TAG,idx,name)
#define SSMSDEFTAG(idx,name) SMSDEFTAG(SMS_STR_TAG,idx,name)
#define BSMSDEFTAG(idx,name) SMSDEFTAG(SMS_BIN_TAG,idx,name)

#undef SMS_BODY_TAGS_SET
#define SMS_BODY_TAGS_SET \
  ISMSDEFTAG(0,SMPP_SCHEDULE_DELIVERY_TIME)\
  ISMSDEFTAG(1,SMPP_REPLACE_IF_PRESENT_FLAG)\
  ISMSDEFTAG(2,SMPP_ESM_CLASS)\
  ISMSDEFTAG(3,SMPP_DATA_CODING)\
  ISMSDEFTAG(4,SMPP_SM_LENGTH)\
  ISMSDEFTAG(5,SMPP_REGISTRED_DELIVERY)\
  ISMSDEFTAG(6,SMPP_PROTOCOL_ID)\
  BSMSDEFTAG(7,SMPP_SHORT_MESSAGE)\
  ISMSDEFTAG(8,SMPP_PRIORITY)\
  ISMSDEFTAG(9,SMPP_USER_MESSAGE_REFERENCE)\
  ISMSDEFTAG(10,SMPP_USSD_SERVICE_OP)\
  ISMSDEFTAG(11,SMPP_DEST_ADDR_SUBUNIT)\
  ISMSDEFTAG(12,SMPP_PAYLOAD_TYPE)\
  SSMSDEFTAG(13,SMPP_RECEIPTED_MESSAGE_ID)\
  ISMSDEFTAG(14,SMPP_MS_MSG_WAIT_FACILITIES)\
  ISMSDEFTAG(15,SMPP_USER_RESPONSE_CODE)\
  ISMSDEFTAG(16,SMPP_SAR_MSG_REF_NUM)\
  ISMSDEFTAG(17,SMPP_LANGUAGE_INDICATOR)\
  ISMSDEFTAG(18,SMPP_SAR_TOTAL_SEGMENTS)\
  ISMSDEFTAG(19,SMPP_NUMBER_OF_MESSAGES)\
  BSMSDEFTAG(20,SMPP_MESSAGE_PAYLOAD)\
  ISMSDEFTAG(21,SMPP_DATA_SM) \
  ISMSDEFTAG(22,SMPP_MS_VALIDITY)\
  ISMSDEFTAG(23,SMPP_MSG_STATE) \
  ISMSDEFTAG(24,SMSC_DISCHARGE_TIME) \
  SSMSDEFTAG(25,SMSC_RECIPIENTADDRESS) \
  ISMSDEFTAG(26,SMSC_STATUS_REPORT_REQUEST) \
  BSMSDEFTAG(28,SMSC_RAW_SHORTMESSAGE) \
  BSMSDEFTAG(29,SMSC_RAW_PAYLOAD) \
  \
  ISMSDEFTAG(30,SMPP_SOURCE_PORT) \
  ISMSDEFTAG(31,SMPP_DESTINATION_PORT) \
  ISMSDEFTAG(32,SMPP_SAR_SEGMENT_SEQNUM) \
  ISMSDEFTAG(33,SMPP_MORE_MESSAGES_TO_SEND) \
  ISMSDEFTAG(34,SMPP_DEST_NETWORK_TYPE) \
  ISMSDEFTAG(35,SMPP_DEST_BEARER_TYPE) \
  ISMSDEFTAG(36,SMPP_QOS_TIME_TO_LIVE) \
  ISMSDEFTAG(37,SMPP_SET_DPF) \
  ISMSDEFTAG(38,SMPP_SOURCE_NETWORK_TYPE) \
  ISMSDEFTAG(39,SMPP_SOURCE_BEARER_TYPE)


#define SMS_LAST_TAG 39

#ifndef _TAGS_INTERNAL_
namespace smsc{
namespace sms{
namespace Tag{
extern int tagTypes[];
SMS_BODY_TAGS_SET
};
};
};
#endif

#endif
