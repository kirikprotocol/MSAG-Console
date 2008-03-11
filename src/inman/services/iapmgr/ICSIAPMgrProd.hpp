#pragma ident "$Id$"
/* ************************************************************************** *
 * IAPManager (abonent policies/providers manager) service producer.
 * ************************************************************************** */
#ifndef __INMAN_ICS_IAPMGR_PRODUCER_HPP
#define __INMAN_ICS_IAPMGR_PRODUCER_HPP

#include "inman/services/iapmgr/ICSIAPManager.hpp"
#include "inman/services/iapmgr/IAPMgrCfgReader.hpp"

#include "inman/services/ICSXcfProducer.hpp"
using smsc::inman::ICSProducerXcfAC_T;
using smsc::inman::ICSIdent;

namespace smsc {
namespace inman {
namespace iapmgr {

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

