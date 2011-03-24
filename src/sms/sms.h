
#ifndef __SMSC_SMS_SMS_H__
#define __SMSC_SMS_SMS_H__

/**
*         SMS
*   SMS .   .
*
* @author Victor V. Makarov
* @version 1.0
* @see MessageStore
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <string>
#include <stdexcept>
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif
#include <memory>

#ifdef _WIN32
#ifndef _MSC_VER
#define __FUNCTION__ __FUNC__
#endif
#endif


#ifndef NOLOGGERPLEASE
#include <util/debug.h>
#else
#define __require__(x)
#endif
#include "sms/sms_const.h"
#include "sms/sms_tags.h"
#include "sms/sms_buf.h"
#include "core/buffers/FixedLengthString.hpp"
#include "util/Exception.hpp"
#include "util/Uint64Converter.h"
#include "core/buffers/TmpBuf.hpp"

namespace smsc {

#ifndef NOLOGGERPLEASE
namespace logger{
extern smsc::logger::Logger* _sms_err_cat;
}
#endif


namespace sms  {

extern void UCS_htons(char* buffer, const char* orig, unsigned len, unsigned esm_class);
extern void UCS_ntohs(char* buffer, const char* orig, unsigned len, unsigned esm_class);

using std::string;
using std::runtime_error;
using std::auto_ptr;

/**

*/
struct DistrList {
  char dl_name[21];
};

static inline bool HSNS_isEqual()
{
  static bool isEqual = (htons(0x1234) == 0x1234);
  return isEqual;
}

/**
*  Address
*   SMS .
*
* @author Victor V. Makarov
* @version 1.0
* @see SMS
*/
struct Address
{
  uint8_t      length, type, plan;
  AddressValue value;

  /**
  * Default ,
  */
  Address() : length(1), type(0), plan(0)
  {
    value[0] = '0'; value[1] = '\0';
  };

  /**
  *   Address,     .
  *
  *
  * @param _len     _value
  * @param _type
  * @param _plan
  * @param _value
  */
  Address(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
    : length(_len), type(_type), plan(_plan)
  {
    setValue(_len, _value);
  };

  /**
  *  ,
  *
  * @param addr    .
  */
  Address(const Address& addr)
    : length(addr.length), type(addr.type), plan(addr.plan)
  {
    setValue(addr.length, addr.value);
  };

  Address(const char* text)
  {
    if(!text || !*text)throw runtime_error("bad address NULL");
    AddressValue addr_value;
    int iplan,itype;
    memset(addr_value,0,sizeof(addr_value));
    int scanned = sscanf(text,".5.%d.%20s",
      &iplan,addr_value);
    if(scanned==2)
    {
      itype=5;
      scanned=1;
    }else
    {
      scanned = sscanf(text,".%d.%d.%20[0123456789?]s",&itype,&iplan,addr_value);
      if(scanned==3)
      {
        scanned=1;
      }
    }
    if ( scanned != 1 )
    {
      scanned = sscanf(text,"+%20[0123456789?]s",addr_value);
      if ( scanned )
      {
        iplan = 1;//ISDN
        itype = 1;//INTERNATIONAL
      }
      else
      {
        scanned = sscanf(text,"%20[0123456789?]s",addr_value);
        if ( !scanned )
          throw runtime_error(string("bad address ")+text);
        else
        {
          iplan = 1;//ISDN
          itype = 0;//UNKNOWN
        }
      }
    }
    plan = (uint8_t)iplan;
    type = (uint8_t)itype;
    length = (uint8_t)strlen(addr_value);
    if(length==0)
    {
      throw runtime_error(string("bad address ")+text);
    }
    memcpy(value,addr_value,sizeof(addr_value));
  }

  /**
  *   '=',
  *
  *
  * @param addr      '='
  * @return
  */
  Address& operator =(const Address& addr)
  {
    type = addr.type; plan = addr.plan;
    setValue(addr.length, addr.value);
    return (*this);
  };

  inline int operator ==(const Address& addr)const
  {
    return (length == addr.length &&
            plan == addr.plan && type == addr.type &&
            memcmp(value, addr.value, length) == 0);
  };

  inline int operator !=(const Address& addr)const
  {
    return !(*this==addr);
  };

  bool operator<(const Address& addr)const
  {
    return type<addr.type ||
           (type==addr.type && plan<addr.plan) ||
           (type==addr.type && plan==addr.plan && strcmp(value,addr.value)<0);
  }


  /**
  *       .
  *      MAX_ADDRESS_VALUE_LENGTH.
  *
  * @param _len
  * @param _value
  */
  inline void setValue(uint8_t _len, const char* _value)
  {
    __require__(_len && _value && _value[0]
      && _len<sizeof(AddressValue));

    memcpy(value, _value, _len*sizeof(uint8_t));
    value[length = _len] = '\0';
  };

  /**
  *
  *
  * @param _value
  *
  *               MAX_ADDRESS_VALUE_LENGTH+1,
  * @return
  */
  inline uint8_t getValue(char* _value) const
  {
    __require__(_value);

    if (length)
    {
      memcpy(_value, value, length*sizeof(uint8_t));
      _value[length] = '\0';
    }
    return length;
  }

  // depricated
  inline uint8_t getLenght() const
  {
    return getLength();
  };

  /**
  *
  *
  * @return
  */
  inline uint8_t getLength() const
  {
    return length;
  };

  /**
  *
  *
  * @param _type
  */
  inline void setTypeOfNumber(uint8_t _type)
  {
    type = _type;
  };

  /**
  *
  *
  * @param _type
  */
  inline uint8_t getTypeOfNumber() const
  {
    return type;
  };

  /**
  *
  *
  * @param _plan
  */
  inline void setNumberingPlan(uint8_t _plan)
  {
    plan = _plan;
  };

  /**
  *
  *
  * @return
  */
  inline uint8_t getNumberingPlan() const
  {
    return plan;
  };

  inline int getText(char* buf,size_t buflen)const
  {
    char vl[32];
    memcpy(vl,value,length);
    vl[length]=0;
    if ( type == 1 && plan == 1 ){
      return snprintf(buf,buflen,"+%s",vl);
    }else if (type == 0 && plan == 1){
      return snprintf(buf,buflen,"%s",vl);
    }else
      return snprintf(buf,buflen,".%d.%d.%s",type,plan,vl);
  }
  inline int toString(char* buf,size_t buflen)const{
    char vl[32];
    memcpy(vl,value,length);
    vl[length]=0;
    return snprintf(buf,buflen,".%d.%d.%s",type,plan,vl);
  }
  inline smsc::core::buffers::FixedLengthString<32> toString()const
  {
    if(length>32)abort();
    char vl[32];
    char buf[48];
    memcpy(vl,value,length);
    vl[length]=0;
    snprintf(buf,sizeof(buf),".%d.%d.%s",type,plan,vl);
    return buf;
  }
  void Clear()
  {
    value[0]=0;
    length=0;
    type=0;
    plan=0;
  }

};

/**
*  Descriptor
*
* (,     SMS)
*
* @author Victor V. Makarov
* @version 1.0
* @see SMS
*/
struct Descriptor
{
  uint8_t         mscLength, imsiLength;
  AddressValue    msc, imsi;
  uint32_t        sme;

