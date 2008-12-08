#ifndef __SCAG_PERS_ABNTADDR_HPP__
#define __SCAG_PERS_ABNTADDR_HPP__

#include <string>
#include <vector>

#include <sms/sms.h>
#include <sms/sms_const.h>
#include <util/crc32.h>

#include "scag/util/storage/Serializer.h"

namespace scag { namespace pers { namespace util {

using std::string;
using smsc::util::crc32;
using std::runtime_error;

class AbntAddr
{
private:
    static const uint32_t ADDRESS_VALUE_SIZE = 8;

    union AbntAddrValue
    {
        uint8_t full_addr[ADDRESS_VALUE_SIZE+3];
        struct _addr_content
        {
            uint8_t length;
            uint8_t plan;
            uint8_t type;
            uint8_t signals[ADDRESS_VALUE_SIZE];
        } addr_content;
    };

public:

    AbntAddr() : number(0) { memset((void*)&value, 0x00, sizeof(value));}
  AbntAddr(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
  {
    value.addr_content.type = _type;
    value.addr_content.plan = _plan;
    setValue(_len, _value);
  };

    /*
     * // extra constructors are EVIL!
  AbntAddr(const AbntAddrValue* _full_addr)
  {
    if (!_full_addr) {
      throw runtime_error("AbntAddr: bad address NULL");
    }
    memcpy((void*)&value.full_addr, (void*)_full_addr, sizeof(value.full_addr));
  };
  AbntAddr(const uint8_t* _full_addr)
  {
    if (!_full_addr) {
      throw runtime_error("AbntAddr: bad address NULL");
    }
    memcpy((void*)&value.full_addr, (void*)_full_addr, sizeof(value.full_addr));
  };
     */

    AbntAddr(const AbntAddr& addr) : number(addr.number)
    {
        memcpy((void*)&(value.full_addr), (void*)&(addr.value.full_addr), sizeof(value.full_addr));
    };

  ~AbntAddr(){};

  AbntAddr(const char* text)
  {
    setAddress(text);
  }

  AbntAddr(const smsc::sms::Address& addr)
  {
    value.addr_content.type = (uint8_t)addr.type;
    value.addr_content.plan = (uint8_t)addr.plan;
    value.addr_content.length = addr.length;
    setValue(addr.length, (char*)&addr.value);
  };

	
  AbntAddr& operator=(const AbntAddr& addr)
  {
    if(&addr == this)
      return (*this);
		
      number = addr.number;
    memcpy((void*)&(value.full_addr), (void*)&(addr.value.full_addr), sizeof(value.full_addr));
    return (*this);
  };

  inline int operator ==(const AbntAddr& addr)const
  {
      // we only compare first 3 bytes of full_addr: length, plan, type
      return ( number == addr.number && memcmp(value.full_addr,addr.value.full_addr,3) == 0 );
      // return (memcmp((void*)&value, (void*)&addr.value, sizeof(value)) == 0);
  };

  inline int operator !=(const AbntAddr& addr)const
  {
    return !(*this==addr);
  };

  bool operator < (const AbntAddr& addr)const
  {
      return ( number < addr.number || 
               ( number == addr.number && memcmp(value.full_addr,addr.value.full_addr,3) < 0) );
      // return (memcmp((void*)&value, (void*)&addr.value, sizeof(value)) < 0);
  }

    /*
  const uint8_t* getAddrSig(void) const
  {
    return (uint8_t*)(value.full_addr);
  }
     */

  /**
   * Метод копирует значение адреса и возвращает его длинну
   *
   * @param _value указатель на буфер куда будет скопированно значение адреса
   *               буфер должен иметь размер не меньше
   *               MAX_ADDRESS_VALUE_LENGTH+1, чтобы принять любое значение
   * @return длинна адреса
   */
  inline uint8_t getSignals(char* _value) const
  {
    //__require__(_value);
    if (!_value) {
      return 0;
    }
    if (value.addr_content.length)
    {
      for(int i=0; i<value.addr_content.length; i++)
        _value[i] = "0123456789"[i%2 ? value.addr_content.signals[i/2] >> 4 :value.addr_content.signals[i/2] & 0x0F];
      _value[value.addr_content.length] = '\0';
    }
    return value.addr_content.length;
  }

  /**
   * Возвращает длинну адреса
   *
   * @return длинна адреса
   */
  inline uint8_t getLength() const
  {
    return value.addr_content.length;
  };

  inline void setTypeOfNumber(uint8_t _type)
  {
    value.addr_content.type = _type;
  };

  inline uint8_t getTypeOfNumber() const
  {
    return value.addr_content.type;
  };

  inline void setNumberingPlan(uint8_t _plan)
  {
    value.addr_content.plan = _plan;
  };

  inline uint8_t getNumberingPlan() const
  {
    return value.addr_content.plan;
  };

  smsc::sms::Address getAddress(void) const
  {
      return smsc::sms::Address(getText().c_str());
  };

  inline std::string getText(void) const
  {
    char vl[32];
    char buf[48];
    if(getSignals(vl))
    {
      if ( value.addr_content.type == 1 && value.addr_content.plan == 1 )
        snprintf(buf,sizeof(buf),"+%s",vl);
      else if (value.addr_content.type == 0 && value.addr_content.plan == 1)
        snprintf(buf,sizeof(buf),"%s",vl);
      else
        snprintf(buf,sizeof(buf),".%d.%d.%s",value.addr_content.type,value.addr_content.plan,vl);
    }
    else
      buf[0]=0;
    return buf;
  }

  inline std::string toString()const
  {
    if(value.addr_content.length>32)abort();
    char vl[32];
    char buf[48];
    if(getSignals(vl))
      snprintf(buf,sizeof(buf),".%d.%d.%s",value.addr_content.type,value.addr_content.plan,vl);
    else
      buf[0]=0;
    return buf;
  }

  void Clear()
  {
      number = 0;
    memset((void*)&value, 0x00, sizeof(value));
  }
	
  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res = crc32(0, value.full_addr, ADDRESS_VALUE_SIZE+3);
    for(; attempt > 0; attempt--) res = crc32(res, value.full_addr, ADDRESS_VALUE_SIZE+3);
    return res;
  }
  static uint32_t CalcHash(AbntAddr key)
  {
      return  crc32(0, key.value.full_addr, ADDRESS_VALUE_SIZE+3);
  }

