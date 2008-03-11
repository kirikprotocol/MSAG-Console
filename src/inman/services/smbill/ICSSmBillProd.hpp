#pragma ident "$Id$"
/* ************************************************************************** *
 * SMS/USSD messages billing service producer.
 * ************************************************************************** */
#ifndef __INMAN_ICS_SMBILLING_PRODUCER_HPP
#define __INMAN_ICS_SMBILLING_PRODUCER_HPP

#include "inman/services/smbill/ICSSmBilling.hpp"
#include "inman/services/smbill/SmBillCfgReader.hpp"

#include "inman/services/ICSXcfProducer.hpp"
using smsc::inman::ICSProducerXcfAC_T;
using smsc::inman::ICSIdent;

namespace smsc {
namespace inman {
namespace smbill {

//SMS/USSD messages billing service. Supports XML file configuration.
class ICSProdSmBilling : public 
    ICSProducerXcfAC_T<ICSSmBilling, ICSSmBillingCfgReader, SmBillingXCFG> {
public:
    ICSProdSmBilling()
        : ICSProducerXcfAC_T<ICSSmBilling, ICSSmBillingCfgReader, SmBillingXCFG>
            (ICSIdent::icsIdSmBilling)
    { }
    ~ICSProdSmBilling()
    { }
};

} //smbill
} //inman
} //smsc
#endif /* __INMAN_ICS_SMBILLING_PRODUCER_HPP */