  /**
  * Default ,
  */
  Descriptor() : mscLength(0), imsiLength(0), sme(0)
  {
    msc[0] = '\0'; imsi[0] = '\0';
  };

  /**
  *   Descriptor,     .
  *
  *
  * @param _mscLen     _msc
  * @param _value   MSC
  * @param _imsiLen     _imsi
  * @param _value   IMSI
  * @param _sme  SME
  */
  Descriptor(uint8_t _mscLen, const char* _msc,
    uint8_t _imsiLen, const char* _imsi, uint32_t _sme)
    : mscLength(_mscLen), imsiLength(_imsiLen), sme(_sme)
  {
    setMsc(mscLength, _msc);
    setImsi(imsiLength, _imsi);
  };

  /**
  *  ,
  *
  *
  * @param descr  .
  */
  Descriptor(const Descriptor& descr)
    : mscLength(descr.mscLength),
    imsiLength(descr.imsiLength), sme(descr.sme)
  {
    setMsc(descr.mscLength, descr.msc);
    setImsi(descr.imsiLength, descr.imsi);
  };

  /**
  *   '=',
  *
  *
  * @param descr      '='
  * @return
  */
  Descriptor& operator =(const Descriptor& descr)
  {
    sme = descr.sme;
    setMsc(descr.mscLength, descr.msc);
    setImsi(descr.imsiLength, descr.imsi);
    return (*this);
  };

  /**
  *     MSC   .
  *      MAX_ADDRESS_VALUE_LENGTH.
  *
  * @param _len      MSC
  * @param _value    MSC
  */
  inline void setMsc(uint8_t _len, const char* _value)
  {
    __require__( _len<sizeof(AddressValue) );

    if (_len && _value)
    {
      memcpy(msc, _value, _len*sizeof(uint8_t));
      msc[mscLength = _len] = '\0';
    }
    else
    {
      memset(msc, 0, sizeof(AddressValue));
      mscLength = 0;
    }
  };

  /**
  *     IMSI   .
  *      MAX_ADDRESS_VALUE_LENGTH.
  *
  * @param _len      IMSI
  * @param _value    IMSI
  */
  inline void setImsi(uint8_t _len, const char* _value)
  {
    __require__( _len<sizeof(AddressValue) );

    if (_len && _value)
    {
      memcpy(imsi, _value, _len*sizeof(uint8_t));
      imsi[imsiLength = _len] = '\0';
    }
    else
    {
      memset(imsi, 0, sizeof(AddressValue));
      imsiLength = 0;
    }
  };

  /**
  *     MSC
  *
  * @param _value
  *                MSC.
  *               MAX_ADDRESS_VALUE_LENGTH+1,
  * @return   MSC
  */
  inline uint8_t getMsc(char* _value) const
  {
    __require__(_value);

    if (mscLength)
    {
      memcpy(_value, msc, mscLength*sizeof(uint8_t));
      _value[mscLength] = '\0';
    }
    return mscLength;
  }

  /**
  *     IMSI
  *
  * @param _value
  *                IMSI.
  *               MAX_ADDRESS_VALUE_LENGTH+1,
  * @return   IMSI
  */
  inline uint8_t getImsi(char* _value) const
  {
    __require__(_value);

    if (imsiLength)
    {
      memcpy(_value, imsi, imsiLength*sizeof(uint8_t));
      _value[imsiLength] = '\0';
    }
    return imsiLength;
  }

