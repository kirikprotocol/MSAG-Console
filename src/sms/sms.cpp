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
		if ( i == Tag::SMPP_SHORT_MESSAGE ) continue;
		if ( i == Tag::SMPP_MESSAGE_PAYLOAD ) continue;
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
          len=prop.properties[i].bValue->length();
          __require__(offset+len<=length);
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
    __require__(tag<=SMS_LAST_TAG);
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
      default:__unreachable__("SMS decoding failure");
    }
  }
};


void Body::setBinProperty(int tag,const char* value, unsigned len)
{
  __require__((tag>>8)==SMS_BIN_TAG);
  tag&=0xff;
  __require__(tag<=SMS_LAST_TAG);
  if ( !HSNS_isEqual() && len != 0 ) {
    if ( tag == unType(Tag::SMPP_SHORT_MESSAGE) || tag == unType(Tag::SMSC_RAW_SHORTMESSAGE))
    {
      __trace2__(":SMS::Body::%s processing SHORT_MESSAGE",__FUNCTION__);
      dropProperty(Tag::SMPP_SHORT_MESSAGE);
      if ( tag == unType(Tag::SMPP_SHORT_MESSAGE) ){
        if ( !prop.properties[unType(Tag::SMPP_DATA_CODING)].isSet )
          throw runtime_error(":SMS::MessageBody::setBinProperty: encoding scheme must be set");
        if ( !prop.properties[unType(Tag::SMPP_ESM_CLASS)].isSet )
          throw runtime_error(":SMS::MessageBody::getBinProperty: ems_class must be set");
        unsigned encoding = prop.properties[unType(Tag::SMPP_DATA_CODING)].getInt();
        if ( encoding != 0x8 ) goto trivial;
        auto_ptr<char> buffer(new char[len]);
        UCS_htons(buffer.get(),value,len,prop.properties[unType(Tag::SMPP_ESM_CLASS)].getInt());
        prop.properties[unType(Tag::SMSC_RAW_SHORTMESSAGE)].setBin(buffer.get(),len);
      }else{
        prop.properties[unType(Tag::SMSC_RAW_SHORTMESSAGE)].setBin(value,len);
      }
    }
    else if ( tag == unType(Tag::SMPP_MESSAGE_PAYLOAD) || tag == unType(Tag::SMSC_RAW_PAYLOAD))
    {
      __trace2__(":SMS::Body::%s processing SHORT_MESSAGE",__FUNCTION__);
      dropProperty(Tag::SMPP_MESSAGE_PAYLOAD);
      if ( tag == unType(Tag::SMPP_MESSAGE_PAYLOAD) ){
        if ( !prop.properties[unType(Tag::SMPP_DATA_CODING)].isSet )
          throw runtime_error(":SMS::MessageBody::setBinProperty: encoding scheme must be set");
        if ( !prop.properties[unType(Tag::SMPP_ESM_CLASS)].isSet )
          throw runtime_error(":SMS::MessageBody::getBinProperty: ems_class must be set");
        unsigned encoding = prop.properties[unType(Tag::SMPP_DATA_CODING)].getInt();
        if ( encoding != 0x8 ) goto trivial;
        auto_ptr<char> buffer(new char[len]);
        UCS_htons(buffer.get(),value,len,prop.properties[unType(Tag::SMPP_ESM_CLASS)].getInt());
        prop.properties[unType(Tag::SMSC_RAW_PAYLOAD)].setBin(buffer.get(),len);
      }else{
        prop.properties[unType(Tag::SMSC_RAW_PAYLOAD)].setBin(value,len);
      }
    }else
      prop.properties[tag].setBin(value,len);
  }else{
trivial:
    __trace2__(":SMS::Body::%s set trivial value",__FUNCTION__);
    if ( tag == unType(Tag::SMPP_SHORT_MESSAGE) ) tag = unType(Tag::SMSC_RAW_SHORTMESSAGE);
    if ( tag == unType(Tag::SMPP_MESSAGE_PAYLOAD) ) tag = unType(Tag::SMSC_RAW_PAYLOAD);
    prop.properties[tag].setBin(value,len);
  }
}

