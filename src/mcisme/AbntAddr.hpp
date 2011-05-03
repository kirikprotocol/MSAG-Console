//------------------------------------
//  AbntAddr.hpp
//  Routman Michael, 2005-2006
//------------------------------------
//
//	���� �������� �������� ������ AbntAddr.
//

#ifndef ___ABNTADDR_H
#define ___ABNTADDR_H

#include <string>
#include <sms/sms.h>
#include <sms/sms_const.h>
#include <util/crc32.h>
#include <vector>
#include <sstream>

#include "util/Exception.hpp"

namespace smsc {
namespace mcisme {

using std::string;
using sms::Address;
using sms::AddressValue;
using smsc::util::crc32;

std::string
hexdmp(const uchar_t* buf, size_t bufSz);

union	AbntAddrValue
{
  uint8_t full_addr[10];
  struct	_addr_content
  {
    uint8_t     length:5;
    uint8_t     plan:3;
    uint8_t type;
    uint8_t signals[8];
  } addr_content;
};

class BadAddrException : public util::Exception {
public:
  BadAddrException(const char * fmt, ...)
  : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

class AbntAddr
{
  AbntAddrValue value;

public:

  AbntAddr(){memset((void*)&value, 0x00, sizeof(value));}

  AbntAddr(uint8_t _len, uint8_t _type, uint8_t _plan, const char* _value)
  {
    value.addr_content.type = _type;
    value.addr_content.plan = _plan;
    setValue(_len, _value);
  };

  AbntAddr(const AbntAddrValue* _full_addr)
  {
    __require__(_full_addr);
    memcpy((void*)&value.full_addr, (void*)_full_addr, sizeof(value.full_addr));
    checkAddrValueValidity(value.addr_content.signals, sizeof(value.addr_content.signals));
  };

  AbntAddr(const uint8_t* _full_addr)
  {
    __require__(_full_addr);
    memcpy((void*)&value.full_addr, (void*)_full_addr, sizeof(value.full_addr));
    checkAddrValueValidity(value.addr_content.signals, sizeof(value.addr_content.signals));
  };

  AbntAddr(const AbntAddr& addr)
  {
    memcpy((void*)&(value.full_addr), (void*)&(addr.value.full_addr), sizeof(value.full_addr));
  };

  ~AbntAddr(){};

  AbntAddr(const char* text)
  {
    if(!text || !*text)
    {
      Clear();
    }
    else
    {
      AddressValue addr_value;
      int iplan,itype;
      memset(addr_value,0,sizeof(addr_value));
      int scaned = sscanf(text,".%d.%d.%20s",	&itype,	&iplan,	addr_value);
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
          {
            Clear();
            return;
          }
          else
          {
            iplan = 1;//ISDN
            itype = 0;//UNKNOWN
          }
        }
      }
      value.addr_content.type = (uint8_t)itype;
      value.addr_content.plan = (uint8_t)iplan;
      value.addr_content.length = static_cast<uint8_t>(strlen(addr_value));
      setValue(static_cast<uint8_t>(strlen(addr_value)), addr_value);
    }
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
    __require__(_len && _value && _value[0]
                && _len < sizeof(value.addr_content.signals)*2);

    memset((void*)&value.addr_content.signals, 0xFF, sizeof(value.addr_content.signals));

    uint8_t	sig1, sig2, i;

    for(i = 0; i < _len>>1; i++)
    {
      sig1 = _value[i*2] - 0x30;
      sig2 = _value[i*2+1] - 0x30;
      if ( sig1 > 9 || sig2 > 9 )
        throw BadAddrException("AbntAddr::setValue::: invalid input address value [%s]", _value);
      value.addr_content.signals[i] = (sig2 << 4) | sig1;
    }
    if( _len & 0x01 ) {
      sig1 = _value[i*2] - 0x30;
      if ( sig1 > 9 )
        throw BadAddrException("AbntAddr::setValue::: invalid input address value [%s]", _value);
      value.addr_content.signals[i] = 0xF0 | sig1;
    }
    value.addr_content.length = _len;
  }

  const uint8_t* getAddrSig(void) const
  {
    return (uint8_t*)&(value.full_addr);
  }

  /**
   * ����� �������� �������� ������ � ���������� ��� ������
   *
   * @param _value ��������� �� ����� ���� ����� ������������ �������� ������
   *               ����� ������ ����� ������ �� ������
   *               MAX_ADDRESS_VALUE_LENGTH+1, ����� ������� ����� ��������
   * @return ������ ������
   */
  inline uint8_t getSignals(char* _value) const
  {
    __require__(_value);

    if (value.addr_content.length)
    {
      for(int i=0; i<value.addr_content.length; i++) {
        uint8_t sig = i%2 ? value.addr_content.signals[i/2] >> 4
                          : value.addr_content.signals[i/2] & 0x0F;
        if ( sig < 0x0a )
          _value[i] = "0123456789"[sig];
        else
          _value[i] = 'f';
      }
      _value[value.addr_content.length] = '\0';
    }
    return value.addr_content.length;
  }

