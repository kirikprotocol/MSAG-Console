#include <stdlib.h>
#include <string.h>

#include "sms.h"

namespace smsc {
namespace sms{

int Body::getRequiredBufferSize() const
{
  int blength = 0;
  for(int i=0;i<=SMS_LAST_TAG;i++)
  {
    if(prop.properties[i].isSet)
    {
      switch(prop.properties[i].type)
      {
        case SMS_INT_TAG:
          blength+=6;
          break;
        case SMS_STR_TAG:
          blength+=prop.properties[i].sValue->length()+1+6;
          break;
        case SMS_BIN_TAG:
          blength+=prop.properties[i].bValue->length()+6;
          break;
      }
    }
  }
  return blength;
};


void Body::encode(uint8_t* buffer,int& length) const
{
  __require__( buffer != 0 );
  __require__( length >= 0 );
  __require__( length >= getRequiredBufferSize() );
  int offset=0;
  for(int i=0;i<=SMS_LAST_TAG;i++)
  {
    if(prop.properties[i].isSet)
    {
      __require__(offset<length);
      switch(prop.properties[i].type)
      {
        case SMS_INT_TAG:
        {
          uint16_t tag=htons(i);
          memcpy(buffer+offset,&tag,2);
          offset+=2;
          __require__(offset<length);
          uint32_t value=htonl(prop.properties[i].iValue);
          __require__(offset<length);
          memcpy(buffer+offset,&value,4);
          offset+=4;
        }break;
        case SMS_STR_TAG:
        {
          uint16_t tag=htons(i|(SMS_STR_TAG<<8));
          memcpy(buffer+offset,&tag,2);
          offset+=2;
          __require__(offset<length);
          uint32_t len=htonl(prop.properties[i].sValue->length()+1);
          memcpy(buffer+offset,&len,4);
          offset+=4;
          __require__(offset<length);
          len=prop.properties[i].sValue->length()+1;
          memcpy(buffer+offset,prop.properties[i].sValue->c_str(),len);
          offset+=len;
        }break;
        case SMS_BIN_TAG:
        {
          uint16_t tag=htons(i|(SMS_BIN_TAG<<8));
          memcpy(buffer+offset,&tag,2);
          offset+=2;
          __require__(offset<length);
          uint32_t len=htonl(prop.properties[i].bValue->length());
          memcpy(buffer+offset,&len,4);
          offset+=4;
          __require__(offset<length);
          len=prop.properties[i].bValue->length();
          memcpy(buffer+offset,prop.properties[i].bValue->c_str(),len);
          offset+=len;
        }break;
      }
    }
  }
};

void Body::decode(uint8_t* buffer,int length)
{
  //__require__( buffer != 0 );
  __require__( length >= 0 );
  int offset=0;
  while(offset<length)
  {
    uint16_t tag;
    memcpy(&tag,buffer+offset,2);
    offset+=2;
    tag=ntohs(tag);
    int type=tag>>8;
    tag&=0xff;
    switch(type)
    {
      case SMS_INT_TAG:
      {
        uint32_t value;
        memcpy(&value,buffer+offset,4);
        offset+=4;
        value=ntohl(value);
        prop.properties[tag].setInt(value);
      }break;
      case SMS_STR_TAG:
      {
        uint32_t len;
        memcpy(&len,buffer+offset,4);
        offset+=4;
        len=ntohl(len);
        __require__(buffer[offset+len-1]==0);
        __require__(offset+len<=length);
        prop.properties[tag].setStr((const char*)buffer+offset);
        offset+=len;
      }break;
      case SMS_BIN_TAG:
      {
        uint32_t len;
        memcpy(&len,buffer+offset,4);
        offset+=4;
        len=ntohl(len);
        __require__(offset+len<=length);
        prop.properties[tag].setBin((const char*)buffer+offset,len);
        offset+=len;
      }break;
    }
  }
};


};//sms
};//smsc
