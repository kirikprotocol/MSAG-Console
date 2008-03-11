#pragma ident "$Id$"
/* ************************************************************************** *
 * Configurable Services Host config file parsing.
 * ************************************************************************** */
#ifndef __INMAN_SVCHOST_CONFIG_PARSING__
#define __INMAN_SVCHOST_CONFIG_PARSING__

#include "inman/common/CSVList.hpp"
using smsc::util::CSVList;

#include "inman/services/ICSCfgReader.hpp"
#include "inman/services/ICSrvLoader.hpp"
#include "inman/services/SVCHostDflts.hpp"

namespace smsc {
namespace inman {

class ICSHostCfgReader : public ICSrvCfgReaderAC_T<SvcHostCFG> {
private:
    ICSLoadupList   loadUp;

    void printKnownUIds(void) const
    {
        ICSIdsSet ids = ICSLoadupsReg::get().knownUIds();
        if (!ids.empty())
            smsc_log_info(logger, "Known UIds: %s", ids.toString().c_str());
    }

    //param value has one of two forms:
    //  "uid: icsUid_name" or "dll: so_lib_name"
    void parseLoadUp(ICSLoadupList & ld_list, const std::string & nm_prm,
                     const char * value)
        throw(ConfigException)
    {
        static const char * alg[] = { "uid", "dll"};
        CSVList prm(value, ':');
        if (prm.size() != 2)
            throw ConfigException("  param %s: value is invalid", nm_prm.c_str());

        ICSLoadupCFG ldUp;
        const ICSLoadupCFG * dfLdUp = NULL;
        std::auto_ptr<ICSProducerAC> allc;
        //determine loadUp type 
        if (!prm[0].compare("dll")) {
            ldUp.ldType = ICSLoadupCFG::icsLdDll;
            //load dll, get service UId and producer
            smsc_log_info(logger, "Loading ICService driver '%s' ..", prm[1].c_str());
            allc.reset(ICSrvLoader::LoadICS(prm[1].c_str()));
            smsc_log_info(logger, "loaded '%s', ICSUId: %s", prm[1].c_str(),
                          ICSIdent::uid2Name(allc->icsUId()));
            ldUp.icsUId = allc->icsUId();
            ldUp.secNm = nm_prm;

            //verify service loadUp settings (UId and linkage type)
            if (!(dfLdUp = ICSLoadupsReg::get().find(ldUp.icsUId)))
                throw ConfigException("Service '%s' is not supported",
                                      ICSIdent::uid2Name(ldUp.icsUId));
            if (dfLdUp->ldType != ldUp.ldType)
                throw ConfigException("only %s linkage is supported for Service '%s'",
                                      dfLdUp->nmLDType(), ICSIdent::uid2Name(ldUp.icsUId));
            /**/
        } else if (!prm[0].compare("uid")) {
            ICSUId uid = ICSIdent::name2UId(prm[1]);
            if (uid == ICSIdent::icsIdUnknown) {
                smsc_log_fatal(logger, "  param %s: service UId is invalid %s",
                                      nm_prm.c_str(), prm[1].c_str());
                printKnownUIds();
                throw ConfigException("param %s: service UId is invalid %s",
                                      nm_prm.c_str(), prm[1].c_str());
            }
            //verify service loadUp settings (UId and linkage type)
            if (!(dfLdUp = ICSLoadupsReg::get().find(uid)))
                throw ConfigException("Service '%s' is not supported",
                                      ICSIdent::uid2Name(uid));
            ldUp = *dfLdUp;
            if (dfLdUp->ldType == ICSLoadupCFG::icsLdDll) {
                //load dll, get service UId and producer
                smsc_log_info(logger, "Loading ICService driver '%s' ..", ldUp.dllNm.c_str());
                allc.reset(ICSrvLoader::LoadICS(ldUp.dllNm.c_str()));
                smsc_log_info(logger, "loaded '%s', ICSUId: %s", ldUp.dllNm.c_str(),
                              ICSIdent::uid2Name(allc->icsUId()));
                if (ldUp.icsUId != allc->icsUId())
                    throw ConfigException("ICSUId mismatch %s{%s} vs %s",
                                          ldUp.dllNm.c_str(), ICSIdent::uid2Name(allc->icsUId()),
                                          ICSIdent::uid2Name(ldUp.icsUId));
            }
            ldUp.secNm = nm_prm;
        } else
            throw ConfigException("param %s: loadUp type is invalid %s",
                                  nm_prm.c_str(), prm[0].c_str());

        //check uniqueness
        if (ld_list.find(ldUp.icsUId))
            throw ConfigException("Multiple loadUp settings for '%s'",
                                  ICSIdent::uid2Name(ldUp.icsUId));

        if (ldUp.ldType == ICSLoadupCFG::icsLdDll) {
            //register producer for Dll
            ICSLoadupsReg::get().resetProducer(ldUp.icsUId, allc.get());
            ldUp.prod = allc.release();
        } else
            ldUp.prod = dfLdUp->prod;
        /**/
        smsc_log_info(logger, "%s", ldUp.Details().c_str());
        ld_list.push_back(ldUp);
        return;
    }

