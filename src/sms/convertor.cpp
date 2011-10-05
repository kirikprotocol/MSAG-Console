//$Id$

#include "sms.h"

#ifndef __FUNCTION__
#define __FUNCTION__ __func__
#endif

namespace smsc{
namespace sms {

template </*(short(*Conv)(short))*/ class ConvT>
static inline
void UCS_T(char* buffer, const char* orig, unsigned len, unsigned esm_class,ConvT Conv)
{
  bool udhi = (esm_class & 0x40) != 0;
  if ( !udhi )
  {
    if ( len%2 != 0 )
    {
      throw runtime_error(":SMS:UCS_<Conv>:Opps, it is UCS text but length%2 != 0");
    }
    for ( unsigned k = 0; k < len/2; ++k )
    {
      ((uint16_t*)buffer)[k] = Conv(((uint16_t*)orig)[k]);
      /*
      uint16_t val;
      memcpy(&val,orig+k*2,2);
      val=Conv(val);
      memcpy(buffer+k*2,&val,2);
      */
    }
  }
  else
  {
    unsigned udhLen = *(unsigned char*)orig;
    if (udhLen > len+1 )
    {
      throw runtime_error(":SMS:UCS_<Conv>: UDH length is great then total data length");
    }
    ++udhLen;
    // copy udh
    if(buffer!=orig)memcpy(buffer,orig,udhLen);
    len -= udhLen;
    buffer += udhLen;
    orig += udhLen;
    if ( len%2 != 0 )
    {
      throw runtime_error(":SMS:UCS_<Conv>:Opps, it is UCS text but length%2 != 0");
    }
    for ( unsigned k = 0; k < len/2; ++k )
    {
      ((uint16_t*)buffer)[k] = Conv(((uint16_t*)orig)[k]);
      /*
      uint16_t val;
      memcpy(&val,orig+k*2,2);
      val=Conv(val);
      memcpy(buffer+k*2,&val,2);
      */
    }
  }
}

static inline uint16_t MyHtoNs(uint16_t v) {return htons(v);}
static inline uint16_t MyNtoHs(uint16_t v) {return ntohs(v);}

void UCS_htons(char* buffer, const char* orig, unsigned len, unsigned esm_class)
{
//  __trace2__(":SMS:%s:",__FUNCTION__);
  UCS_T(buffer,orig,len,esm_class,MyHtoNs);
}
void UCS_ntohs(char* buffer, const char* orig, unsigned len, unsigned esm_class)
{
//  __trace2__(":SMS:%s:",__FUNCTION__);
  UCS_T(buffer,orig,len,esm_class,MyNtoHs);
}

}}
