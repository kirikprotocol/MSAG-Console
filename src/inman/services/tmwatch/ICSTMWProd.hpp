/* ************************************************************************* *
 * TimeWatchers service producer.
 * ************************************************************************* */
#ifndef __INMAN_ICS_TMWATCHER_PRODUCER_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICS_TMWATCHER_PRODUCER_HPP

#include "inman/services/tmwatch/ICSTMWatcher.hpp"

namespace smsc    {
namespace inman   {

//TimeWatchers service producer. No XML configuration supported!
class ICSProdTMWatcher : public ICSProducerAC {
public:
    ICSProdTMWatcher()
        : ICSProducerAC(ICSIdent::icsIdTimeWatcher)
    { }
    ~ICSProdTMWatcher()
    { }

    // -----------------------------------
    // -- ICSProducerAC interafce methods
    // -----------------------------------
    //Returns true if service supports configuration by XML file
    bool isConfigurable(void) const { return false; }
    //Creates service basing on Config parsed by ICSrvCfgReaderAC
    ICServiceAC * 
        newService(ICServicesHostITF * svc_host, Logger * use_log = NULL)
    { return new ICSTMWatcher(svc_host, use_log); }
};

} //inman
} //smsc
#endif /* __INMAN_ICS_TMWATCHER_PRODUCER_HPP */