  uint64_t getNumber() const {
    return number;
  }

  void setAddress(const char* address) {
    if(!address || !*address)
    {
      throw runtime_error("AbntAddr::setAddress: bad address NULL");
    }
    smsc::sms::AddressValue addr_value;
    int iplan,itype;
    memset(addr_value,0,sizeof(addr_value));
    int scaned = sscanf(address,".%d.%d.%15s", &itype, &iplan, addr_value);
    if (scaned != 3)
    {
      scaned = sscanf(address, "+%15[0123456789?]s", addr_value);
      if (scaned)
      {
        iplan = 1;//ISDN
        itype = 1;//INTERNATIONAL
      }
      else
      {
        scaned = sscanf(address, "%15[0123456789?]s", addr_value);
        if (!scaned)
        {
          throw runtime_error(string("AbntAddr::setAddress: bad address ") + address);
        }
        iplan = 1;//ISDN
        itype = 1;//INTERNATIONAL
        //itype = 0;//UNKNOWN
      }
    }
    value.addr_content.type = (uint8_t)itype;
    value.addr_content.plan = (uint8_t)iplan;
    value.addr_content.length = static_cast<uint8_t>(strlen(addr_value));
    setValue(value.addr_content.length, addr_value);
  }

    /*
  void setAddrValue(const char* val) {
    if (!val) {
      return;
    }
    memcpy(value.full_addr, val, ADDRESS_VALUE_SIZE);
  }
     */

    // necessary for serialization
  const char* getContentSignals() const {
    return (const char*)value.addr_content.signals;
  }

    // from deserialization
  void setAllValues(uint8_t len, uint8_t plan, uint8_t type, const char* signals) {
    value.addr_content.length = len;
    value.addr_content.plan = plan;
    value.addr_content.type = type;
    memcpy(value.addr_content.signals, signals, getValueSize());
  }

    // for serialization
  uint32_t getValueSize() const {
     return ADDRESS_VALUE_SIZE;
  }

protected:
  inline void setValue(uint8_t _len, const char* _value)
  {
    if (!_len || !_value || !*_value) {
      throw runtime_error("AbntAddr::setValue: bad address NULL");
    }
    if ((_len >= ADDRESS_VALUE_SIZE * 2) || (!isdigit(_value[_len - 1]))) {
      throw runtime_error(string("AbntAddr::setValue: bad address ") + _value);
    }
    number = atoll(_value);
    if (number == 0) {
      throw runtime_error(string("AbntAddr::setValue: bad address ") + _value);
    }

    memset((void*)&value.addr_content.signals, 0xFF, sizeof(value.addr_content.signals));
    uint8_t	sig1, sig2, i;

    for(i = 0; i < _len>>1; i++)
    {
      if (!isdigit(_value[i*2]) || !isdigit(_value[i*2+1])) {
        throw runtime_error(string("AbntAddr::setValue: bad address ") + _value);
      }
      sig1 = _value[i*2] - 0x30;
      sig2 = _value[i*2+1] - 0x30;
      value.addr_content.signals[i] = (sig2 << 4) | sig1;
    }
    if( ((_len>>1)<<1) != _len)
      value.addr_content.signals[i] = 0xF0 | (uint8_t)(_value[i*2] - 0x30);

    value.addr_content.length = _len;
  };


private:
  AbntAddrValue value;
  uint64_t number;
};

} //util
} //pers
} //scag

inline scag::util::storage::Serializer& operator << (scag::util::storage::Serializer& ser, 
                                                     const scag::pers::util::AbntAddr& addr) { 
  uint8_t len = addr.getLength() & 0x1F;
  uint8_t plan = (addr.getNumberingPlan() & 0x07) << 5;
  uint8_t res = len | plan;
  ser << res;
  ser << addr.getTypeOfNumber();

  ser.writeAsIs(addr.getValueSize(), (const char*)(addr.getContentSignals()));
  return ser; 
};

inline scag::util::storage::Deserializer& operator >> (scag::util::storage::Deserializer& deser,
                                                 scag::pers::util::AbntAddr& addr) { 
  uint8_t lenplan = 0;
  deser >> lenplan;
  uint8_t type = 0;
  deser >> type;
  uint8_t len = lenplan & 0x1F;
  uint8_t plan = (lenplan >> 5) & 0x07;
  const char* buf = deser.readAsIs(addr.getValueSize());
  addr.setAllValues(len, plan, type, buf);
  return deser;
};


#endif

