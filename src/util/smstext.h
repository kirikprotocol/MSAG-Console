#include "sms/sms.h"
#include "smpp/smpp_structures.h"
#include "util/recoder/recode_dll.h"


namespace smsc{
namespace util{

using namespace smsc::sms;
using namespace smsc::smpp;

static inline int getSmsText(SMS* sms,char* buf)
{
  int coding = sms->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING);
  int len = sms->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
  const char *data=sms->getStrProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE).c_str();
  if(coding==DataCoding::DEFAULT)
  {
    Convert7BitToText(data,len,buf,MAX_SHORT_MESSAGE_LENGTH);
    len=strlen(buf);
  }else if(coding==DataCoding::UCS2)
  {
    ConvertUCS2ToMultibyte((const short*)data,len,buf,MAX_SHORT_MESSAGE_LENGTH,CONV_ENCODING_ANSI);
    len/=2;
  }else
  {
    memcpy(buf,data,len);
  }
  buf[len]=0;
  return len;
}

};

};
