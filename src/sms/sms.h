
#ifndef __SMSC_SMS_SMS_H__
#define __SMSC_SMS_SMS_H__

/**
* Файл содержит описание внутренней структуры данных для представления SMS
* в системе SMS центра. Используется системой хранения.
*
* @author Victor V. Makarov
* @version 1.0
* @see MessageStore
*/

#include <stdio.h>
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


#include <util/debug.h>
#include "sms/sms_const.h"
#include "sms/sms_tags.h"

namespace smsc {
namespace sms  {



extern void UCS_htons(char* buffer, const char* orig, unsigned len, unsigned esm_class);
extern void UCS_ntohs(char* buffer, const char* orig, unsigned len, unsigned esm_class);

using std::string;
using std::runtime_error;
using std::auto_ptr;

static bool HSNS_isEqual()
{
  static bool isEqual = (htons(0x1234) == 0x1234);
  return isEqual;
}

/**
* Структура Address предназначена для хранения
* адресов в SMS сообщении.
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
  * Default конструктор, просто инициализирует некоторые поля нулями
  */
  Address() : length(1), type(0), plan(0)
  {
    value[0] = '0'; value[1] = '\0';
  };

  /**
  * Конструктор для Address, инициализирует поля структуры реальными данными.
  * Копирует даннуе из буфера к себе
  *
  * @param _len   длинна буфера _value
  * @param _type  тип адреса
  * @param _plan  план нумерации
  * @param _value значение адреса
  */
  Address(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
    : length(_len), type(_type), plan(_plan)
  {
    setValue(_len, _value);
  };

  /**
  * Конструктор копирования, используется для создания адреса по образцу
  *
  * @param addr   образец адреса.
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
    int scaned = sscanf(text,".%d.%d.%20s",
      &itype,
      &iplan,
      addr_value);
    if ( scaned != 3 )
    {
      scaned = sscanf(text,"+%20[0123456789?]s",addr_value);
      if ( scaned )
      {
        iplan = 1;//ISDN
        itype = 1;//INTERNATIONAL
      }
      else
      {
        scaned = sscanf(text,"%20[0123456789?]s",addr_value);
        if ( !scaned )
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
    length = strlen(addr_value);
    memcpy(value,addr_value,sizeof(addr_value));
  }

  /**
  * Переопределённый оператор '=',
  * используется для копирования адресов друг в друга
  *
  * @param addr   Правая часть оператора '='
  * @return Ссылку на себя
  */
  Address& operator =(const Address& addr)
  {
    type = addr.type; plan = addr.plan;
    setValue(addr.length, addr.value);
    return (*this);
  };

  inline int operator ==(const Address& addr)
  {
    return (length == addr.length &&
            plan == addr.plan && type == addr.type &&
            memcmp(value, addr.value, length) == 0);
  };


  /**
  * Метод устанавливает значение адреса и его длинну.
  * Длинна адреса должна быть меньше MAX_ADDRESS_VALUE_LENGTH.
  *
  * @param _len   длинна нового адреса
  * @param _value значение нового адреса
  */
  inline void setValue(uint8_t _len, const char* _value)
  {
    __require__(_len && _value && _value[0]
      && _len<sizeof(AddressValue));

    memcpy(value, _value, _len*sizeof(uint8_t));
    value[length = _len] = '\0';
  };

  /**
  * Метод копирует значение адреса и возвращает его длинну
  *
  * @param _value указатель на буфер куда будет скопированно значение адреса
  *               буфер должен иметь размер не меньше
  *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы принять любое значение
  * @return длинна адреса
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
  * Возвращает длинну адреса
  *
  * @return длинна адреса
  */
  inline uint8_t getLength() const
  {
    return length;
  };

  /**
  * Устанавливает тип адреса
  *
  * @param _type  тип адреса
  */
  inline void setTypeOfNumber(uint8_t _type)
  {
    type = _type;
  };

  /**
  * Возвращает тип адреса
  *
  * @param _type  тип адреса
  */
  inline uint8_t getTypeOfNumber() const
  {
    return type;
  };

  /**
  * Устанавливает план нумерации адреса
  *
  * @param _plan  план нумерации адреса
  */
  inline void setNumberingPlan(uint8_t _plan)
  {
    plan = _plan;
  };

  /**
  * Возвращает план нумерации адреса
  *
  * @return план нумерации адреса
  */
  inline uint8_t getNumberingPlan() const
  {
    return plan;
  };

  inline int getText(char* buf,int buflen)const
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
  inline int toString(char* buf,int buflen)const{
    char vl[32];
    memcpy(vl,value,length);
    vl[length]=0;
    return snprintf(buf,buflen,".%d.%d.%s",type,plan,vl);
  }
};

/**
* Структура Descriptor предназначена для хранения
* информации идентифицирующей взаимодействующую сторону
* (например, источник или приёмник сообщения SMS)
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
  * Default конструктор, просто инициализирует некоторые поля нулями
  */
  Descriptor() : mscLength(0), imsiLength(0), sme(0)
  {
    msc[0] = '\0'; imsi[0] = '\0';
  };

