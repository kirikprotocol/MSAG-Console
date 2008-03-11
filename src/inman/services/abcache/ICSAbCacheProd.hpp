#pragma ident "$Id$"
/* ************************************************************************** *
 * Abonents cache service producer.
 * ************************************************************************** */
#ifndef __INMAN_ICS_ABNT_CACHE_PRODUCER_HPP
#define __INMAN_ICS_ABNT_CACHE_PRODUCER_HPP

#include "inman/services/abcache/ICSAbCache.hpp"
#include "inman/services/abcache/AbCacheCfgReader.hpp"

#include "inman/services/ICSXcfProducer.hpp"
using smsc::inman::ICSProducerXcfAC_T;
using smsc::inman::ICSIdent;

namespace smsc {
namespace inman {
namespace cache {

//Abonent cache service configurator
class ICSProdAbCache : public 
    ICSProducerXcfAC_T<ICSAbntCache, ICSAbCacheCfgReader, AbonentCacheCFG> {
public:
    ICSProdAbCache()
        : ICSProducerXcfAC_T<ICSAbntCache, ICSAbCacheCfgReader, AbonentCacheCFG>
            (ICSIdent::icsIdAbntCache)
    { }
    ~ICSProdAbCache()
    { }
};

} //cache
} //inman
} //smsc
#endif /* __INMAN_ICS_ABNT_CACHE_PRODUCER_HPP */