  // depricated
  inline uint8_t getMscLenght() const
  {
    return getMscLength();
  };

  /**
  *    MSC
  *
  * @return   MSC
  */
  inline uint8_t getMscLength() const
  {
    return mscLength;
  };

  // depricated
  inline uint8_t getImsiLenght() const
  {
    return getImsiLength();
  };

  /**
  *    IMSI
  *
  * @return   IMSI
  */
  inline uint8_t getImsiLength() const
  {
    return imsiLength;
  };

  /**
  *   SME
  *
  * @return   IMSI
  */
  inline void setSmeNumber(uint32_t _sme)
  {
    sme = _sme;
  };

  /**
  *   SME
  *
  * @return  SME
  */
  inline uint32_t getSmeNumber() const
  {
    return sme;
  };

  void Clear()
  {
    mscLength=0;
    imsiLength=0;
    msc[0]=0;
    imsi[0]=0;
    sme=0;
  }
};


struct OptionalProperty{
  uint16_t isSetVal;
  uint16_t type;
  int iValue;
  SmsPropertyBuf xValue;

  static SmsPropertyBuf* nullStr;

  OptionalProperty():isSetVal(0),type(SMS_INT_TAG){}
  OptionalProperty(const OptionalProperty& prop):isSetVal(0),type(SMS_INT_TAG)
  {
    *this=prop;
  }
  bool isSet()const
  {
    return isSetVal==1;
  }
  OptionalProperty& operator=(const OptionalProperty& src)
  {
    isSetVal=src.isSetVal;

    if(src.isSetVal==1)
    {
      type=src.type;
      switch(type)
      {
        case SMS_INT_TAG:iValue=src.iValue;break;
        case SMS_STR_TAG:
        {
          xValue.assign(src.xValue.data(),src.xValue.length());
        }break;
        case SMS_BIN_TAG:
        {
          xValue.assign(src.xValue.data(),src.xValue.length());
        }break;
      }
    }
    return *this;
  }
  ~OptionalProperty()
  {
  }
  void setInt(int value)
  {
    type=SMS_INT_TAG;
    iValue=value;
    isSetVal=1;
  }
  void setStr(const char* str)
  {
    type=SMS_STR_TAG;
    xValue=str;
    isSetVal=1;
  }
  void setBin(const char* bin,int len)
  {
    type=SMS_BIN_TAG;
    xValue.assign(bin,len);
    isSetVal=1;
  }
  int getInt()const
  {
    return isSetVal?iValue:0;
  }
  const SmsPropertyBuf& getStr()const
  {
    if(isSetVal==1)return xValue;
    else return *nullStr;
  }
  const char* getBin(unsigned* len)const
  {
    if(isSetVal==1)
    {
      if(len)*len=(unsigned)xValue.length();
      return xValue.data();
    }else
    {
      if(len)*len=0;
      return 0;
    }
  }
  void Unset()
  {
    isSetVal=0;
  }
};

struct PropertySet{
  OptionalProperty properties[SMS_LAST_TAG+1];
};

/**
*
*
* @author Victor V. Makarov
* @version 10.0
* @see SMS
*/
struct Body
{
private:
  mutable auto_ptr<uint8_t> buff;
  mutable int         buffLen;
  mutable PropertySet prop;
public:
  /**
  * Default ,
  */
  Body() : buffLen(0) {};

  /**
  *   Body,     .
  *
  *
  * @param data
  * @param len      data
  */
  Body(uint8_t* data, int len)
    : buff(0), buffLen(0)
  {
    setBuffer(data, len);
  };

  /**
  *  ,
  *
  * @param body    .
  */
  Body(const Body& body)
    : buffLen(0)
  {
    /*int len = body.getBufferLength();
    uint8_t* b = new uint8_t[len];
    memcpy(b,body.getBuffer(),len);
    setBuffer(b, len);*/
    buff=auto_ptr<uint8_t>(0);
    prop=body.prop;
  }

  const PropertySet& getProperties()const
  {
    return prop;
  }

  PropertySet& getProperties()
  {
    return prop;
  }

  /**
  *   '=',
  *
  *
  * @param body      '='
  * @return
  */
  Body& operator =(const Body& body)
  {
    buffLen=0;
    buff=auto_ptr<uint8_t>(0);
    prop=body.prop;
    /*int len = body.getBufferLength();
    uint8_t* b = new uint8_t[len];
    memcpy(b,body.getBuffer(),len);
    setBuffer(b, len);*/
    return (*this);
  };

  /**
  *    .
  *  ,     .
  *
  *
  * @return   .  NULL.
  */
  char* getDecodedText()
  {
    return 0L;
  };

  uint8_t* getBuffer()    const
  {
    int blength=getBufferLength();
    if ( !buff.get() || buffLen < blength )
    {
      buff = auto_ptr<uint8_t>(new uint8_t[blength]);
    }
    buffLen = blength;
    encode(buff.get(),buffLen);
    return buff.get();
  }

  int getBufferLength() const
  {
    return getRequiredBufferSize();
  }

  void setBuffer(uint8_t* buffer, int length)
  {
    //encode(buffer,length);
    decode(buffer,length);
    this->buff = auto_ptr<uint8_t>(buffer);
    //delete buffer;
  }