  /**
  * Конструктор для Descriptor, инициализирует поля структуры реальными данными.
  * Копирует даннуе из буферов к себе
  *
  * @param _mscLen   длинна буфера _msc
  * @param _value значение адреса MSC
  * @param _imsiLen   длинна буфера _imsi
  * @param _value значение адреса IMSI
  * @param _sme номер SME
  */
  Descriptor(uint8_t _mscLen, const char* _msc,
    uint8_t _imsiLen, const char* _imsi, uint32_t _sme)
    : mscLength(_mscLen), imsiLength(_imsiLen), sme(_sme)
  {
    setMsc(mscLength, _msc);
    setImsi(imsiLength, _imsi);
  };

  /**
  * Конструктор копирования, используется для
  * создания дескриптора по образцу
  *
  * @param descr образец дескриптора.
  */
  Descriptor(const Descriptor& descr)
    : mscLength(descr.mscLength),
    imsiLength(descr.imsiLength), sme(descr.sme)
  {
    setMsc(descr.mscLength, descr.msc);
    setImsi(descr.imsiLength, descr.imsi);
  };

  /**
  * Переопределённый оператор '=',
  * используется для копирования дескрипторов друг в друга
  *
  * @param descr   Правая часть оператора '='
  * @return Ссылку на себя
  */
  Descriptor& operator =(const Descriptor& descr)
  {
    sme = descr.sme;
    setMsc(descr.mscLength, descr.msc);
    setImsi(descr.imsiLength, descr.imsi);
    return (*this);
  };

  /**
  * Метод устанавливает значение адреса MSC и его длинну.
  * Длинна адреса должна быть меньше MAX_ADDRESS_VALUE_LENGTH.
  *
  * @param _len   длинна нового адреса MSC
  * @param _value значение нового адреса MSC
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
  * Метод устанавливает значение адреса IMSI и его длинну.
  * Длинна адреса должна быть меньше MAX_ADDRESS_VALUE_LENGTH.
  *
  * @param _len   длинна нового адреса IMSI
  * @param _value значение нового адреса IMSI
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
  * Метод копирует значение адреса MSC и возвращает его длинну
  *
  * @param _value указатель на буфер куда будет скопированно значение
  *               адреса MSC. Буфер должен иметь размер не меньше
  *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы принять любое значение
  * @return длинна адреса MSC
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
  * Метод копирует значение адреса IMSI и возвращает его длинну
  *
  * @param _value указатель на буфер куда будет скопированно значение
  *               адреса IMSI. Буфер должен иметь размер не меньше
  *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы принять любое значение
  * @return длинна адреса IMSI
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
  * Возвращает длинну адреса MSC
  *
  * @return длинна адреса MSC
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
  * Возвращает длинну адреса IMSI
  *
  * @return длинна адреса IMSI
  */
  inline uint8_t getImsiLength() const
  {
    return imsiLength;
  };

  /**
  * Устанавливает номер SME
  *
  * @return длинна адреса IMSI
  */
  inline void setSmeNumber(uint32_t _sme)
  {
    sme = _sme;
  };