const char* Body::getBinProperty(int tag,unsigned* len)const
{
  __require__((tag>>8)==SMS_BIN_TAG);
  tag&=0xff;
  __require__(tag<=SMS_LAST_TAG);
  if ( !HSNS_isEqual() ) {
    if ( tag == unType(Tag::SMPP_SHORT_MESSAGE) ) {
      __trace2__(":SMS::Body::%s processing SHORT_MESSAGE",__FUNCTION__);
      if ( !hasBinProperty(Tag::SMPP_SHORT_MESSAGE) )
      {
        if ( !prop.properties[unType(Tag::SMPP_DATA_CODING)].isSet )
          throw runtime_error(":SMS::MessageBody::getBinProperty: encoding scheme must be set");
        if ( !prop.properties[unType(Tag::SMPP_ESM_CLASS)].isSet )
          throw runtime_error(":SMS::MessageBody::getBinProperty: ems_class must be set");
        unsigned encoding = prop.properties[unType(Tag::SMPP_DATA_CODING)].getInt();
        if ( encoding != 0x8 ) goto trivial;
        auto_ptr<char> buffer;
        unsigned len;
        const char* orig = prop.properties[unType(Tag::SMSC_RAW_SHORTMESSAGE)].getBin(&len);
        if ( len > 0 ){
          buffer = auto_ptr<char>(new char[len]);
          UCS_ntohs(buffer.get(),orig,len,prop.properties[unType(Tag::SMPP_ESM_CLASS)].getInt());
          const_cast<Body*>(this)->setBinProperty(Tag::SMPP_SHORT_MESSAGE,buffer.get(),len);
        }else{
          const_cast<Body*>(this)->setBinProperty(Tag::SMPP_SHORT_MESSAGE,"",len);
        }
      }
    }
    else if ( tag == unType(Tag::SMPP_MESSAGE_PAYLOAD) ){
      __trace2__(":SMS::Body::%s processing MESSAGE_PAYLOAD",__FUNCTION__);
      if ( !hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) )
      {
        if ( !prop.properties[unType(Tag::SMPP_DATA_CODING)].isSet )
          throw runtime_error(":SMS::MessageBody::getBinProperty: encoding scheme must be set");
        if ( !prop.properties[unType(Tag::SMPP_ESM_CLASS)].isSet )
          throw runtime_error(":SMS::MessageBody::getBinProperty: ems_class must be set");
        unsigned encoding = prop.properties[unType(Tag::SMPP_DATA_CODING)].getInt();
        if ( encoding != 0x8 ) goto trivial;
        auto_ptr<char> buffer;
        unsigned len;
        const char* orig = prop.properties[unType(Tag::SMSC_RAW_PAYLOAD)].getBin(&len);
        if ( len > 0 ){
          buffer = auto_ptr<char>(new char[len]);
          UCS_ntohs(buffer.get(),orig,len,prop.properties[unType(Tag::SMPP_ESM_CLASS)].getInt());
          const_cast<Body*>(this)->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,buffer.get(),len);
        }else{
          const_cast<Body*>(this)->setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD,"",len);
        }
      }
    }
  }else{
trivial:
    __trace2__(":SMS::Body::%s get trivial value",__FUNCTION__);
    if ( tag == unType(Tag::SMPP_SHORT_MESSAGE) )
	    tag = unType(Tag::SMSC_RAW_SHORTMESSAGE);
	  if ( tag == unType(Tag::SMPP_MESSAGE_PAYLOAD) )
	  	tag = unType(Tag::SMSC_RAW_PAYLOAD);
  }
  return prop.properties[tag].getBin(len);
}


};//sms
};//smsc
