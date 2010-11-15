/* ************************************************************************** *
 * IAProvider utilizing MAP service ATSI.
 * ************************************************************************** */
#ifndef __INMAN_ICS_IAPRVD_ATSI_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAPRVD_ATSI_PRODUCER_HPP

#include "inman/services/iapATSI/ICSIAPrvdATSI.hpp"
#include "inman/services/iapATSI/IAPrvdATSICfgReader.hpp"
#include "inman/services/ICSXcfProducer.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih {

using smsc::inman::ICSIdent;
using smsc::inman::ICSProducerXcfAC_T;

//IAPrvdATSI service producer. Supports XML file configuration.
class ICSProdIAPrvdATSI : public
    ICSProducerXcfAC_T<ICSIAPrvdATSI, ICSIAPrvdATSICfgReader, IAProviderATSI_XCFG> {
public:
    ICSProdIAPrvdATSI()
        : ICSProducerXcfAC_T<ICSIAPrvdATSI, ICSIAPrvdATSICfgReader, IAProviderATSI_XCFG>
            (ICSIdent::icsIAPrvdATSI)
    { }
    ~ICSProdIAPrvdATSI()
    { }
};

} //atih
} //iaprvd
} //inman
} //smsc
#endif /* __INMAN_ICS_IAPRVD_ATSI_PRODUCER_HPP */