  /**
  * Возвращает номер SME
  *
  * @return номер SME
  */
  inline uint32_t getSmeNumber() const
  {
    return sme;
  };
};

static const string nullStr="";

struct OptionalProperty{
  uint16_t isSet;
  uint16_t type;
  union{
    int iValue;
    string* sValue;
    string* bValue;
  };
  OptionalProperty():isSet(0),type(SMS_INT_TAG),iValue(0){}
  OptionalProperty(const OptionalProperty& prop)
  {
    *this=prop;
  }
  OptionalProperty& operator=(const OptionalProperty& src)
  {
    isSet=src.isSet;
    type=src.type;
    if(isSet)
    {
      switch(type)
      {
        case SMS_INT_TAG:iValue=src.iValue;break;
        case SMS_STR_TAG:
        {
          if(!sValue)sValue=new string;
          *sValue=*src.sValue;
        }break;
        case SMS_BIN_TAG:
        {
          if(!bValue)bValue=new string;
          *bValue=*src.bValue;
        }break;
      }
    }
    return *this;
  }
  ~OptionalProperty()
  {
    if(isSet)
    {
      if(type==SMS_STR_TAG)delete sValue;
      else
      if(type==SMS_BIN_TAG)delete bValue;
    }
  }
  void setInt(int value)
  {
    type=SMS_INT_TAG;
    iValue=value;
    isSet=1;
  }
  void setStr(const char* str)
  {
    type=SMS_STR_TAG;
    if(!isSet)sValue=new string;
    *sValue=str;
    isSet=1;
  }
  void setBin(const char* bin,int len)
  {
    type=SMS_BIN_TAG;
    if(!isSet)bValue=new string;
    bValue->assign(bin,len);
    isSet=1;
  }
  int getInt()const
  {
    return iValue;
  }
  const string& getStr()const
  {
    if(isSet)return *sValue;
    else return nullStr;
  }
  const char* getBin(unsigned* len)const
  {
    if(isSet)
    {
      if(len)*len=bValue->length();
      return bValue->c_str();
    }else
    {
      if(len)*len=0;
      return 0;
    }
  }
};

struct PropertySet{
  OptionalProperty properties[SMS_LAST_TAG+1];
};

/**
* Структура описывает тело сообщения
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
  * Default конструктор, просто инициализирует поля нулями
  */
  Body() : buffLen(0) {};

  /**
  * Конструктор для Body, инициализирует поля структуры реальными данными.
  * Копирует даннуе из буфера к себе
  *
  * @param data   данные в теле сообщения
  * @param len    длинна буфера data
  */
  Body(uint8_t* data, int len)
    : buff(0), buffLen(0)
  {
    setBuffer(data, len);
  };

  /**
  * Конструктор копирования, используется для создания тела по образцу
  *
  * @param body   образец тела.
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
  };

  /**
  * Переопределённый оператор '=',
  * используется для копирования тел сообщений друг в друга
  *
  * @param body   Правая часть оператора '='
  * @return ссылку на себя
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
  * Метод декодирует тело сообщения.
  * Выделяет память, после использования её необходимо освободить.
  * Пока не имплементированно
  *
  * @return декодированное тело сообщения. Сейчас NULL.
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
      // Need to implement this functionality.
      return getIntProperty(Tag::SMPP_SM_LENGTH);
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

  const string& getStrProperty(int tag)const
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

  bool hasIntProperty(int tag)const
  {
    __require__((tag>>8)==SMS_INT_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    return prop.properties[tag].isSet!=0;
  }

  bool hasStrProperty(int tag)const
  {
    __require__((tag>>8)==SMS_STR_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    return prop.properties[tag].isSet!=0;
  }

  bool hasBinProperty(int tag)const
  {
    __require__((tag>>8)==SMS_BIN_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    if ( tag == unType(Tag::SMPP_SHORT_MESSAGE) ) {
      //return prop.properties[Tag::SMSC_RAW_SHORTMESSAGE].isSet!=0;;
      tag = unType(Tag::SMSC_RAW_SHORTMESSAGE);
    }
    return prop.properties[tag].isSet!=0;
  }

  void dropProperty(int tag)
  {
    __require__((tag>>8)==SMS_BIN_TAG);
    tag&=0xff;
    __require__(tag<=SMS_LAST_TAG);
    prop.properties[tag].~OptionalProperty();
    new(&prop.properties[tag]) OptionalProperty();
  }
};


/**
* Структура описывающая SMS в контексте операций
* SMS ценра. Используется системой хранения.
*
* @author Victor V. Makarov
* @version 1.0
* @see MessageStore
*/
struct SMS
{
  State       state;
  time_t      submitTime;     // Время/Дата поступления на SMSC
  time_t      validTime;      // Время/Дата до которого сообщение валидно

