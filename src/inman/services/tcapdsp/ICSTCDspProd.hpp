/* ************************************************************************** *
 * TCAP Dispatcher service producer.
 * ************************************************************************** */
#ifndef __INMAN_ICS_TCAPDISP_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TCAPDISP_PRODUCER_HPP

#include "inman/services/tcapdsp/ICSTCDsp.hpp"
#include "inman/services/tcapdsp/TCDspCfgReader.hpp"

#include "inman/services/ICSXcfProducer.hpp"

namespace smsc {
namespace inman {
namespace inap {

using smsc::inman::ICSIdent;
using smsc::inman::ICSProducerXcfAC_T;

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

