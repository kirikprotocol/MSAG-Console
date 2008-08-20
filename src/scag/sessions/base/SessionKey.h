#ifndef _SCAG_SESSIONS_SESSIONKEY_H
#define _SCAG_SESSIONS_SESSIONKEY_H

#include "util/int.h"
#include "scag/util/storage/Serializer.h"
#include "logger/Logger.h"
#include "sms/sms.h"  // for Address

namespace scag2 {
namespace sessions {

using namespace scag::util::storage;

class SessionKey
{
    // static std::string isdnToString( uint64_t isdn );
    // static uint64_t stringToIsdn( const std::string& ab );
    // static void getLogger();

public:
    SessionKey() : addr_(), str_() {
        // if ( ! log_ ) getLogger();
        // dumpkey();
    }
    // explicit SessionKey( uint64_t ab ) : msisdn_(ab), str_(isdnToString(ab)) {
    // if ( ! log_ ) getLogger();
    // dumpkey();
    // }
    explicit SessionKey( const smsc::sms::Address& ab ) : addr_(ab) {}

    explicit SessionKey( const std::string& ab ) : addr_(ab.c_str()) {}

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

    /*
    uint64_t abonent() const {
        return msisdn_;
    }
     */

    inline const std::string& toString() const {
        if ( str_.empty() ) fillString();
        return str_;
        // char buf[100];
        // snprintf( buf, sizeof(buf), "%llu", msisdn_ );
        // return std::string(buf);
    }

    uint64_t toIndex() const;

    Serializer& serialize( Serializer& s ) const;
    Deserializer& deserialize( Deserializer& s ) throw (DeserializerException);

    // from sms::Address
    const SessionKey& operator = ( const smsc::sms::Address& a );

    // const SessionKey& operator = ( const SessionKey& sk );

    /// abonent address
    const smsc::sms::Address& address() const {
        return addr_;
    }

private:
    // void dumpkey() const {
    // smsc_log_debug( log_, "key set '%s'/%llu", str_.c_str(), msisdn_ );
    // }
    void fillString() const {
        str_ = addr_.toString();
    }

private:
    // static smsc::logger::Logger* log_;

private:
    smsc::sms::Address  addr_;
    mutable std::string str_;
    // uint64_t    msisdn_;
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

private:
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
