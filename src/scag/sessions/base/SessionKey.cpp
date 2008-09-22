#include <cassert>
#include <sys/time.h>
// #include "sms/sms.h"
#include "SessionKey.h"
#include "scag/exc/SCAGExceptions.h"
#include "core/synchronization/Mutex.hpp"


namespace {
bool zeroadrdone = false;
uint64_t zeroadr_;
smsc::core::synchronization::Mutex zeromutex;
}


namespace scag2 {
namespace sessions {

using namespace scag::exceptions;

/*
smsc::logger::Logger* SessionKey::log_ = 0;

std::string SessionKey::isdnToString( uint64_t isdn )
{
    char buf[50];
    snprintf( buf, sizeof(buf), ".0.1.%llu", isdn );
    return std::string(buf);
}

uint64_t SessionKey::stringToIsdn( const std::string& ab )
{
    const smsc::sms::Address a( ab.c_str() );
    if ( a.plan != 1 )
        throw SCAGException( "SessionKey: wrong address %s, bad plan", ab.c_str() );

    std::string val(a.value);
    if ( a.type == 1 ) {
        // international
    } else if ( a.type == 0 ) {
        // unknown
        // FIXME: should we convert to international?
    } else {
        throw SCAGException( "SessionKey: wrong address %s, bad type", ab.c_str() );
    }
    uint64_t isdn;
    for ( const char* p = val.c_str(); *p != '\0'; ++p ) {
        const char c = *p;
        if ( c >= '0' && c <= '9' ) {
            isdn = isdn*10 + (c-'0');
        } else {
            throw SCAGException( "SessionKey: wrong address %s, bad value", ab.c_str() );
        }
    }
    return isdn;
}


void SessionKey::getLogger()
{
    MutexGuard mg(logmtx);
    if ( ! log_ ) log_ = smsc::logger::Logger::getInstance("sess.key");
}
 */


/*
uint64_t StoredSessionKey::toIndex() const
{
    uint64_t res = 0;
    const char* err = 0;
    do {
        if ( addr_.type != 1 || addr_.plan != 1 || addr_.length < 1 ) {
            err = "wrong type/plan/length";
            break;
        }
        for ( unsigned i = 0; i < addr_.length; ++i ) {
            const char c = addr_.value[i];
            if ( c < '0' || c > '9' ) {
                err = "not-a-digit";
                break;
            }
            res = res*10 + (c-'0');
        }
    } while ( false );

    if ( err ) {
        smsc_log_error( smsc::logger::Logger::getInstance("sess.key"), "wrong address (%s): %s", err, toString().c_str() );
        throw SCAGException( "wrong address (%s): %s", err, toString().c_str() );
    }
    return res;
}
 */


uint64_t StoredSessionKey::zeroadr()
{
    if ( ! ::zeroadrdone ) {
        MutexGuard mg(::zeromutex);
        ::zeroadrdone = true;
        ::zeroadr_ = setaddr(0,0,0);
    }
    return ::zeroadr_;
}


StoredSessionKey::StoredSessionKey() : msisdn_(zeroadr()) {}


std::string StoredSessionKey::toString() const
{
    char buf[128];
    snprintf( buf, sizeof(buf), ".%1u.%1u.%llu", unsigned(ton()), unsigned(npi()), adr() );
    return std::string(buf);
}






SessionKey::SessionKey( const smsc::sms::Address& a )
{
    *this = a;
}


SessionKey::SessionKey( const std::string& a ) 
{
    smsc::sms::Address ad(a.c_str());
    *this = ad;
}


smsc::sms::Address SessionKey::address() const
{
    char buf[30];
    snprintf( buf, sizeof(buf), "%llu", adr() );
    return smsc::sms::Address( ton(), npi(), uint8_t(strlen(buf)), buf );
}


Serializer& SessionKey::serialize( Serializer& s ) const
{
    return s << msisdn_;
}


Deserializer& SessionKey::deserialize( Deserializer& s ) throw (DeserializerException)
{
    s >> msisdn_;
    str_ = StoredSessionKey::toString();
    return s;
}


const SessionKey& SessionKey::operator = ( const smsc::sms::Address& a )
{
    const uint64_t addr = strtoull( a.value, NULL, 10 );
    msisdn_ = setaddr( a.type, a.plan, addr );
    str_ = StoredSessionKey::toString();
    return *this;
}


SessionPrimaryKey::SessionPrimaryKey( const SessionKey& k )
{
    ::gettimeofday(&borntime_,0);
    key_ = k.toString();
    addtokey();
}


void SessionPrimaryKey::setBornTime( const timeval& tv )
{
    borntime_ = tv;
    const char* p = strrchr( key_.c_str(), '/' );
    assert( p );
    key_.erase( p - key_.c_str() );
    addtokey();
}


void SessionPrimaryKey::addtokey()
{
    char buf[128];
    sprintf( buf,"/%ld%d", borntime_.tv_sec, int(borntime_.tv_usec/1000) );
    key_.append( buf );
}

} // namespace sessions
} // namespace scag