  uint32_t    attempts;       // Количество неуспешных попыток доставки
  uint32_t    lastResult;     // Результат последней попытки
  time_t      lastTime;       // Время/Дата последней попытки доставки
  time_t      nextTime;       // Время/Дата слудующей попытки доставки

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

  Body        messageBody;    // Тело сообщения + PDU поля.
  bool        attach;

  RouteId     routeId;        // Additional fields
  int32_t     serviceId;      // Additional fields
  int32_t     priority;       // Additional fields

  SmeSystemIdType srcSmeId;
  SmeSystemIdType dstSmeId;

  /**
  * Default конструктор, просто инициализирует поле state как ENROUTE
  * и прочие поля дефолтными значениями
  */
  SMS() : state(ENROUTE), submitTime(0), validTime(0),
    attempts(0), lastResult(0), lastTime(0), nextTime(0),
    messageReference(0), needArchivate(true),
    deliveryReport(0), billingRecord(0), attach(false),
    serviceId(0), priority(0)
  {
    eServiceType[0]='\0'; routeId[0]='\0';
    srcSmeId[0]='\0'; dstSmeId[0]='\0';
  };


  /**
  * Конструктор копирования, используется для создания
  * SMS по образцу
  *
  * @param sms    образец SMS
  */
  SMS(const SMS& sms) :
    state(sms.state),
    submitTime(sms.submitTime),
    validTime(sms.validTime),
    attempts(sms.attempts),
    lastResult(sms.lastResult),
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
    priority(sms.priority)
  {
    strncpy(eServiceType, sms.eServiceType, sizeof(eServiceType));
    strncpy(routeId, sms.routeId, sizeof(routeId));
    strncpy(srcSmeId, sms.srcSmeId, sizeof(srcSmeId));
    strncpy(dstSmeId, sms.dstSmeId, sizeof(dstSmeId));
  };

  /**
  * Переопределённый оператор '=',
  * используется для копирования сообщений
  *
  * @param sms   Правая часть оператора '='
  * @return ссылку на себя
  */
  SMS& operator =(const SMS& sms)
  {
    state = sms.state;
    submitTime = sms.submitTime;
    validTime = sms.validTime;
    attempts = sms.attempts;
    lastResult = sms.lastResult;
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

    strncpy(eServiceType, sms.eServiceType, sizeof(eServiceType));
    strncpy(routeId, sms.routeId, sizeof(routeId));
    strncpy(srcSmeId, sms.srcSmeId, sizeof(srcSmeId));
    strncpy(dstSmeId, sms.dstSmeId, sizeof(dstSmeId));

    return (*this);
  };

  /**
  * Возвращает состояние сообщения
  *
  * @return состояние сообщения
  */
  inline State getState() const
  {
    return state;
  };

  /**
  * Устанавливает адрес отправителя.
  * Копирует адрес во внутренние структуры
  *
  * @param length длинна адреса (0 < length <= MAX_ADDRESS_VALUE_LENGTH)
  * @param type   тип адреса
  * @param plan   план нумерации адреса
  * @param buff   значение адреса
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
  * Устанавливает адрес отправителя
  * Копирует адрес во внутренние структуры
  *
  * @param address новый адрес отправителя
  * @see Address
  */
  inline void setOriginatingAddress(const Address& address)
  { // Copies address value from 'address' to static structure
    originatingAddress = address;
  };

  /**
  * Возвращает адрес отправителя
  *
  * @return адрес отправителя
  * @see Address
  */
  inline const Address& getOriginatingAddress() const
  {
    return originatingAddress;
  };

  /**
  * Возвращает адрес отправителя
  *
  * @return адрес отправителя
  * @see Address
  */
  inline Address& getOriginatingAddress()
  {
    return originatingAddress;
  };

