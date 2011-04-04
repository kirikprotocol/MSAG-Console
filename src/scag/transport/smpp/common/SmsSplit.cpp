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

uint32_t getNextSmsPart(SMS& orgSms, SMS& partSms, uint32_t refNum, uint32_t seq, uint8_t udh, bool usePayload)
{
  uint32_t maxlen = MAX_SMS_PART_LEN, datalen, sent = seq * maxlen;
  const char *data = orgSms.getBinProperty(orgSms.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) ? 
      Tag::SMPP_MESSAGE_PAYLOAD : Tag::SMPP_SHORT_MESSAGE, &datalen) + sent;

  if(sent >= datalen) return 0;
     
  uint32_t piece = datalen - sent > maxlen ? maxlen : datalen - sent;
  uint32_t count = datalen / maxlen + (datalen % maxlen ? 1 : 0);

  partSms = orgSms;
  partSms.dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
  // partSms.dropProperty(Tag::SMSC_RAW_PAYLOAD);
  partSms.dropProperty(Tag::SMPP_SHORT_MESSAGE);
  // partSms.dropProperty(Tag::SMSC_RAW_SHORTMESSAGE);

  uint16_t field = usePayload ? Tag::SMPP_MESSAGE_PAYLOAD : Tag::SMPP_SHORT_MESSAGE;
  if(udh)
  {
      std::auto_ptr<char> buf(new char[piece + 5 + udh]);
      char* d = buf.get();
      if(udh == 1) // 8 bit
      {
          d[0] = 5;
          d[1] = 0;
          d[2] = 3;
          d[3] = refNum & 0xff;;
          d[4] = count;
          d[5] = ++seq;
      }
      else
      {
          d[0] = 6;
          d[1] = 8;
          d[2] = 4;
          d[3] = refNum >> 8;
          d[4] = refNum & 0xff;
          d[5] = count;
          d[6] = ++seq;
      }
      udh += 5;
      memcpy(d + udh, data, piece);
      partSms.setIntProperty(Tag::SMPP_ESM_CLASS, partSms.getIntProperty(Tag::SMPP_ESM_CLASS) | 0x40);
      partSms.setBinProperty(field, d, piece + udh);
  }
  else
  {
      partSms.setIntProperty(Tag::SMPP_SAR_SEGMENT_SEQNUM, ++seq);
      partSms.setIntProperty(Tag::SMPP_SAR_TOTAL_SEGMENTS, count);
      partSms.setIntProperty(Tag::SMPP_SAR_MSG_REF_NUM, refNum);
      partSms.setBinProperty(field, data, piece);
  }
  partSms.setIntProperty(Tag::SMPP_SM_LENGTH, usePayload ? 0 : piece + udh);

  return seq;
}

#undef MAX_SMS_PART_LEN

}}}
