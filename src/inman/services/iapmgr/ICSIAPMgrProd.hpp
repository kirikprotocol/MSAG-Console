/* ************************************************************************** *
 * IAPManager (abonent policies/providers manager) service producer.
 * ************************************************************************** */
#ifndef __INMAN_ICS_IAPMGR_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_IAPMGR_PRODUCER_HPP

#include "inman/services/iapmgr/ICSIAPManager.hpp"
#include "inman/services/iapmgr/IAPMgrCfgReader.hpp"

#include "inman/services/ICSXcfProducer.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {

using smsc::inman::ICSIdent;
using smsc::inman::ICSProducerXcfAC_T;

//IAPManager service producer. Supports XML file configuration.
class ICSProdIAPManager : public 
    ICSProducerXcfAC_T<ICSIAPManager, ICSIAPMgrCfgReader, IAPManagerCFG> {
public:
    ICSProdIAPManager()
        : ICSProducerXcfAC_T<ICSIAPManager, ICSIAPMgrCfgReader, IAPManagerCFG>
            (ICSIdent::icsIdIAPManager)
    { }
    ~ICSProdIAPManager()
    { }
};

} //iapmgr
} //inman
} //smsc
#endif /* __INMAN_ICS_IAPMGR_PRODUCER_HPP */

