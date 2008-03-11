#pragma ident "$Id$"
/* ************************************************************************** *
 * Abonent detector service producer.
 * ************************************************************************** */
#ifndef __INMAN_ICS_ABNTDETECTOR_PRODUCER_HPP
#define __INMAN_ICS_ABNTDETECTOR_PRODUCER_HPP

#include "inman/services/abdtcr/ICSAbntDtcr.hpp"
#include "inman/services/abdtcr/AbntDtcrCfgReader.hpp"

#include "inman/services/ICSXcfProducer.hpp"
using smsc::inman::ICSProducerXcfAC_T;
using smsc::inman::ICSIdent;

namespace smsc {
namespace inman {
namespace abdtcr {

// Abonent detector service producer. Supports XML file configuration.
class ICSProdAbntDetector : public 
    ICSProducerXcfAC_T<ICSAbntDetector, ICSAbntDetectorCfgReader, AbntDetectorXCFG> {
public:
    ICSProdAbntDetector()
        : ICSProducerXcfAC_T<ICSAbntDetector, ICSAbntDetectorCfgReader, AbntDetectorXCFG>
            (ICSIdent::icsIdAbntDetector)
    { }
    ~ICSProdAbntDetector()
    { }
};

} //abdtcr
} //inman
} //smsc
#endif /* __INMAN_ICS_ABNTDETECTOR_PRODUCER_HPP */

