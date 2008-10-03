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






SessionKey::SessionKey( const StoredSessionKey& sk ) :
StoredSessionKey(sk)
{
    str_ = StoredSessionKey::toString();
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
