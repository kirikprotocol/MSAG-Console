/* ************************************************************************** *
 * ICService producer that supports service configuration by XML file.
 * ************************************************************************** */
#ifndef __INMAN_ICSERVICE_XCFPRODUCER_DEFS_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICSERVICE_XCFPRODUCER_DEFS_HPP__

#include "inman/services/ICSCfgReader.hpp"

namespace smsc  {
namespace inman {

//Generic ICService producer that supports service configuration by XML file!
class ICSProducerXcfAC : public ICSProducerAC {
public:
    ICSProducerXcfAC(ICSUId ics_id)
        : ICSProducerAC(ics_id)
    { }

    //Creates XML config parser
    virtual ICSrvCfgReaderAC * newCfgReader(Config & root_sec,
                                            const char * ics_sec,
                                            Logger * use_log = NULL) = 0;
    //Returns current XML config parser
    virtual ICSrvCfgReaderAC * getCfgReader(void) const = 0;

    // -------------------------------------
    // ICSProducerAC interface methods:
    // -------------------------------------
    //Returns true if service supports configuration by XML file
    bool isConfigurable(void) const { return true; }
};


//ICService producer that creates service initializing it by configuration
//object created by config reader from XML file!
template <
      class _ICSrvTArg/* : public ICServiceAC_T */
    , class _ReaderTArg/* : public ICSrvCfgReaderAC */
    , class _CfgTArg>
class ICSProducerXcfAC_T : public ICSProducerXcfAC {
protected:
    std::auto_ptr<_ReaderTArg> xcfReader;

public:
    ICSProducerXcfAC_T(ICSUId ics_id) : ICSProducerXcfAC(ics_id)
    { }
    virtual ~ICSProducerXcfAC_T()
    { }

    // -------------------------------------
    // ICSProducerXcfAC interface methods:
    // -------------------------------------
    //Creates XML config parser
    ICSrvCfgReaderAC * newCfgReader(Config &root_sec, const char *ics_sec,
                                    Logger * use_log = NULL)
    {
        xcfReader.reset(new _ReaderTArg(root_sec, use_log, ics_sec));
        return xcfReader.get();
    }
    //Returns current XML config parser
    ICSrvCfgReaderAC * getCfgReader(void) const
    {
        return xcfReader.get();
    }

    // -------------------------------------
    // ICSProducerAC interface methods:
    // -------------------------------------
    //Returns service dependencies
    const ICSIdsSet * Deps(void) 
    {
        if (xcfReader.get())
            xcfReader->Deps().exportDeps(icsDeps);
        return ICSProducerAC::Deps();
    }
    //Creates service basing on Config parsed by ICSrvCfgReaderAC
    //Returns NULL if configuration isn't completely parsed
    ICServiceAC * newService(ICServicesHostITF * svc_host, Logger * use_log = NULL)
    {
        if (!xcfReader.get()
            || (xcfReader->icsCfgState() < ICSrvCfgReaderAC::cfgPartial))
            return NULL;
        std::auto_ptr<_CfgTArg> xcfg(xcfReader->rlseConfig());
        return new _ICSrvTArg(xcfg, svc_host, use_log);
    }
};

} //inman
} //smsc
#endif /* __INMAN_ICSERVICE_XCFPRODUCER_DEFS_HPP__ */

