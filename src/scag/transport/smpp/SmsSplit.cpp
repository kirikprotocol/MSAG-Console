#include "SmsSplit.h"

namespace scag { namespace transport { namespace smpp {

#define MAX_SMS_PART_LEN 132

bool isSliced(SMS& sms)
{
    return sms.hasIntProperty(smsc::sms::Tag::SMPP_SAR_MSG_REF_NUM) || (sms.getIntProperty(Tag::SMPP_ESM_CLASS) & 0x40);
}

uint32_t getPartsCount(SMS& orgSms)
{
  uint32_t datalen;
  orgSms.getBinProperty(orgSms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) ? 
      Tag::SMPP_MESSAGE_PAYLOAD : Tag::SMPP_SHORT_MESSAGE, &datalen);

  return datalen / MAX_SMS_PART_LEN + (datalen % MAX_SMS_PART_LEN ? 1 : 0);
}

uint32_t getNextSmsPart(SMS& orgSms, SMS& partSms, uint32_t refNum, uint32_t seq, bool udh)
{
  uint32_t maxlen = MAX_SMS_PART_LEN, datalen, sent = seq * maxlen;
  const char *data = orgSms.getBinProperty(orgSms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) ? 
      Tag::SMPP_MESSAGE_PAYLOAD : Tag::SMPP_SHORT_MESSAGE, &datalen) + sent;

  if(sent >= datalen) return 0;
     
  uint32_t piece = datalen - sent > maxlen ? maxlen : datalen - sent;
  uint32_t count = datalen / maxlen + (datalen % maxlen ? 1 : 0);

  partSms = orgSms;
  partSms.dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
  partSms.dropProperty(Tag::SMSC_RAW_PAYLOAD);  

  if(udh)
  {
      std::auto_ptr<char> buf(new char[piece + 7]);
      char* d = buf.get();
      d[0]=6;
      d[1]=8;
      d[2]=4;
      d[3]=refNum >> 8;
      d[4]=refNum & 0xff;
      d[5]=count;
      d[6]=++seq;
      memcpy(d + 7, data, piece);      
      partSms.setIntProperty(Tag::SMPP_ESM_CLASS, partSms.getIntProperty(Tag::SMPP_ESM_CLASS) | 0x40);
      partSms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, d, piece + 7);
      partSms.setIntProperty(Tag::SMPP_SM_LENGTH, piece + 7);
  }
  else
  {
      partSms.setIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM, ++seq);
      partSms.setIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS, count);
      partSms.setIntProperty(Tag::SMPP_SAR_MSG_REF_NUM, refNum);
      partSms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, data, piece);
      partSms.setIntProperty(Tag::SMPP_SM_LENGTH, piece);  
  }

  return seq;
}

#undef MAX_SMS_PART_LEN

}}}
