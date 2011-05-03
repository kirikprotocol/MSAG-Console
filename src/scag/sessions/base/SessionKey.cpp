#include <sys/time.h>
// #include "sms/sms.h"
#include "SessionKey.h"
#include "scag/exc/SCAGExceptions.h"
#include "core/synchronization/Mutex.hpp"


namespace {

bool zeroadrdone = false;
uint64_t zeroadr_ = uint64_t(-1LL);
smsc::core::synchronization::Mutex zeromutex;
const uint8_t maxlen = 16;
uint64_t lencut[maxlen+1];

}


namespace scag2 {
namespace sessions {

using namespace scag::exceptions;

uint64_t StoredSessionKey::zeroadr()
{
    if ( zeroadr_ == uint64_t(-1LL) ) {
        uint64_t v = setaddr(0,0,1,0);
        MutexGuard mg(zeromutex);
        if ( zeroadr_ == uint64_t(-1LL) ) { zeroadr_ = v; }
    }
    return ::zeroadr_;
}


uint64_t StoredSessionKey::len2cut(uint8_t len)
{
    if ( ! ::zeroadrdone ) fillstatic();
    if ( len > ::maxlen ) {
        throw exceptions::SCAGException("unappropriate length for session key: %u", len);
    }
    return ::lencut[len];
}


void StoredSessionKey::fillstatic()
{
    MutexGuard mg(::zeromutex);
    if ( ::zeroadrdone ) return;
    // zeroadr_ = setaddr(0,0,0,0);
    uint64_t val = 1;
    for ( uint8_t l = 0; l <= ::maxlen; ++l ) {
        ::lencut[l] = val;
        val *= 10;
    }
    zeroadrdone = true;
}


StoredSessionKey::StoredSessionKey() : msisdn_(zeroadr()) {}


std::string StoredSessionKey::toString() const
{
    char buf[30];
    // NOTE: we don't put a dot between npi and address
    snprintf( buf, sizeof(buf), ".%1.1u.%1.1u%llu",
              unsigned(ton()), unsigned(npi()), adr() );
    // check that position, it must be '1'
    if ( buf[4] != '1' ) {
        throw SCAGException("corrupted sessionkey encountered: %s",buf);
    }
    buf[4] = '.'; // replace it with a dot
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
    if ( buf[0] != '1' ) {
        throw SCAGException("corrupted address encountered: %s", buf);
    }
    // remove leading '1'
    return smsc::sms::Address( uint8_t(strlen(buf))-1, ton(), npi(), buf+1 );
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
    msisdn_ = setaddr( a.type, a.plan, a.length, addr );
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
    if (!p) {
        throw smsc::util::Exception("born time has no '/'");
    }
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
