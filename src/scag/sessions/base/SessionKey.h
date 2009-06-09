#ifndef _SCAG_SESSIONS_SESSIONKEY_H
#define _SCAG_SESSIONS_SESSIONKEY_H

#include "util/int.h"
#include "scag/util/io/Serializer.h"
#include "logger/Logger.h"
#include "sms/sms.h"  // for Address
#include "core/synchronization/Mutex.hpp"

namespace scag2 {
namespace sessions {

using namespace scag::util::io;

class StoredSessionKey
{
public:
    static uint32_t CalcHash( const StoredSessionKey& key )
    {
        register const uint64_t abonent = key.toIndex();
        const uint32_t high = uint32_t(abonent >> 32);
        const uint32_t low = uint32_t(abonent);
        return low ^ (high * 33) ^ (((high >> 24) & 0xffff)*37);
    }

protected:
    static inline uint64_t setaddr( uint8_t ton, uint8_t npi, uint64_t addr ) {
        // NOTE: it is made so that for ton=1,npi=1 the msisdn_ == addr.
        return addr + ((ton^1)&0xf)*toncoef + ((npi^1)&0xf)*npicoef;
    }

private:
    static const uint64_t toncoef = 0x1000000000000000ULL;
    static const uint64_t npicoef = 0x0100000000000000ULL;
    static const uint64_t adrmask = 0x00ffffffffffffffULL;
    static uint64_t zeroadr();

public:
    StoredSessionKey();

    // explicit StoredSessionKey( const smsc::sms::Address& ab );
    // explicit StoredSessionKey( const std::string& ab );

    bool operator == ( const StoredSessionKey& k ) const
    {
        return (msisdn_ == k.msisdn_);
    }

    bool operator < ( const StoredSessionKey& k ) const
    {
        return msisdn_ < k.msisdn_;
    }

    inline uint64_t toIndex() const {
        return msisdn_;
    }

    inline void setMSISDN( uint64_t m ) {
        msisdn_ = m;
    }

    // from sms::Address
    // const StoredSessionKey& operator = ( const smsc::sms::Address& a );

    // abonent address
    // const smsc::sms::Address& address() const {
    // return addr_;
    // }

    std::string toString() const;
    // return addr_.toString();
    // }

protected:
    explicit StoredSessionKey( uint64_t a ) : msisdn_(a) {}

    inline uint8_t ton() const {
        return uint8_t(((msisdn_/toncoef)^1)&0xf);
    }
    inline uint8_t npi() const {
        return uint8_t(((msisdn_/npicoef)^1)&0xf);
    }
    inline uint64_t adr() const {
        return msisdn_ & adrmask;
    }

protected:
    uint64_t              msisdn_;   // abonent number, including ton+npi prefix
    // smsc::sms::Address addr_;
};


inline Serializer& operator << ( Serializer& s, const StoredSessionKey& sk )
{
    return s << sk.toIndex();
}


inline Deserializer& operator >> ( Deserializer& s, StoredSessionKey& sk )
{
    uint64_t m;
    s >> m; sk.setMSISDN(m);
    return s;
}


class SessionKey : public StoredSessionKey
{
    // static std::string isdnToString( uint64_t isdn );
    // static uint64_t stringToIsdn( const std::string& ab );
    // static void getLogger();

public:
    SessionKey() {}
    SessionKey( const StoredSessionKey& sk );
    explicit SessionKey( const smsc::sms::Address& ab ); // : StoredSessionKey(ab) {}
    explicit SessionKey( const std::string& ab ); // : StoredSessionKey(ab) {}

    /*
    bool operator == ( const SessionKey& k ) const
    {
        return (addr_ == k.addr_);
    }

    bool operator < ( const SessionKey& k ) const
    {
        return addr_ < k.addr_;
    }

    static uint32_t CalcHash( const SessionKey& key )
    {
        const uint64_t abonent = key.toIndex();
        const uint32_t high = uint32_t(abonent >> 32);
        const uint32_t low = uint32_t(abonent);
        return low ^ (high * 33) ^ (((high >> 24) & 0xffff)*37);
    }

     */

    inline const std::string& toString() const {
        return str_;
    }
    smsc::sms::Address address() const;

    Serializer& serialize( Serializer& s ) const;
    Deserializer& deserialize( Deserializer& s ) throw (DeserializerException);

    // from sms::Address
    const SessionKey& operator = ( const smsc::sms::Address& a );

private:
    // void fillString() const {
    // str_ = addr_.toString();
    // }

private:
    /*
    struct sharedstr {
        smsc::core::synchronization::Mutex mtx;
        int   count;
        char* str;
    };

    // cache
    mutable RefPtr<std::string,smsc::core::synchronization::Mutex> str_;
     */
    std::string str_;
};


class SessionPrimaryKey
{
public:
    SessionPrimaryKey( const SessionKey& a );
    const std::string& toString() const {
        return key_;
    }
    const timeval& bornTime() const {
        return borntime_;
    }
    void setBornTime( const timeval& tv );

private:
    void addtokey();

    SessionPrimaryKey();
    // SessionPrimaryKey( const SessionPrimaryKey& );
    // SessionPrimaryKey& operator = ( const SessionPrimaryKey& );
private:
    std::string key_;
    timeval     borntime_;
};


} // namespace sessions
} // namespace scag

#endif /* !_SCAG_SESSIONS_SESSIONKEY_H */