  /**
   * ���������� ������ ������
   *
   * @return ������ ������
   */
  inline uint8_t getLength() const
  {
    return value.addr_content.length;
  }

  inline void setTypeOfNumber(uint8_t _type)
  {
    value.addr_content.type = _type;
  }

  inline uint8_t getTypeOfNumber() const
  {
    return value.addr_content.type;
  }

  inline void setNumberingPlan(uint8_t _plan)
  {
    value.addr_content.plan = _plan;
  }

  inline uint8_t getNumberingPlan() const
  {
    return value.addr_content.plan;
  }

  //inline int getText(char* buf,int buflen)const
  //{
  //	char vl[32];
  //	getSignals(vl);
  //	if ( value.addr_type == 1 && value.num_plan == 1 ){
  //		return snprintf(buf,buflen,"+%s",vl);
  //	}else if (value.addr_type == 0 && value.num_plan == 1){
  //		return snprintf(buf,buflen,"%s",vl);
  //	}else
  //		return snprintf(buf,buflen,".%d.%d.%s",value.addr_type,value.num_plan,vl);
  //}

  Address getAddress(void) const
  {
    return Address(getText().c_str());
  }

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

  //inline int toString(char* buf,int buflen)const
  //{
  //	char vl[32];
  //	getSignals(vl);
  //	return snprintf(buf,buflen,".%d.%d.%s",value.addr_type,value.num_plan,vl);
  //}
  //
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

  void checkAddrValueValidity(uint8_t* addr_signals, size_t addr_signals_sz) {
    for(unsigned i=0; i<addr_signals_sz; ++i) {
      uint8_t sig1 = (addr_signals[i] >> 4) & 0x0F;
      uint8_t sig2 = addr_signals[i] & 0x0F;
      if ( (sig1 > 9 && sig1 != 0x0f) || (sig2 > 9 && sig2 != 0x0f) )
        throw BadAddrException("AbntAddr::checkAddrValueValidity::: invalid input address value [%s]", hexdmp(addr_signals, addr_signals_sz).c_str());
    }
  }
};

struct MCEvent
{
  uint8_t       id;
  time_t        dt;
  AbntAddrValue	caller;
  uint16_t      callCount;
  uint8_t       missCallFlags;

  MCEvent(const uint8_t& _id):id(_id), dt(0), callCount(0), missCallFlags(0) {}
  MCEvent():id(0), dt(0), callCount(0), missCallFlags(0)
  {
    memset((void*)&(caller.full_addr), 0xFF, sizeof(caller.full_addr));
  }

  MCEvent(const MCEvent& e): id(e.id), dt(e.dt), callCount(e.callCount), missCallFlags(e.missCallFlags)
  {
    memcpy((void*)&(caller.full_addr), (void*)&(e.caller.full_addr), sizeof(caller.full_addr));
  }
  MCEvent& operator=(const AbntAddr& addr)
  {
    id = 0; dt = 0; callCount = 1; missCallFlags = 0;
    memcpy((void*)&(caller.full_addr), (void*)addr.getAddrSig(), sizeof(caller.full_addr));
    return *this;
  }

  MCEvent& operator=(const MCEvent& e)
  {
    if(&e == this)
      return *this;

    id = e.id; dt = e.dt; callCount = e.callCount; missCallFlags = e.missCallFlags;
    memcpy((void*)&(caller.full_addr), (void*)&(e.caller.full_addr), sizeof(caller.full_addr));
    return *this;
  }

  const char* toString(char* textString,size_t size) const
  {
    snprintf(textString, size, "id=%d,dt=%d,caller=%s,callCount=%d,missCallFlags=0x%02X", id, dt, AbntAddr(&caller).getText().c_str(), callCount, missCallFlags);

    return textString;
  }
};

struct MCEventOut {
  MCEventOut(const std::string& aCaller, const std::string& aMsg) : caller(aCaller), msg(aMsg) {}

  std::string caller;
  std::string msg;
  std::vector<MCEvent> srcEvents;

  std::string toString() const {
    std::string result;
    result+="caller=[";
    result+=caller;
    result+="],msg=[";
    result+=msg;
    result+="],srcEvents=[";

    char buf[128];
    for(std::vector<MCEvent>::const_iterator iter = srcEvents.begin(), end_iter = srcEvents.end(); iter != end_iter;)
    {
      result+=iter->toString(buf,sizeof(buf));
      if ( ++iter != end_iter )
      {
        result += ",";
      }
    }
    result += "]";

    return result;
  }
};

};	//  namespace mcisme
};	//  namespace smsc
#endif
