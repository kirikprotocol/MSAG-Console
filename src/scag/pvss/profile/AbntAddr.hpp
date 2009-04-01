#ifndef _SCAG_PVSS_PROFILE_ABNTADDR_HPP_
#define _SCAG_PVSS_PROFILE_ABNTADDR_HPP_

#include <string>
#include <cstdlib>
// #include <vector>
// #include <sms/sms.h>
// #include <sms/sms_const.h>
#include <cstring>
#include <util/crc32.h>
#include "util/byteorder.h"
#include "scag/exc/IOException.h"
#include "scag/util/storage/Serializer.h"

namespace scag2 {
namespace pvss {

class AbntAddr
{
public:
    static bool getAllowNewPacking() { return allowNewPacking_; }
    static void setAllowNewPacking( bool yes ) { allowNewPacking_ = yes; }

private:
    static const unsigned PACKED_SIZE = 8;
    static bool allowNewPacking_;

public:
    // default ctor
    AbntAddr() { number_.number = 0; }
    
    AbntAddr( uint8_t len, uint8_t type, uint8_t plan, const char* val )
    {
        setValue( len, type, plan, val );
    }

    /*
    AbntAddr( const char* text ) 
    {
        fromString( text );
    }
     */

    // default impl is fine.
    // AbntAddr( const AbntAddr& );
    // AbntAddr& operator = ( const AbntAddr& );
    // ~AbntAddr() {}

    /*
    AbntAddr( const smsc::sms::Address& addr )
    {
        tp_.data.type = addr.type;
        tp_.data.plan = addr.plan;
        setValue( addr.length, addr.value );
    }
     */


    // === operators
    inline bool operator == ( const AbntAddr& addr ) const
    {
        return number_.number == addr.number_.number;
    }


    inline bool operator != ( const AbntAddr& addr ) const
    {
        return !(*this == addr);
    }


    inline bool operator < ( const AbntAddr& addr ) const
    {
        return number_.number < addr.number_.number;
    }


    inline void setTypeOfNumber( uint8_t type ) {
        number_.data.ton = type;
    }


    inline uint8_t getTypeOfNumber() const {
        return number_.data.ton;
    }


    inline void setNumberingPlan( uint8_t plan ) {
        number_.data.npi = plan;
    }


    inline uint8_t getNumberingPlan() const {
        return number_.data.npi;
    }


    inline uint8_t getLength() const {
        return number_.data.len;
    }


    inline uint64_t getNumber() const {
        return number_.data.value;
    }


    // NOTE: buf must be large enough
    // @return the length of the address
    static unsigned unpack( char* buf, const char* from )
    {
        const bool oldformat = (from[7] == char(0xff));
        unsigned pos = 0;
        unsigned char h, l, c;
        for ( unsigned idx = 0; idx < PACKED_SIZE-1; ++idx ) {
            c = static_cast<unsigned char>(*from++);
            if ( oldformat ) {
                l = c >> 4;
                h = c & 0xf;
            } else {
                h = c >> 4;
                l = c & 0xf;
            }
            if ( h > 0x9 ) break;
            buf[pos++] = char(h+0x30);
            if ( l > 0x9 ) break;
            buf[pos++] = char(l+0x30);
        }
        buf[pos] = '\0';
        return pos;
    }
    

    /// pack into buffer, buffer must be large enough
    void pack( char* buf ) const
    {
        memset(buf,0xff,PACKED_SIZE);

        bool oldformat = !getAllowNewPacking();
        unsigned len = number_.data.len;
        if ( len > (PACKED_SIZE-1)*2 ) {
            // we cannot distinguish in that case
            oldformat = true;
        }

        if ( len > PACKED_SIZE*2 ) {
            len = PACKED_SIZE*2;
        }

        if ( !oldformat ) buf[7] = 0xfe;
        char tmp[20];
        snprintf(tmp,sizeof(tmp),"%0*llu",len,number_.data.value);
        unsigned char h, l, c;
        unsigned pos = 0;
        for ( unsigned idx = 0; idx < len; ) {
            h = tmp[idx++]-0x30;
            l = tmp[idx++];
            if ( l == '\0') l = 0xf;
            else l -= 0x30;
            if ( oldformat ) std::swap(h,l);
            c = (h << 4) | l;
            buf[pos++] = char(c);
        }
    }


    inline std::string toString() const
    {
        unsigned len = number_.data.len;
        if ( len == 0 || getTypeOfNumber() > 0x9 ) return "";
        char ret[35];
        sprintf(ret,".%u.%u.%0*llu",getTypeOfNumber(),getNumberingPlan(),len,number_.data.value);
        return ret;
    }