    bool addDfltLoadUps(ICSLoadupList & ld_list) throw(ConfigException)
    {
        ICSLoadupList::iterator it = ICSLoadupsReg::get().appendDefaults(ld_list);
        for (; it != ld_list.end(); ++it) {
            if (!it->prod && it->ldType == ICSLoadupCFG::icsLdDll) {
                //load Dll and get producer
                smsc_log_info(logger, "Loading ICService driver '%s' ..", it->dllNm.c_str());
                it->prod = ICSLoadupsReg::get().getProducer(it->icsUId);
                smsc_log_info(logger, "loaded '%s' driver, UId: %s", it->dllNm.c_str(),
                              ICSIdent::uid2Name(it->prod->icsUId()));
            }
            smsc_log_info(logger, "%s", it->Details().c_str());
        }
        return !ld_list.empty();
    }

    //Reads services loadup configuration
    bool readLoadUps(XConfigView & cfg_sec, ICSLoadupList & ld_list)
        throw(ConfigException)
    {
        if (!cfg_sec.findSubSection("Services")) {
            smsc_log_warn(logger, "'Services' section is empty or missed!");
            return addDfltLoadUps(ld_list);
        }
        std::auto_ptr<XConfigView>  cfgSub(cfg_sec.getSubConfig("Services"));
        std::auto_ptr<CStrSet>      srvs(cfgSub->getStrParamNames());
        if (srvs->empty()) {
            smsc_log_warn(logger, "'Services' section is empty or missed!");
            return addDfltLoadUps(ld_list);
        }
        for (CStrSet::iterator it = srvs->begin(); it != srvs->end(); ++it) {
            const char * cstr = cfgSub->getString(it->c_str());
            if (!cfg_sec.findSubSection(it->c_str()))
                throw ConfigException("%s section is missed", it->c_str());
            parseLoadUp(ld_list, *it, cstr);
        }
        return addDfltLoadUps(ld_list);
    }

protected:
    static const unsigned int _DFLT_CLIENT_CONNS = 3;

    //Parses XML configuration entry section, updates dependencies.
    //Returns status of config parsing, 
    CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException)
    {
        XConfigView cfgSec(rootSec, nmCfgSection());
        
        const char * cstr = NULL;
        try { cstr = cfgSec.getString("version");
        } catch (const ConfigException & exc) { }
        if (!cstr || !cstr[0])
            smsc_log_warn(logger, "Config version is not set");
        else
            smsc_log_info(logger, "Config version: %s", cstr);

        //Read initial services loadup configuration
        if (!readLoadUps(cfgSec, loadUp))
            throw ConfigException("No services loadUps defined!");

        //create configuration for already prepared producers from initial LoadUp list
        for (ICSLoadupList::iterator it = loadUp.begin(); it != loadUp.end(); ++it) {
            icsCfg->prodReg.insert(it->icsUId, new ICSProducerCFG(it->prod, rootSec, it->secNm, logger));
        }
        //Read configurations for prepared services, load up newly requested ones
        ICSProducerCFG * prodCfg = NULL;
        ICSProducersReg::size_type attempt = icsCfg->prodReg.size() + 1;
        while ((prodCfg = icsCfg->prodReg.nextToRead()) != 0) { //xcfReader is set
            if (!--attempt)
                throw ConfigException("'%s' service config parsing failure",
                                            ICSIdent::uid2Name(prodCfg->icsUId));
            if (prodCfg->xcfReader->readConfig()) { //throws
                //process requested dependencies
                ICSArgsMap deps = prodCfg->xcfReader->Deps().Map();
                for (ICSArgsMap::iterator it = deps.begin(); it != deps.end(); ++it) {
                    ICSUId reqUId = it->first;
                    ICSProducerCFG * reqCfg = icsCfg->prodReg.find(reqUId);
                    if (!reqCfg) { //dependent producer not configured, service not loaded
                        smsc_log_debug(logger, "'%s' requires the '%s'",
                                      ICSIdent::uid2Name(prodCfg->icsUId),
                                      ICSIdent::uid2Name(reqUId));
                        const ICSLoadupCFG * ldUp = ICSLoadupsReg::get().find(reqUId);
                        if (!ldUp)
                            throw ConfigException("Service '%s' loadup is not defined!",
                                                  ICSIdent::uid2Name(reqUId));
                        smsc_log_info(logger, "%s", ldUp->Details().c_str());
                        ICSProducerAC * allc = NULL;
                        if (!ldUp->prod && ldUp->ldType == ICSLoadupCFG::icsLdDll) {
                            //load Dll and get producer
                            smsc_log_info(logger, "Loading ICService driver '%s' ..", ldUp->dllNm.c_str());
                            allc = ICSLoadupsReg::get().getProducer(ldUp->icsUId);
                            if (allc)
                                smsc_log_info(logger, "loaded '%s', ICSUId: %s", ldUp->dllNm.c_str(),
                                          ICSIdent::uid2Name(allc->icsUId()));
                        } else
                            allc = ldUp->prod;
                        if (!allc)
                            throw ConfigException("Service '%s' producer is not defined!",
                                                  ICSIdent::uid2Name(reqUId));
                        icsCfg->prodReg.insert(ldUp->icsUId,
                                    reqCfg = new ICSProducerCFG(allc, rootSec, ldUp->secNm, logger));
                        ++attempt;
                    }
                    if (reqCfg->cfgState() != ICSrvCfgReaderAC::cfgComplete)
                        reqCfg->xcfReader->addArguments(it->second);
                }
            }
        }
        /**/
        return ICSrvCfgReaderAC::cfgComplete;
    }

public:
    ICSHostCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
        : ICSrvCfgReaderAC_T<SvcHostCFG>(root_sec, use_log, ics_sec)
    { }
    ~ICSHostCfgReader()
    { }
};


} //inman
} //smsc
#endif /* __INMAN_SVCHOST_CONFIG_PARSING__ */

