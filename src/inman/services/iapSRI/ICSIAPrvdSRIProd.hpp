/* ************************************************************************** *
 * IAProvider utilizing MAP service CH-SRI.
 * ************************************************************************** */
#ifndef __INMAN_ICS_IAPRVD_SRI_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAPRVD_SRI_PRODUCER_HPP

#include "inman/services/iapSRI/ICSIAPrvdSRI.hpp"
#include "inman/services/iapSRI/IAPrvdSRICfgReader.hpp"
#include "inman/services/ICSXcfProducer.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

using smsc::inman::ICSIdent;
using smsc::inman::ICSProducerXcfAC_T;

//IAPrvdSRI service producer. Supports XML file configuration.
class ICSProdIAPrvdSRI : public
    ICSProducerXcfAC_T<ICSIAPrvdSRI, ICSIAPrvdSRICfgReader, IAProviderSRI_XCFG> {
public:
    ICSProdIAPrvdSRI()
        : ICSProducerXcfAC_T<ICSIAPrvdSRI, ICSIAPrvdSRICfgReader, IAProviderSRI_XCFG>
            (ICSIdent::icsIAPrvdSRI)
    { }
    ~ICSProdIAPrvdSRI()
    { }
};

} //sri
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_ICS_IAPRVD_SRI_PRODUCER_HPP */