  /**
  * Устанавливает адрес получателя
  * Копирует адрес во внутренние структуры
  *
  * @param length длинна адреса (0 < length <= MAX_ADDRESS_VALUE_LENGTH)
  * @param type   тип адреса
  * @param plan   план нумерации адреса
  * @param buff   значение адреса
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
  * Устанавливает адрес получателя
  * Копирует адрес во внутренние структуры
  *
  * @param address новый адрес получателя
  * @see Address
  */
  inline void setDestinationAddress(const Address& address)
  { // Copies address value from 'address' to static structure
    destinationAddress = address;
  };

  /**
  * Возвращает адрес получателя
  *
  * @return адрес получателя
  * @see Address
  */
  inline const Address& getDestinationAddress() const
  {
    return destinationAddress;
  };

  /**
  * Возвращает адрес получателя
  *
  * @return адрес получателя
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
  * Устанавливает дескриптор отправителя
  * Копирует адрес во внутренние структуры
  *
  * @param descriptor новый дескриптор отправителя
  * @see Descriptor
  */
  inline void setOriginatingDescriptor(const Descriptor& descriptor)
  { // Copies descriptor from 'descriptor' to static structure
    originatingDescriptor = descriptor;
  };

  /**
  * Устанавливает дескриптор отправителя
  * Копирует адрес во внутренние структуры
  *
  * @param _mscLen   длинна буфера _msc
  * @param _value значение адреса MSC
  * @param _imsiLen   длинна буфера _imsi
  * @param _value значение адреса IMSI
  * @param _sme номер SME
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
  * Возвращает дескриптор отправителя
  *
  * @return дескриптор отправителя
  * @see Descriptor
  */
  inline const Descriptor& getOriginatingDescriptor() const
  {
    return originatingDescriptor;
  };

  /**
  * Возвращает дескриптор отправителя
  *
  * @return дескриптор отправителя
  * @see Descriptor
  */
  inline Descriptor& getOriginatingDescriptor()
  {
    return originatingDescriptor;
  };

  /**
  * Устанавливает дескриптор получателя
  * Копирует адрес во внутренние структуры
  *
  * @param descriptor новый дескриптор получателя
  * @see Descriptor
  *
  inline void setDestinationDescriptor(const Descriptor& descriptor)
  { // Copies descriptor from 'descriptor' to static structure
      destinationDescriptor = descriptor;
  };*/

  /**
  * Возвращает дескриптор получателя
  *
  * @return дескриптор получателя
  * @see Descriptor
  */
  inline const Descriptor& getDestinationDescriptor() const
  {
    return destinationDescriptor;
  };

  /**
  * Возвращает дескриптор получателя
  *
  * @return дескриптор получателя
  * @see Descriptor
  *
  inline Descriptor& getDestinationDescriptor()
  {
  return destinationDescriptor;
    };*/


  /**
  * Устанавливает время валидности сообщения.
  *
  * @param time   дата, до которой сообщение валидно
  *               (не интервал времени).
  */
  inline void setValidTime(time_t time)
  {
    validTime = time;
  };

  /**
  * Возвращает время валидности сообщения.
  *
  * @return time   дата, до которой сообщение валидно
  *         (не интервал времени).
  */
  inline time_t getValidTime() const
  {
    return validTime;
  };

  /**
  * Устанавливает время поступления сообщения в SMSC
  *
  * @param time   время поступления сообщения в SMSC
  */
  inline void setSubmitTime(time_t time)
  {
    submitTime = time;
  };

  /**
  * Возвращает время поступления сообщения в SMSC
  *
  * @return время поступления сообщения в SMSC
  */
  inline time_t getSubmitTime() const
  {
    return submitTime;
  };

  /**
  * Устанавливает время последней попытки доставки сообщения из SMSC
  *
  * @param time   время последней попытки доставки сообщения из SMSC
  *
  inline void setLastTime(time_t time)
  {
      lastTime = time;
  };*/

  /**
  * Возвращает время последней попытки доставки сообщения из SMSC
  *
  * @return время последней попытки доставки сообщения из SMSC
  */
  inline time_t getLastTime() const
  {
    return lastTime;
  };

