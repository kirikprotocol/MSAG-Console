#pragma ident "$Id$"
/* ************************************************************************* *
 * Signalling Task Sheduler service producer.
 * ************************************************************************* */
#ifndef __INMAN_ICS_SCHEDULER_PRODUCER_HPP
#define __INMAN_ICS_SCHEDULER_PRODUCER_HPP

#include "inman/services/scheduler/ICSScheduler.hpp"
using smsc::inman::ICSIdent;
using smsc::inman::ICSProducerAC;
using smsc::inman::ICServicesHostITF;

namespace smsc    {
namespace inman   {

//Signalling Task Sheduler service producer. No XML configuration supported!
class ICSProdScheduler : public ICSProducerAC {
public:
    ICSProdScheduler()
        : ICSProducerAC(ICSIdent::icsIdScheduler)
    { }
    ~ICSProdScheduler()
    { }

    // -----------------------------------
    // -- ICSProducerAC interafce methods
    // -----------------------------------
    //Returns true if service supports configuration by XML file
    inline bool isConfigurable(void) const { return false; }
    //Creates service basing on Config parsed by ICSrvCfgReaderAC
    inline ICServiceAC * 
        newService(ICServicesHostITF * svc_host, Logger * use_log = NULL)
    { return new ICSTaskScheduler(svc_host, use_log); }
};

} //inman
} //smsc
#endif /* __INMAN_ICS_SCHEDULER_PRODUCER_HPP */

