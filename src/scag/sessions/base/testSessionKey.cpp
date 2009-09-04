#include <iostream>
#include <algorithm>
#include "logger/Logger.h"
#include "scag/util/io/Drndm.h"
#include "SessionKey.h"

using namespace scag2::sessions;

std::ostream& printKey( const SessionKey& sk )
{
    char buf[256];
    sprintf(buf,"%016llx %08x %u %u %s %s",
            sk.toIndex(), SessionKey::CalcHash(sk),
            sk.ton(), sk.npi(),
            sk.toString().c_str(),
            sk.address().toString().c_str() );
    if ( sk.toString() != std::string(sk.address().toString().c_str()) ) {
        std::cout << "# sk= " << sk.toString() << " != ad= " << sk.address().toString().c_str() << std::endl;
    }
    return std::cout << buf << std::endl;
}


int main()
{
    smsc::logger::Logger::Init();
    std::vector< SessionKey > keys;
    keys.reserve(200);
    SessionKey sk;
    keys.push_back(sk);
    scag::util::Drndm& rnd = scag::util::Drndm::getRnd();
    for ( size_t i = 0; i < 100; ++i ) {
        const uint8_t type = uint8_t(double(rnd.getNextNumber()) / rnd.randmax() * 10);
        const uint8_t plan = uint8_t(double(rnd.getNextNumber()) / rnd.randmax() * 10);
        const uint8_t len = uint8_t(double(rnd.getNextNumber()) / rnd.randmax() * 17) + 1;
        char val[30];
        char* p = val;
        for ( p = val; (p - val) < len; ++p ) {
            *p = char(uint8_t(double(rnd.getNextNumber()) / rnd.randmax() * 10) + '0');
        }
        *p = '\0';
        {
            char buf[256];
            sprintf(buf,"# l,t,n,v=%u,%u,%u,%s",len,type,plan,val);
            std::cout << buf << std::endl;
        }
        try {
            smsc::sms::Address a(len,type,plan,val);
            sk = a;
        } catch ( std::exception& e ) {
            char buf[256];
            sprintf(buf,"# exc on key(%u,%u,%u,%s): %s",len,type,plan,val,e.what());
            std::cout << buf << std::endl;
        }
        keys.push_back(sk);
    }
    std::sort( keys.begin(), keys.end() );
    for ( std::vector< SessionKey >::const_iterator i = keys.begin();
          i != keys.end(); ++i ) {
        printKey(*i);
    }
    return 0;
}