  unsigned getShortMessageLength()
  {
      if( hasIntProperty(Tag::SMPP_SM_LENGTH) ) {
          return getIntProperty(Tag::SMPP_SM_LENGTH);
      } else if( hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD) ) {
          unsigned len = 0;
          getBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, &len);
          return len;
      }
      return 0;
  }

  void encode(uint8_t* buffer,int& length) const;

  int getRequiredBufferSize() const;

  void decode(uint8_t* buffer,int length);

  void setStrProperty(int tag,const char* value)
  {
    __require__((tag>>8)==SMS_STR_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    prop.properties[tag].setStr(value);
  }

  static int unType(int tag) { return tag&0x0ff; }

  void setBinProperty(int tag,const char* value, unsigned len);

  void setIntProperty(int tag,const uint32_t value)
  {
    __require__((tag>>8)==SMS_INT_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    prop.properties[tag].setInt(value);
  }

  const SmsPropertyBuf& getStrProperty(int tag)const
  {
    __require__((tag>>8)==SMS_STR_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    return prop.properties[tag].getStr();
  }

  const char* getBinProperty(int tag,unsigned* len)const;

  uint32_t getIntProperty(int tag)const
  {
    __require__((tag>>8)==SMS_INT_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    return prop.properties[tag].getInt();
  }

  bool hasProperty(int tag)const
  {
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    return prop.properties[tag].isSet();
  }

  bool hasIntProperty(int tag)const
  {
    __require__((tag>>8)==SMS_INT_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    return prop.properties[tag].isSet();
  }

  bool hasStrProperty(int tag)const
  {
    __require__((tag>>8)==SMS_STR_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    return prop.properties[tag].isSet();
  }

  bool hasBinProperty(int tag)const
  {
    __require__((tag>>8)==SMS_BIN_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    /*
    if ( tag == unType(Tag::SMPP_SHORT_MESSAGE) ) {
      //return prop.properties[Tag::SMSC_RAW_SHORTMESSAGE].isSet!=0;;
      tag = unType(Tag::SMSC_RAW_SHORTMESSAGE);
    }
    if ( tag == unType(Tag::SMPP_MESSAGE_PAYLOAD) ) {
      //return prop.properties[Tag::SMSC_RAW_SHORTMESSAGE].isSet!=0;;
      tag = unType(Tag::SMSC_RAW_PAYLOAD);
    }*/
    return prop.properties[tag].isSet();
  }

  void dropProperty(int tag)
  {
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    prop.properties[tag].Unset();
  }

  void dropIntProperty(int tag)
  {
    __require__((tag>>8)==SMS_INT_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    prop.properties[tag].isSetVal=0;
    prop.properties[tag].iValue=0;
  }

  template <class TLOG> void Print(TLOG* f)
  {
    for(int i=0;i<=SMS_LAST_TAG;i++)
    {
      if(prop.properties[i].isSet())
      {
        switch(prop.properties[i].type)
        {
          case SMS_INT_TAG:
            fprintf(f,"i:%s:%d=%d\n",Tag::tagNames[i],i,prop.properties[i].iValue);
            break;
          case SMS_STR_TAG:
            fprintf(f,"s:%s:%d(%ld)=%s\n",Tag::tagNames[i],i,prop.properties[i].xValue.length(),prop.properties[i].xValue.c_str());
            break;
          case SMS_BIN_TAG:
            fprintf(f,"b:%s:%d(%ld)=%s\n",Tag::tagNames[i],i,prop.properties[i].xValue.length(),prop.properties[i].xValue.c_str());
            break;
        }
      }
    }
  }

  void Clear()
  {
    for(int i=0;i<=SMS_LAST_TAG;i++)
    {
      prop.properties[i].Unset();
    }
  }

};


/**
*   SMS
* SMS .   .
*
* @author Victor V. Makarov
* @version 1.0
* @see MessageStore
*/


struct SMS
{
  State       state;
  time_t      submitTime;     // /   SMSC
  time_t      validTime;      // /

  uint32_t    attempts;       //
  uint32_t    lastResult;     //
  uint32_t    oldResult;      //   lastResult
  time_t      lastTime;       // /
  time_t      nextTime;       // /

  Address     originatingAddress;
  Address     destinationAddress;
  Address     dealiasedDestinationAddress;

  uint16_t    messageReference;
  EService    eServiceType;

  bool        needArchivate;
  uint8_t     deliveryReport;
  uint8_t     billingRecord;

  Descriptor  originatingDescriptor;
  Descriptor  destinationDescriptor;

  Body        messageBody;    //   + PDU .
  bool        attach;

  RouteId     routeId;        // Additional fields
  int32_t     serviceId;      // Additional fields
  int32_t     priority;       // Additional fields

  SmeSystemIdType srcSmeId;
  SmeSystemIdType dstSmeId;

  uint16_t    concatMsgRef;   // For concatenated messages only
  uint8_t     concatSeqNum;   //

  int         dialogId;  // used for transaction mode, not stored in DB

  /**
  * Default ,    state  ENROUTE
  *
  */
  SMS() : state(ENROUTE), submitTime(0), validTime(0),
    attempts(0), lastResult(0), oldResult(0),lastTime(0), nextTime(0),
    messageReference(0), needArchivate(true),
    deliveryReport(0), billingRecord(0), attach(false),
    serviceId(0), priority(0),concatMsgRef(0),concatSeqNum(0),dialogId(-1)
  {
    eServiceType[0]='\0'; routeId[0]='\0';
    srcSmeId[0]='\0'; dstSmeId[0]='\0';
  };


  /**
  *  ,
  * SMS
  *
  * @param sms     SMS
  */
  SMS(const SMS& sms) :
    state(sms.state),
    submitTime(sms.submitTime),
    validTime(sms.validTime),
    attempts(sms.attempts),
    lastResult(sms.lastResult),
    oldResult(sms.oldResult),
    lastTime(sms.lastTime),
    nextTime(sms.nextTime),
    originatingAddress(sms.originatingAddress),
    destinationAddress(sms.destinationAddress),
    dealiasedDestinationAddress(sms.dealiasedDestinationAddress),
    messageReference(sms.messageReference),
    needArchivate(sms.needArchivate),
    deliveryReport(sms.deliveryReport),
    billingRecord(sms.billingRecord),
    originatingDescriptor(sms.originatingDescriptor),
    destinationDescriptor(sms.destinationDescriptor),
    messageBody(sms.messageBody),
    attach(sms.attach),
    serviceId(sms.serviceId),
    priority(sms.priority),
    concatMsgRef(sms.concatMsgRef),
    concatSeqNum(sms.concatSeqNum),
    dialogId(sms.dialogId)
  {
    strncpy(eServiceType, sms.eServiceType, sizeof(eServiceType));
    strncpy(routeId, sms.routeId, sizeof(routeId));
    strncpy(srcSmeId, sms.srcSmeId, sizeof(srcSmeId));
    strncpy(dstSmeId, sms.dstSmeId, sizeof(dstSmeId));
  };

  /**
  *   '=',
  *
  *
  * @param sms      '='
  * @return
  */
  SMS& operator =(const SMS& sms)
  {
    state = sms.state;
    submitTime = sms.submitTime;
    validTime = sms.validTime;
    attempts = sms.attempts;
    lastResult = sms.lastResult;
    oldResult = sms.oldResult;
    lastTime = sms.lastTime;
    nextTime = sms.nextTime;
    originatingAddress = sms.originatingAddress;
    destinationAddress = sms.destinationAddress;
    dealiasedDestinationAddress = sms.dealiasedDestinationAddress;
    messageReference = sms.messageReference;
    needArchivate = sms.needArchivate;
    deliveryReport = sms.deliveryReport;
    billingRecord = sms.billingRecord;
    originatingDescriptor = sms.originatingDescriptor;
    destinationDescriptor = sms.destinationDescriptor;
    messageBody = sms.messageBody;
    attach = sms.attach;
    serviceId = sms.serviceId;
    priority = sms.priority;

    concatMsgRef=sms.concatMsgRef;
    concatSeqNum=sms.concatSeqNum;

    dialogId=sms.dialogId;

    strncpy(eServiceType, sms.eServiceType, sizeof(eServiceType));
    strncpy(routeId, sms.routeId, sizeof(routeId));
    strncpy(srcSmeId, sms.srcSmeId, sizeof(srcSmeId));
    strncpy(dstSmeId, sms.dstSmeId, sizeof(dstSmeId));


    return (*this);
  };

  /**
  *
  *
  * @return
  */
  inline State getState() const
  {
    return state;
  };

  /**
  *   .
  *
  *
  * @param length   (0 < length <= MAX_ADDRESS_VALUE_LENGTH)
  * @param type
  * @param plan
  * @param buff
  * @see Address
  */
  inline void setOriginatingAddress(uint8_t length, uint8_t type,
    uint8_t plan, const char* buff)
  { // Copies address value from 'buff' to static structure
    originatingAddress.setTypeOfNumber(type);
    originatingAddress.setNumberingPlan(plan);
    originatingAddress.setValue(length, buff);
  };

  /**
  *
  *
  *
  * @param address
  * @see Address
  */
  inline void setOriginatingAddress(const Address& address)
  { // Copies address value from 'address' to static structure
    originatingAddress = address;
  };

  /**
  *
  *
  * @return
  * @see Address
  */
  inline const Address& getOriginatingAddress() const
  {
    return originatingAddress;
  };

  /**
  *
  *
  * @return
  * @see Address
  */
  inline Address& getOriginatingAddress()
  {
    return originatingAddress;
  };

  /**
  *
  *
  *
  * @param length   (0 < length <= MAX_ADDRESS_VALUE_LENGTH)
  * @param type
  * @param plan
  * @param buff
  * @see Address
  */
  inline void setDestinationAddress(uint8_t length, uint8_t type,
    uint8_t plan, const char* buff)
  { // Copies address value from 'buff' to static structure
    destinationAddress.setTypeOfNumber(type);
    destinationAddress.setNumberingPlan(plan);
    destinationAddress.setValue(length, buff);
  };

  /**
  *
  *
  *
  * @param address
  * @see Address
  */
  inline void setDestinationAddress(const Address& address)
  { // Copies address value from 'address' to static structure
    destinationAddress = address;
  };

  /**
  *
  *
  * @return
  * @see Address
  */
  inline const Address& getDestinationAddress() const
  {
    return destinationAddress;
  };

  /**
  *
  *
  * @return
  * @see Address
  */
  inline Address& getDestinationAddress()
  {
    return destinationAddress;
  };

  inline void setDealiasedDestinationAddress(uint8_t length, uint8_t type,
    uint8_t plan, const char* buff)
  { // Copies address value from 'buff' to static structure
    dealiasedDestinationAddress.setTypeOfNumber(type);
    dealiasedDestinationAddress.setNumberingPlan(plan);
    dealiasedDestinationAddress.setValue(length, buff);
  };

  inline void setDealiasedDestinationAddress(const Address& address)
  { // Copies address value from 'address' to static structure
    dealiasedDestinationAddress = address;
  };

  inline const Address& getDealiasedDestinationAddress() const
  {
    return dealiasedDestinationAddress;
  };

  inline Address& getDealiasedDestinationAddress()
  {
    return dealiasedDestinationAddress;
  };

  /**
  *
  *
  *
  * @param descriptor
  * @see Descriptor
  */
  inline void setOriginatingDescriptor(const Descriptor& descriptor)
  { // Copies descriptor from 'descriptor' to static structure
    originatingDescriptor = descriptor;
  };

  /**
  *
  *
  *
  * @param _mscLen     _msc
  * @param _value   MSC
  * @param _imsiLen     _imsi
  * @param _value   IMSI
  * @param _sme  SME
  *
  * @see Descriptor
  */
  inline void setOriginatingDescriptor(uint8_t _mscLen, const char* _msc,
    uint8_t _imsiLen, const char* _imsi, uint32_t _sme)
  { // Copies descriptor from 'descriptor' to static structure
    originatingDescriptor.setMsc(_mscLen, _msc);
    originatingDescriptor.setImsi(_imsiLen, _imsi);
    originatingDescriptor.setSmeNumber(_sme);
  };

  /**
  *
  *
  * @return
  * @see Descriptor
  */
  inline const Descriptor& getOriginatingDescriptor() const
  {
    return originatingDescriptor;
  };

  /**
  *
  *
  * @return
  * @see Descriptor
  */
  inline Descriptor& getOriginatingDescriptor()
  {
    return originatingDescriptor;
  };

  /**
  *
  *
  *
  * @param descriptor
  * @see Descriptor
  *
  inline void setDestinationDescriptor(const Descriptor& descriptor)
  { // Copies descriptor from 'descriptor' to static structure
      destinationDescriptor = descriptor;
  };*/

  /**
  *
  *
  * @return
  * @see Descriptor
  */
  inline const Descriptor& getDestinationDescriptor() const
  {
    return destinationDescriptor;
  };

  /**
  *
  *
  * @return
  * @see Descriptor
  *
  inline Descriptor& getDestinationDescriptor()
  {
  return destinationDescriptor;
    };*/


  /**
  *    .
  *
  * @param time   ,
  *               (  ).
  */
  inline void setValidTime(time_t time)
  {
    validTime = time;
  };

  /**
  *    .
  *
  * @return time   ,
  *         (  ).
  */
  inline time_t getValidTime() const
  {
    return validTime;
  };

  /**
  *      SMSC
  *
  * @param time       SMSC
  */
  inline void setSubmitTime(time_t time)
  {
    submitTime = time;
  };

  /**
  *      SMSC
  *
  * @return     SMSC
  */
  inline time_t getSubmitTime() const
  {
    return submitTime;
  };

  /**
  *        SMSC
  *
  * @param time         SMSC
  *
  inline void setLastTime(time_t time)
  {
      lastTime = time;
  };*/

  /**
  *        SMSC
  *
  * @return       SMSC
  */
  inline time_t getLastTime() const
  {
    return lastTime;
  };

  /**
  *        SMSC
  *
  * @param time         SMSC
  */
  inline void setNextTime(time_t time)
  {
    nextTime = time;
  };

  /**
  *        SMSC
  *
  * @return       SMSC
  */
  inline time_t getNextTime() const
  {
    return nextTime;
  };

  /**
  *    (MR),
  *
  *
  * @param mr        (MR)
  */
  inline void setMessageReference(uint16_t mr)
  {
    messageReference = mr;
  };

  /**
  *    (MR),
  *
  *
  * @return    (MR)
  */
  inline uint16_t getMessageReference() const
  {
    return messageReference;
  };

  /**
  *
  *
  * @param req
  */
  inline void setDeliveryReport(uint8_t report)
  {
    deliveryReport = report;
  };

  /**
  *
  *
  * @return
  */
  inline uint8_t getDeliveryReport() const
  {
    return deliveryReport;
  };

  /**
  *
  *
  * @param req
  */
  inline void setBillingRecord(uint8_t billing)
  {
    billingRecord = billing;
  };

  inline bool billingRequired()const
  {
    return billingRecord==BILLING_NORMAL   || billingRecord==BILLING_MT ||
           billingRecord==BILLING_ONSUBMIT || billingRecord==BILLING_CDR;
  }

  /**
  *
  *
  * @return
  */
  inline uint8_t getBillingRecord() const
  {
    return billingRecord;
  };

  /**
  *  ,
  *
  *
  * @param arc    ,
  */
  inline void setArchivationRequested(bool arc)
  {
    needArchivate = arc;
  };

  /**
  *  ,
  *
  *
  * @return ,       ( / )
  */
  inline bool isArchivationRequested() const
  {
    return needArchivate;
  };

  /**
  *
  * //
  *
  * @return  //
  */
  inline uint32_t getLastResult() const
  {
    return lastResult;
  };

  /**
  *
  * //
  *
  *
  * @param value
  *
  */

  inline void setLastResult(uint32_t value)
  {
    oldResult=lastResult;
    lastResult=value;
  }

  /**
  *
  *
  * @param count
  *
  */
  inline void setAttemptsCount(uint32_t count)
  {
  attempts = count;
  };

  /**
  *
  *
  * @return
  */
  inline uint32_t getAttemptsCount() const
  {
    return attempts;
  };

  /**
  *
  *
  * @return
  * @see Body
  */
  inline const Body& getMessageBody() const
  {
    return messageBody;
  };

  /**
  *
  *
  * @return
  * @see Body
  */
  inline Body& getMessageBody()
  {
    return messageBody;
  };

  inline void setStringField(char* field, const char* val, int maxLen)
  {
      if (val) {
        strncpy(field, val, maxLen); field[maxLen]='\0';
      } else field[0]='\0';
  }
  inline void getStringField(const char* field, char* val, int maxLen) const
  {
      __require__(val);
      val[0]='\0'; strncpy(val, field, maxLen); val[maxLen]='\0';
  }

  /**
  *   -  SME.
  *
  * @param type - SME
  */
  inline void setEServiceType(const char* type)
  {
      setStringField(eServiceType, type, MAX_ESERVICE_TYPE_LENGTH);
  };
  /**
  *   -
  *
  * @param type
  *
  *               MAX_ESERVICE_TYPE_LENGTH+1,
  *
  */
  inline void getEServiceType(char* type) const
  {
      getStringField(eServiceType, type, MAX_ESERVICE_TYPE_LENGTH);
  };
  /**
  *   -
  *
  * @return -
  */
  inline const char* getEServiceType() const
  {
      return eServiceType;
  };

  /**
  *
  *   .
  *
  * @param pri
  */
  inline void setPriority(int32_t pri) {
      priority = pri;
  };
  /**
  *
  *   .
  *
  * @return
  */
  inline int32_t getPriority() const {
      return priority;
  };

  /**
  *
  *   .
  *
  * @param id
  */
  inline void setServiceId(int32_t id) {
      serviceId = id;
  };
  /**
  *
  *   .
  *
  * @return
  */
  inline uint32_t getServiceId() const {
      return serviceId;
  };

  /**
  *    .
  *
  * @param route
  */
  inline void setRouteId(const char* route)
  {
      setStringField(routeId, route, MAX_ROUTE_ID_TYPE_LENGTH);
  };
  /**
  *
  *
  * @param route
  *
  *
  *               MAX_ROUTE_ID_TYPE_LENGTH+1
  */
  inline void getRouteId(char* route) const
  {
      getStringField(routeId, route, MAX_ROUTE_ID_TYPE_LENGTH);
  };
  /**
  *
  *
  * return
  */
  inline const char* getRouteId() const
  {
    return routeId;
  };

  /**
  *    sme-.
  *
  * @param id  sme-
  */
  inline void setSourceSmeId(const char* id)
  {
      setStringField(srcSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
  };
  /**
  *    sme-
  *
  * @param id
  *                sme-
  *
  *               MAX_SMESYSID_TYPE_LENGTH+1
  */
  inline void getSourceSmeId(char* id) const
  {
      getStringField(srcSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
  };
  /**
  *    sme-
  *
  * return  sme-
  */
  inline const char* getSourceSmeId() const
  {
    return srcSmeId;
  };

  /**
  *    sme-.
  *
  * @param id  sme-
  */
  inline void setDestinationSmeId(const char* id)
  {
    setStringField(dstSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
  };

  /**
  *    sme-
  *
  * @param id
  *                sme-
  *
  *               MAX_SMESYSID_TYPE_LENGTH+1
  */
  inline void getDestinationSmeId(char* id) const
  {
      getStringField(dstSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
  };
  /**
  *    sme-
  *
  * return  sme-
  */
  inline const char* getDestinationSmeId() const
  {
    return dstSmeId;
  };

  inline void setConcatMsgRef(uint16_t newval)
  {
    concatMsgRef=newval;
  }

  inline uint16_t getConcatMsgRef()
  {
    return concatMsgRef;
  }

  inline void setConcatSeqNum(uint8_t newval)
  {
    concatSeqNum=newval;
  }

  inline uint8_t getConcatSeqNum()const
  {
    return concatSeqNum;
  }

  void setStrProperty(int tag,const char* value)
  {
    messageBody.setStrProperty(tag,value);
  }
  void setBinProperty(int tag,const char* value, unsigned len)
  {
    messageBody.setBinProperty(tag,value,len);
  }
  void setIntProperty(int tag,const uint32_t value)
  {
    messageBody.setIntProperty(tag,value);
  }
  const SmsPropertyBuf& getStrProperty(int tag)const
  {
    return messageBody.getStrProperty(tag);
  }
  const char* getBinProperty(int tag,unsigned* len)const
  {
    return messageBody.getBinProperty(tag,len);
  }
  uint32_t getIntProperty(int tag)const
  {
    return messageBody.getIntProperty(tag);
  }
  bool hasProperty(int tag)const
  {
    return messageBody.hasProperty(tag);
  }
  bool hasIntProperty(int tag)const
  {
    return messageBody.hasIntProperty(tag);
  }
  bool hasStrProperty(int tag)const
  {
    return messageBody.hasStrProperty(tag);
  }
  bool hasBinProperty(int tag)const
  {
    return messageBody.hasBinProperty(tag);
  }
  void dropProperty(int tag)
  {
    messageBody.dropProperty(tag);
  }


  ~SMS() {}

  bool Invalidate(const char* file,int line)
  {
    if(messageBody.hasBinProperty(Tag::SMPP_MESSAGE_PAYLOAD))
    {
      if(messageBody.hasBinProperty(Tag::SMPP_SHORT_MESSAGE))
      {
        unsigned len=0;
        messageBody.getBinProperty(Tag::SMPP_SHORT_MESSAGE,&len);
        if(len==0)return true;
#ifndef NOLOGGERPLEASE
        logger::_sms_err_cat->log_(logger::Logger::LEVEL_WARN, "both payload and shortmessage present at %s:%d",file,line);
        //smsc_log_warn(logger::_sms_err_cat, "both payload and shortmessage present at %s:%d",file,line);
#endif
        return false;
      }
    }
    return true;
  }

  void Clear()
  {
    state=ENROUTE;
    submitTime=0;
    validTime=0;
    attempts=0;
    lastResult=0;
    oldResult=0;
    lastTime=0;
    nextTime=0;
    messageReference=0;
    needArchivate=false;

    deliveryReport=0;
    billingRecord=0;
    attach=false;

    serviceId=0;
    priority=0;
    concatMsgRef=0;
    concatSeqNum=0;
    dialogId=-1;

    eServiceType[0]='\0';
    routeId[0]='\0';
    srcSmeId[0]='\0';
    dstSmeId[0]='\0';
    originatingAddress.Clear();
    destinationAddress.Clear();
    dealiasedDestinationAddress.Clear();

    originatingDescriptor.Clear();
    destinationDescriptor.Clear();

    messageBody.Clear();
  }
};

struct SMSPartInfo{
  enum{
    flPartPresent=1,
    flHasSRR=2
  };
  SMSPartInfo():fl(flPartPresent){}
  uint8_t fl;//flags
  uint8_t dc;//dc of part
  uint8_t mr;//mr of part
  uint64_t stime;// submit time of part
  enum{SIZE=11};
};

inline int getSMSPartsCount(SMS& sms)
{
  if(!sms.hasBinProperty(Tag::SMSC_ORGPARTS_INFO))
  {
    throw smsc::util::Exception("sms part info requested, but not present");
  }
  unsigned len;
  const uint8_t* data=(const uint8_t*)sms.getBinProperty(Tag::SMSC_ORGPARTS_INFO,&len);
  if(data[0]==0)
  {
    throw smsc::util::Exception("invalid sms part info detected");
  }
  return (len-1)/data[0];
}

inline SMSPartInfo getSMSPartInfoBin(const uint8_t* data,int len,int partIdx)
{
  SMSPartInfo rv;
  uint8_t sz=*data;
  data++;
  if((unsigned(partIdx)+1)*sz>len)
  {
    throw smsc::util::Exception("sms part info index is out of range (idx=%d, len=%u)",partIdx,len);
  }
  data+=partIdx*sz;
  rv.fl=data[0];
  rv.dc=data[1];
  rv.mr=data[2];
  if(sz>=11)
  {
    memcpy(&rv.stime,data+3,8);
    rv.stime=smsc::util::Uint64Converter::toHostOrder(rv.stime);
  }else
  {
    rv.stime=0;
  }
  return rv;
}

inline SMSPartInfo getSMSPartInfo(SMS& sms,int partIdx)
{
  if(!sms.hasBinProperty(Tag::SMSC_ORGPARTS_INFO))
  {
    throw smsc::util::Exception("sms part info requested, but not present");
  }
  unsigned len;
  const uint8_t* data=(const uint8_t*)sms.getBinProperty(Tag::SMSC_ORGPARTS_INFO,&len);
  return getSMSPartInfoBin(data,len,partIdx);
}


inline void fillSMSPartInfo(SMS& sms,int partsNum,int partIdx,SMSPartInfo partInfo)
{
  if(!sms.hasBinProperty(Tag::SMSC_ORGPARTS_INFO))
  {
    smsc::core::buffers::TmpBuf<uint8_t,1+SMSPartInfo::SIZE*10> buf(1+partsNum*SMSPartInfo::SIZE);
    //char buf[1+SMSPartInfo::SIZE*256]={SMSPartInfo::SIZE,0,};
    buf.get()[0]=SMSPartInfo::SIZE;
    memset(buf.get()+1,0,partsNum*SMSPartInfo::SIZE);
    sms.setBinProperty(Tag::SMSC_ORGPARTS_INFO,(char*)buf.get(),1+partsNum*SMSPartInfo::SIZE);
  }
  unsigned len;
  uint8_t* data=(uint8_t*)sms.getBinProperty(Tag::SMSC_ORGPARTS_INFO,&len);
  if((unsigned(partIdx)+1)*SMSPartInfo::SIZE>len)
  {
    throw smsc::util::Exception("sms part info index is out of range (idx=%d, len=%u)",partIdx,len);
  }
  uint8_t sz=*data;
  data++;
  data+=partIdx*sz;
  data[0]=partInfo.fl;
  data[1]=partInfo.dc;
  data[2]=partInfo.mr;
  if(sz>=11)
  {
    uint64_t tmp=smsc::util::Uint64Converter::toNetworkOrder(partInfo.stime);
    memcpy(data+3,&tmp,8);
  }
}

}//sms
}//smsc

#endif
