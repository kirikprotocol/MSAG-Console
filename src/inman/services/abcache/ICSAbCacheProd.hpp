/* ************************************************************************** *
 * Abonents cache service producer.
 * ************************************************************************** */
#ifndef __INMAN_ICS_ABNT_CACHE_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_ABNT_CACHE_PRODUCER_HPP

#include "inman/services/abcache/ICSAbCache.hpp"
#include "inman/services/abcache/AbCacheCfgReader.hpp"
#include "inman/services/ICSXcfProducer.hpp"

namespace smsc {
namespace inman {
namespace cache {

using smsc::inman::ICSIdent;
using smsc::inman::ICSProducerXcfAC_T;

//Abonent cache service producer
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

