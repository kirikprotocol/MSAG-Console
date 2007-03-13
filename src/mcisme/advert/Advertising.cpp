
// #include all that you need !!!

#include "Advertising.h"
#include "AdvertisingImpl.h"

namespace scag {
namespace advert {

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

static bool  bAdvertisingInited = false;
static Mutex initAdvertisingLock;

inline unsigned GetLongevity(Advertising*) { return 10; }
typedef SingletonHolder<AdvertisingImpl> SingleAI;

uint64_t BannerRequest::lastId = 0;		// общий счетчик 
Mutex 	 BannerRequest::lastIdMutex;	// мьютекс для доступа к счетчику

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

void Advertising::Init(const std::string& host, int port, int timeout, int maxcount)
{
    if (!bAdvertisingInited)
    {
        MutexGuard guard(initAdvertisingLock);
        if (!bAdvertisingInited) {
            AdvertisingImpl& ai = SingleAI::Instance();
            ai.init(host, port, timeout, maxcount);
            bAdvertisingInited = true;
        }
    }
}


Advertising& Advertising::Instance()
{
    if (!bAdvertisingInited) 
    {
        MutexGuard guard(initAdvertisingLock);
        if (!bAdvertisingInited) 
            throw SCAGException("Advertising not inited!");
    }
    return SingleAI::Instance();

}


} // advert
} // scag

