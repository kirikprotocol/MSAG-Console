#ifndef _SCAG_PVSS_PROFILE_ABNTADDR_HPP_
#define _SCAG_PVSS_PROFILE_ABNTADDR_HPP_

#include <string>
#include <stdlib.h>
// #include <vector>
// #include <sms/sms.h>
// #include <sms/sms_const.h>
#include <cstring>
#include <util/crc32.h>
#include "util/byteorder.h"
// #include "util/debug.h"
#include "scag/exc/IOException.h"
#include "scag/util/io/Serializer.h"

namespace scag2 {
namespace pvss {

class AbntAddr
{
public:
    static bool getAllowNewPacking() { return allowNewPacking_; }
    static void setAllowNewPacking( bool yes ) { allowNewPacking_ = yes; }

    static const unsigned PACKED_SIZE = 8;

private:
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
        const bool newformat = ((from[PACKED_SIZE-1] & char(0x0f)) == 0x0e);
        char* ptr = buf;
        unsigned char h, l, c;
        for ( unsigned idx = 0; idx < PACKED_SIZE; ++idx ) {
            c = static_cast<unsigned char>(*from++);
            if ( newformat ) {
                h = c >> 4;
                l = c & 0xf;
            } else {
                l = c >> 4;
                h = c & 0xf;
            }
            if ( h > 0x9 ) break;
            *ptr++ = char(h+0x30);
            if ( l > 0x9 ) break;
            *ptr++ = char(l+0x30);
        }
        *ptr = '\0';
        return unsigned(ptr - buf);
    }
    

    /// pack into buffer, buffer must be of PACKED_SIZE bytes
    void pack( char* buf ) const
    {
        memset(buf,0xff,PACKED_SIZE);

        bool oldformat = !getAllowNewPacking();
        unsigned len = number_.data.len;
        if ( len > (PACKED_SIZE-1)*2 ) {
            // we cannot distinguish in that case
            oldformat = true;
        }

        if ( len > PACKED_SIZE*2 ) { len = PACKED_SIZE*2; }

        if ( !oldformat ) buf[PACKED_SIZE-1] = 0xfe;
        unsigned char l = 0;
        if ((len & 1)) {
            l = 0xf;
        }
        char* ptr = buf+((len+1)/2);
        uint64_t val = number_.data.value;
        for ( ; len > 0; ) {
            unsigned char h = static_cast<unsigned char>(val % 10);
            val /= 10;
            --len;
            if ( !(len & 1) ) {
                // need to write
                if (oldformat) {
                    *--ptr = char((l << 4) | h);
                } else {
                    *--ptr = char((h << 4) | l);
                }
            } else {
                l = h;
            }
        }
        /*
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
         */
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
        char buf[30];
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
            // __warning2__("%s: length mismatch: len=%u val=%s", where, len, value);
            len = vallen;
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


} //pvss
} //scag2


inline scag::util::io::Serializer& operator << ( scag::util::io::Serializer& ser,
                                                 const scag2::pvss::AbntAddr& addr) { 
    const size_t wpos = ser.wpos();
    ser.setwpos(wpos+3+scag2::pvss::AbntAddr::PACKED_SIZE); // 3 + 8
    unsigned char* ptr = ser.data() + wpos;
    *ptr = addr.getLength();
    *++ptr = addr.getTypeOfNumber();
    *++ptr = addr.getNumberingPlan();
    addr.pack(reinterpret_cast<char*>(++ptr));
    return ser; 
};

inline scag::util::io::Deserializer& operator >> ( scag::util::io::Deserializer& dsr,
                                                   scag2::pvss::AbntAddr& addr) { 
    const unsigned char* ptr = dsr.curpos();
    dsr.setrpos(dsr.rpos()+3+scag2::pvss::AbntAddr::PACKED_SIZE);
    uint8_t len = *ptr;
    uint8_t type = *++ptr;
    uint8_t plan = *++ptr;
    char val[30];
    scag2::pvss::AbntAddr::unpack(val,reinterpret_cast<const char*>(++ptr));
    if ( len > strlen(val) ) {
        // throw scag::exceptions::IOException("wrong abntaddr: len=%u, val=%s", unsigned(len), val);
        // __warning2__("wrong abntaddr: len=%u, val=%s", unsigned(len), val);
        len = static_cast<unsigned char>(strlen(val));
    }
    addr.setValue( len, type, plan, val );
    return dsr;
};


#endif