  /**
  * Устанавливает время следующей попытки доставки сообщения из SMSC
  *
  * @param time   время следующей попытки доставки сообщения из SMSC
  */
  inline void setNextTime(time_t time)
  {
    nextTime = time;
  };

  /**
  * Возвращает время следующей попытки доставки сообщения из SMSC
  *
  * @return время следующей попытки доставки сообщения из SMSC
  */
  inline time_t getNextTime() const
  {
    return nextTime;
  };

  /**
  * Устанавливает номер сообщения (MR),
  * для идентификации сообщений приходящих с одного адреса
  *
  * @param mr     идентификационный номер сообщения (MR)
  */
  inline void setMessageReference(uint16_t mr)
  {
    messageReference = mr;
  };

  /**
  * Возвращает номер сообщения (MR),
  * для идентификации сообщений приходящих с одного адреса
  *
  * @return идентификационный номер сообщения (MR)
  */
  inline uint16_t getMessageReference() const
  {
    return messageReference;
  };

  /**
  * Устанавливает тип отчета о доставке сообщения
  *
  * @param req    тип отчета о доставке сообщения
  */
  inline void setDeliveryReport(uint8_t report)
  {
    deliveryReport = report;
  };

  /**
  * Возвращает тип отчета о доставке сообщения
  *
  * @return      тип отчета о доставке сообщения
  */
  inline uint8_t getDeliveryReport() const
  {
    return deliveryReport;
  };

  /**
  * Устанавливает тип биллинга сообщения
  *
  * @param req    тип биллинга сообщения
  */
  inline void setBillingRecord(uint8_t billing)
  {
    billingRecord = billing;
  };

  /**
  * Возвращает тип биллинга сообщения
  *
  * @return      тип биллинга сообщения
  */
  inline uint8_t getBillingRecord() const
  {
    return billingRecord;
  };

  /**
  * Устанавливает признак, нужно ли архивировать сообщение после
  * успешной или неуспешной попытке доставки
  *
  * @param arc    признак, нужно ли архивировать сообщение
  */
  inline void setArchivationRequested(bool arc)
  {
    needArchivate = arc;
  };

  /**
  * Возвращает признак, нужно ли архивировать сообщение после
  * успешной или неуспешной попытке доставки
  *
  * @return признак, нужно ли нужно ли архивировать сообщение (да / нет)
  */
  inline bool isArchivationRequested() const
  {
    return needArchivate;
  };

  /**
  * Возвращает причину в случае
  * отказа/некорректности/недоставки последней попытки доставки сообщения
  *
  * @return причина отказа/некорректности/недоставки сообщения
  */
  inline uint32_t getLastResult() const
  {
    return lastResult;
  };

  /**
  * Устанавливает количество неудач при доставке сообщения
  *
  * @param count  количество неудач при доставке сообщения
  *
  inline void setAttemptsCount(uint32_t count)
  {
  attempts = count;
  };*/

  /**
  * Возвращает количество неудач при доставке сообщения
  *
  * @return количество неудач при доставке сообщения
  */
  inline uint32_t getAttemptsCount() const
  {
    return attempts;
  };

  /**
  * Возвращает тело сообщения
  *
  * @return тело сообщения
  * @see Body
  */
  inline const Body& getMessageBody() const
  {
    return messageBody;
  };

  /**
  * Возвращает тело сообщения
  *
  * @return тело сообщения
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
  * Метод устанавливает имя-тип сервиса SME.
  *
  * @param type имя-тип SME
  */
  inline void setEServiceType(const char* type)
  {
      setStringField(eServiceType, type, MAX_ESERVICE_TYPE_LENGTH);
  };
  /**
  * Метод возвращает имя-тип сервиса
  *
  * @param type   указатель на буфер куда будет скопированно имя
  *               буфер должен иметь размер не меньше
  *               MAX_ESERVICE_TYPE_LENGTH+1, чтобы принять
  *               любое значение
  */
  inline void getEServiceType(char* type) const
  {
      getStringField(eServiceType, type, MAX_ESERVICE_TYPE_LENGTH);
  };
  /**
  * Метод возвращает имя-тип сервиса
  *
  * @return имя-тип сервиса
  */
  inline const char* getEServiceType() const
  {
      return eServiceType;
  };

