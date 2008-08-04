#ifndef _SCAG_SESSIONS_SESSIONKEY_H
#define _SCAG_SESSIONS_SESSIONKEY_H

#include "util/int.h"
#include "scag/util/storage/Serializer.h"

namespace smsc {
    namespace sms {
        struct Address;
    }
}


namespace scag2 {
namespace sessions {

using namespace scag::util::storage;

class SessionKey
{
public:
    SessionKey() : msisdn_(uint64_t(-1)) {}
    explicit SessionKey( uint64_t ab ) : msisdn_(ab) {}

    bool operator == ( const SessionKey& k ) const
    {
        return (msisdn_ == k.msisdn_);
    }

    bool operator < ( const SessionKey& k ) const
    {
        return msisdn_ < k.msisdn_;
    }

    static uint32_t CalcHash( const SessionKey& key )
    {
        const uint32_t high = uint32_t(key.abonent() >> 32);
        const uint32_t low = uint32_t(key.abonent());
        return low ^ (high * 33) ^ (((high >> 24) & 0xffff)*37);
    }

    uint64_t abonent() const {
        return msisdn_;
    }

    std::string toString() const {
        char buf[100];
        snprintf( buf, sizeof(buf), "%llu", msisdn_ );
        return std::string(buf);
    }

    uint64_t toIndex() const {
        return msisdn_;
    }

    Serializer& serialize( Serializer& s ) const;
    Deserializer& deserialize( Deserializer& s ) throw (DeserializerException);

    // from sms::Address
    const SessionKey& operator = ( const smsc::sms::Address& a );

private:
    uint64_t msisdn_;
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
