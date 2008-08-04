#include "SmppUtil.h"

namespace scag {
namespace transport {
namespace smpp {

void stripUnknownSmppOptionals(smsc::sms::SMS& sms,const std::vector<int>& allowedTags)
{
  if(!sms.hasBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS))return;
  unsigned optLen;
  const char* oldopt=sms.getBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS,&optLen);
  smsc::core::buffers::TmpBuf<char,1024> newopt(optLen);
  unsigned off=0;
  uint16_t tag,len;
  while(off<optLen)
  {
    memcpy(&tag,oldopt+off,2);tag=htons(tag);
    memcpy(&len,oldopt+off+2,2);len=htons(len);
    bool allow=false;
    for(std::vector<int>::const_iterator it=allowedTags.begin();it!=allowedTags.end();it++)
    {
      if(tag==*it)
      {
        allow=true;
        break;
      }
    }
    if(allow)
      newopt.Append(oldopt+off,len+2+2);
    off+=len+2+2;
  }

  if(newopt.GetPos()!=0)
    sms.setBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS,newopt.get(),(unsigned)newopt.GetPos());
  else
    sms.messageBody.dropProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS);
}

} // namespace smpp
} // namespace transport
} // namespace scag
