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
using smsc::sms::Address;
using smsc::sms::AddressValue;
using smsc::util::crc32;

using std::runtime_error;

static const uint32_t ADDRESS_VALUE_SIZE = 10;

union AbntAddrValue
{
  uint8_t full_addr[ADDRESS_VALUE_SIZE];
  struct _addr_content
  {
    uint8_t length:5;
    uint8_t plan:3;
    uint8_t type;
    uint8_t signals[8];
  } addr_content;
};


class AbntAddr
{
public:

  AbntAddr() { memset((void*)&value, 0x00, sizeof(value));}
  AbntAddr(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
  {
    value.addr_content.type = _type;
    value.addr_content.plan = _plan;
    setValue(_len, _value);
  };

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

  AbntAddr(const AbntAddr& addr)
  {
    memcpy((void*)&(value.full_addr), (void*)&(addr.value.full_addr), sizeof(value.full_addr));
  };

  ~AbntAddr(){};

  AbntAddr(const char* text)
  {
    setAddress(text);
  }

  AbntAddr(const Address& addr)
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
		
    memcpy((void*)&(value.full_addr), (void*)&(addr.value.full_addr), sizeof(value.full_addr));
    return (*this);
  };

  inline int operator ==(const AbntAddr& addr)const
  {
    return (memcmp((void*)&value, (void*)&addr.value, sizeof(value)) == 0);
  };

  inline int operator !=(const AbntAddr& addr)const
  {
    return !(*this==addr);
  };

  bool operator<(const AbntAddr& addr)const
  {
    return (memcmp((void*)&value, (void*)&addr.value, sizeof(value)) < 0);
  }

  inline void setValue(uint8_t _len, const char* _value)
  {
    if (!_len || !_value || !*_value) {
      throw runtime_error("AbntAddr::setValue: bad address NULL");
    }
    if ((_len >= sizeof(value.addr_content.signals) * 2) || (!isdigit(_value[_len - 1]))) {
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

  const uint8_t* getAddrSig(void) const
  {
    return (uint8_t*)&(value.full_addr);
  }

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

  Address getAddress(void) const
  {
    return Address(getText().c_str());
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
    memset((void*)&value, 0x00, sizeof(value));
  }
	
  uint32_t HashCode(uint32_t attempt)const
  {
    uint32_t res = crc32(0, value.full_addr, sizeof(value.full_addr));
    for(; attempt > 0; attempt--) res = crc32(res, value.full_addr, sizeof(value.full_addr));
    return res;
  }
  static uint32_t CalcHash(AbntAddr key)
  {
      return  crc32(0, key.value.full_addr, sizeof(key.value.full_addr));
  }

  uint64_t getNumber() const {
    return number;
  }

  void setAddress(const char* address) {
    if(!address || !*address)
    {
      throw runtime_error("AbntAddr::setAddress: bad address NULL");
    }
    AddressValue addr_value;
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

  void setAddrValue(const char* val) {
    if (!val) {
      return;
    }
    memcpy(value.full_addr, val, ADDRESS_VALUE_SIZE);
  }

  uint32_t getValueSize() const {
    return ADDRESS_VALUE_SIZE;
  }

private:
  AbntAddrValue value;
  uint64_t number;
};

} //util
} //pers
} //scag

inline scag::util::storage::Serializer& operator << (scag::util::storage::Serializer& ser, 
                                                     const scag::pers::util::AbntAddr& addr) { 
  ser.writeAsIs(addr.getValueSize(), (const char*)(addr.getAddrSig()));
  return ser; 
};

inline scag::util::storage::Deserializer& operator >> (scag::util::storage::Deserializer& deser,
                                                 scag::pers::util::AbntAddr& addr) { 
  const char* buf = deser.readAsIs(addr.getValueSize());
  addr.setAddrValue(buf);
  return deser;
};


#endif

