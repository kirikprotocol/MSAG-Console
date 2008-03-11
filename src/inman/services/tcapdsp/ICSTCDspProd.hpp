#pragma ident "$Id$"
/* ************************************************************************** *
 * TCAP Dispatcher service producer.
 * ************************************************************************** */
#ifndef __INMAN_ICS_TCAPDISP_PRODUCER_HPP
#define __INMAN_ICS_TCAPDISP_PRODUCER_HPP

#include "inman/services/tcapdsp/ICSTCDsp.hpp"
#include "inman/services/tcapdsp/TCDspCfgReader.hpp"

#include "inman/services/ICSXcfProducer.hpp"
using smsc::inman::ICSProducerXcfAC_T;
using smsc::inman::ICSIdent;

namespace smsc {
namespace inman {
namespace inap {

//TCAP Dispatcher service producer.
class ICSProdTCAPDispatcher : public 
    ICSProducerXcfAC_T<ICSTCDispatcher, ICSTCDispCfgReader, TCDsp_CFG> {
public:
    ICSProdTCAPDispatcher()
        : ICSProducerXcfAC_T<ICSTCDispatcher, ICSTCDispCfgReader, TCDsp_CFG>
            (ICSIdent::icsIdTCAPDisp)
    { }
    ~ICSProdTCAPDispatcher()
    { }
};

} //inap
} //inman
} //smsc
#endif /* __INMAN_ICS_TCAPDISP_PRODUCER_HPP */

