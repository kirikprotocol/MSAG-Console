/* ************************************************************************* *
 * INMan Configurable Services Host types, interfaces and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_SVC_HOST_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_SVC_HOST_DEFS_HPP

#include "inman/common/ObjRegistryT.hpp"
using smsc::util::POBJRegistry_T;

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/ICSXcfProducer.hpp"

namespace smsc {
namespace inman {

struct ICSProducerCFG {
    ICSUId          icsUId;         //
    ICSProducerAC * allc;           //service producer
    ICSrvCfgReaderAC *  xcfReader;  //XML configuration reader

    ICSProducerCFG(ICSProducerAC * use_allc, Config & root_sec,
                   const char * sec_nm, Logger * use_log)
        : icsUId(use_allc->icsUId()), allc(use_allc)
        , xcfReader(0)
    { 
        xcfReader = (!allc->isConfigurable() ? NULL :
                  ((ICSProducerXcfAC*)allc)->newCfgReader(root_sec, sec_nm, use_log));
    }
    ICSrvCfgReaderAC::CfgState cfgState(void) const
    {
         return (xcfReader ? xcfReader->icsCfgState() : ICSrvCfgReaderAC::cfgComplete);
    }
    bool cfgToRead(void) const
    {
        return (xcfReader ? xcfReader->hasToRead() : false);
    }
};

class ICServicesReg : public POBJRegistry_T<ICSUId, ICServiceAC> {
public:
    ICServicesReg() : POBJRegistry_T<ICSUId, ICServiceAC>()
    { }
    //NOTE: if service with UId == exclude_id is the only one matching state
    //condition, then it's returned.
    ICServiceAC * nextWithLesserState(ICSState tgt_state,
                                      ICSUId exclude_id = ICSIdent::icsIdUnknown) const
    {
        unsigned i = (exclude_id == ICSIdent::icsIdUnknown) ? 1 : 0;
        do {
            for (const_iterator it = begin(); it != end(); ++it) {
                if ((it->second->icsState() < tgt_state)
                    && (it->first != exclude_id))
                    return it->second;
            }
            exclude_id = ICSIdent::icsIdUnknown;
        } while ((++i) < 2);
        return NULL;
    }
};

class ICSProducersReg : public POBJRegistry_T<ICSUId, ICSProducerCFG> {
public:
    ICSProducersReg() : POBJRegistry_T<ICSUId, ICSProducerCFG>()
    { }

    ICSProducerCFG * nextToRead(void) const
    {
        for (const_iterator it = begin(); it != end(); ++it) {
            if (it->second->cfgToRead())
                return it->second;
        }
        return NULL;
    }
};

struct SvcHostCFG {
    ICSProducersReg prodReg;
    ICServicesReg   srvReg;
};

} //inman
} //smsc

#endif /* __INMAN_SVC_HOST_DEFS_HPP */