    void fromString( const char* address )
    {
        static const char* where = "AbntAddr::fromString";
        if ( !address ) throw exceptions::IOException("%s: bad address null", where);
        int iplan, itype;
        char buf[20];
        int scaned = sscanf( address, ".%u.%u.%15s", &itype, &iplan, buf);
        if ( scaned != 3 ) {
            scaned = sscanf( address, "+%15[0123456789]s", buf);
            if (scaned) {
                iplan = 1;
                itype = 1;
            } else {
                scaned = sscanf(address, "%15[0123456789]s", buf);
                if (!scaned) throw exceptions::IOException("%s: bad address %s", where, address);
                iplan = 1;
                itype = 1;
            }
        }
        setValue(unsigned(strlen(buf)),itype,iplan,buf);
    }


    // hashing.
    // NOTE: it may differ for little/big endian systems
    inline uint32_t HashCode( uint32_t attempt = 0 ) const
    {
        uint32_t res = smsc::util::crc32(0, &number_.number, sizeof(number_.number) );
        for ( ; attempt > 0; --attempt ) res = smsc::util::crc32(res,&number_.number,sizeof(number_.number));
        return res;
    }


    // hashing #2, see notes to HashCode
    static uint32_t CalcHash( const AbntAddr& key )
    {
        return key.HashCode(0);
    }


    void setValue( unsigned len, uint8_t type, uint8_t plan, const char* value )
    {
        static const char* where = "AbntAddr::setValue";
        // if ( !len || !value || !*value ) {
        // throw exceptions::IOException("%s: bad address NULL", where);
        // }
        if ( len > PACKED_SIZE*2 ) {
            throw exceptions::IOException("%s: too large value, len=%u val=%s", where, len, value);
        }
        const unsigned vallen = unsigned(strlen(value));
        if ( vallen < len ) {
            throw exceptions::IOException("%s: length mismatch: len=%u val=%s", where, len, value);
        }
        number_.data.ton = type;
        number_.data.npi = plan;
        uint64_t newval = 0;
        number_.data.len = len;
        const char* p = value;
        for ( unsigned idx = len; idx > 0; --idx ) {
            if ( !isdigit(*p) ) throw exceptions::IOException("%s: not a digit (0x%x) in val=%s", where, unsigned(*p) & 0xff, value );
            newval *= 10;
            newval += unsigned((*p++) - 0x30);
        }
        number_.data.value = newval;
    }

private:
    union {
        uint64_t number;
        struct {
            // NOTE: value has highest precedence
#if	BYTE_ORDER == BIG_ENDIAN
            uint64_t value:52;    // address
            unsigned len:4;       // length
            unsigned ton:4;
            unsigned npi:4;
#endif
#if	BYTE_ORDER == LITTLE_ENDIAN
            unsigned npi:4;
            unsigned ton:4;
            unsigned len:4;       // length
            uint64_t value:52;    // address
#endif
        } data;
    } number_;
};

/*
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

  inline uint8_t getSignals(char* _value) const
  {
    if (!_value) {
      return 0;
    }
    if (value.addr_content.length)
    {
      for(int i=0; i<value.addr_content.length; i++)
        _value[i] = "0123456789"[i%2 ? value.addr_content.signals[i/2] >> 4 :value.addr_content.signals[i/2] & 0x0F];
    }
    _value[value.addr_content.length] = '\0';
    return value.addr_content.length;
  }

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
    if (value.addr_content.length > ADDRESS_VALUE_SIZE * 2) {
      throw runtime_error("AbntAddr::toString: error address length");
    }
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
      char buf[32];
      getSignals(buf);
      number = atoll(buf);
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
 */

} //pvss
} //scag2


inline scag::util::storage::Serializer& operator << ( scag::util::storage::Serializer& ser, 
                                                      const scag2::pvss::AbntAddr& addr) { 
  //uint8_t len = addr.getLength() & 0x1F;
  //uint8_t plan = (addr.getNumberingPlan() & 0x07) << 5;
  //uint8_t res = len | plan;
  //ser << res;
    ser << addr.getLength();
    ser << addr.getNumberingPlan();
    ser << addr.getTypeOfNumber();
    char buf[10];
    addr.pack(buf);
    ser.writeAsIs(8,buf);
    return ser; 
};

inline scag::util::storage::Deserializer& operator >> ( scag::util::storage::Deserializer& deser,
                                                        scag2::pvss::AbntAddr& addr) { 
  //uint8_t lenplan = 0;
  //deser >> lenplan;
  //uint8_t type = 0;
  //deser >> type;
  //uint8_t len = lenplan & 0x1F;
  //uint8_t plan = (lenplan >> 5) & 0x07;

    uint8_t len = 0;
    deser >> len;
    uint8_t plan = 0;
    deser >> plan;
    uint8_t type = 0;
    deser >> type;
    const char* buf = deser.readAsIs(8);
    char val[30];
    scag2::pvss::AbntAddr::unpack(val,buf);
    assert( len == strlen(val) );
    addr.setValue( len, plan, type, val );
    return deser;
};


#endif

