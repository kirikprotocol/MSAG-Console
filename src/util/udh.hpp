#ifndef __SMSC_UTIL_UDH_HPP_
#define __SMSC_UTIL_UDH_HPP_

#include <inttypes.h>
#include "util/debug.h"

namespace smsc{
namespace util{

inline bool findConcatInfo(unsigned char* body,uint16_t& mr,uint8_t& idx,uint8_t& num,bool& havemoreudh)
{
  unsigned int udhLen=*body;
  unsigned int i=1;
  bool haveconcat=false;
  havemoreudh=false;
  while(i<udhLen)
  {
    if(body[i]==0) // 8 bit concat info
    {
      mr=body[i+2];
      num=body[i+3];
      idx=body[i+4];
      if(idx==0 || idx>num)
      {
        __warning2__("concat info seqnum=%u!!!",num);
        break;
      }
      haveconcat=true;
    }else if(body[i]==8) //16 concat info
    {
      mr=(body[i+2]<<8)|body[i+3];
      num=body[i+4];
      idx=body[i+5];
      if(idx==0 || idx>num)
      {
        __warning2__("concat info seqnum=%u!!!",num);
        break;
      }
      haveconcat=true;
    }
    else //something else
    {
      havemoreudh=true;
    }
    i=i+2+body[i+1];
  }
  return haveconcat;
}

}; //util
}; //smsc

#endif