  /**
  *
  * Устанавливает приоритет сообщения.
  *
  * @param pri    приоритет сообщения
  */
  inline void setPriority(int32_t pri) {
      priority = pri;
  };
  /**
  *
  * Возвращает приоритет сообщения.
  *
  * @return приоритет сообщения
  */
  inline int32_t getPriority() const {
      return priority;
  };

  /**
  *
  * Устанавливает идентификатор сервиса.
  *
  * @param id    идентификатор сервиса
  */
  inline void setServiceId(int32_t id) {
      serviceId = id;
  };
  /**
  *
  * Возвращает идентификатор сервиса.
  *
  * @return идентификатор сервиса
  */
  inline uint32_t getServiceId() const {
      return serviceId;
  };

  /**
  * Метод устанавливает идентификатор маршрута.
  *
  * @param route идентификатор маршрута
  */
  inline void setRouteId(const char* route)
  {
      setStringField(routeId, route, MAX_ROUTE_ID_TYPE_LENGTH);
  };
  /**
  * Метод возвращает идентификатор маршрута
  *
  * @param route  указатель на буфер куда будет скопирован
  *               идентификатор маршрута
  *               буфер должен иметь размер не меньше
  *               MAX_ROUTE_ID_TYPE_LENGTH+1
  */
  inline void getRouteId(char* route) const
  {
      getStringField(routeId, route, MAX_ROUTE_ID_TYPE_LENGTH);
  };
  /**
  * Метод возвращает идентификатор маршрута
  *
  * return идентификатор маршрута
  */
  inline const char* getRouteId() const
  {
    return routeId;
  };

  /**
  * Метод устанавливает идентификатор sme-источника.
  *
  * @param id идентификатор sme-источника
  */
  inline void setSourceSmeId(const char* id)
  {
      setStringField(srcSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
  };
  /**
  * Метод возвращает идентификатор sme-источника
  *
  * @param id     указатель на буфер куда будет скопирован
  *               идентификатор sme-источника
  *               буфер должен иметь размер не меньше
  *               MAX_SMESYSID_TYPE_LENGTH+1
  */
  inline void getSourceSmeId(char* id) const
  {
      getStringField(srcSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
  };
  /**
  * Метод возвращает идентификатор sme-источника
  *
  * return идентификатор sme-источника
  */
  inline const char* getSourceSmeId() const
  {
    return srcSmeId;
  };

  /**
  * Метод устанавливает идентификатор sme-приёмника.
  *
  * @param id идентификатор sme-приёмника
  */
  inline void setDestinationSmeId(const char* id)
  {
    setStringField(dstSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
  };

  /**
  * Метод возвращает идентификатор sme-приёмника
  *
  * @param id     указатель на буфер куда будет скопирован
  *               идентификатор sme-приёмника
  *               буфер должен иметь размер не меньше
  *               MAX_SMESYSID_TYPE_LENGTH+1
  */
  inline void getDestinationSmeId(char* id) const
  {
      getStringField(dstSmeId, id, MAX_SMESYSID_TYPE_LENGTH);
  };
  /**
  * Метод возвращает идентификатор sme-приёмника
  *
  * return идентификатор sme-приёмника
  */
  inline const char* getDestinationSmeId() const
  {
    return dstSmeId;
  };

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
  const string& getStrProperty(int tag)
  {
    return messageBody.getStrProperty(tag);
  }
  const char* getBinProperty(int tag,unsigned* len)
  {
    return messageBody.getBinProperty(tag,len);
  }
  uint32_t getIntProperty(int tag)
  {
    return messageBody.getIntProperty(tag);
  }
  bool hasIntProperty(int tag)
  {
    return messageBody.hasIntProperty(tag);
  }
  bool hasStrProperty(int tag)
  {
    return messageBody.hasStrProperty(tag);
  }
  bool hasBinProperty(int tag)
  {
    return messageBody.hasBinProperty(tag);
  }

  ~SMS() {}
};

};//sms
};//smsc

#endif
