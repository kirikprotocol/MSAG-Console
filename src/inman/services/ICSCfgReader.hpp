#pragma ident "$Id$"
/* ************************************************************************** *
 * 
 * ************************************************************************** */
#ifndef __INMAN_ICSERVICE_CFG_READER_HPP__
#define __INMAN_ICSERVICE_CFG_READER_HPP__

#include "inman/common/XCFManager.hpp"
#include "inman/common/XCFView.hpp"
using smsc::util::config::XCFManager;
using smsc::util::config::Config;
using smsc::util::config::XConfigView;
using smsc::util::config::CStrSet;
using smsc::util::config::ConfigException;

#include "inman/services/ICSrvDefs.hpp"

namespace smsc  {
namespace inman {

typedef std::map<ICSUId, CStrSet /*ics_arg*/> ICSArgsMap;

//Inman Configurable Services Configuration Dependencies
class ICSCfgDeps {
protected:
    ICSArgsMap  icsArgs;

    ICSArgsMap::iterator getNode(ICSUId uid)
    {
        ICSArgsMap::iterator it = icsArgs.find(uid);
        if (it == icsArgs.end()) {
            std::pair<ICSArgsMap::iterator, bool> res =
                icsArgs.insert(ICSArgsMap::value_type(uid, CStrSet()));
            it = res.first;
        }
        return it;
    }

public:
    ICSCfgDeps()
    { }
    ~ICSCfgDeps()
    { }

    void exportDeps(ICSIdsSet & ics_deps) const
    {
        for (ICSArgsMap::const_iterator it = icsArgs.begin();
                                        it != icsArgs.end(); ++it)
            ics_deps.insert(it->first);
    }
    //
    inline bool empty(void) const { return icsArgs.empty(); }
    //
    inline const ICSArgsMap & Map(void) const { return icsArgs; }
    //
    const ICSArgsMap::value_type * LookUp(ICSUId uid) const
    {
        ICSArgsMap::const_iterator it = icsArgs.find(uid);
        return (it == icsArgs.end()) ? NULL : it.operator->();
    }

    inline void insert(ICSUId uid) { getNode(uid); }
    //
    void insert(const ICSIdsSet & ics_ids)
    {
        for (ICSIdsSet::const_iterator it = ics_ids.begin(); it != ics_ids.end(); ++it)
            getNode(*it);
    }
    inline void insert(ICSUId uid, const std::string & ics_arg)
    {
        getNode(uid)->second.insert(ics_arg);
    }
    //
    inline void insert(ICSUId uid, CStrSet & ics_arg)
    {
        getNode(uid)->second.insert(ics_arg.begin(), ics_arg.end());
    }
    void insert(ICSUId uid, const CStrSet & ics_arg)
    {
        CStrSet & args = getNode(uid)->second;
        for (CStrSet::const_iterator it = ics_arg.begin(); it != ics_arg.end(); ++it) {
            args.insert(*it);
        }
    }
};

class ICSrvCfgReaderAC {
public:
    enum CfgState { cfgNone = 0, cfgPartial, cfgComplete };

protected:
    Config &        rootSec;
    CfgState        cfgState;
    CStrSet         icsArg; //arguments customizing configuration reading
    std::string     icsSec; //name of config.xml section
                            //containing service configuration
    ICSCfgDeps      icsDeps;//service dependencies list
    Logger *        logger;

    //Parses XML configuration entry section, updates dependencies.
    //Returns status of config parsing, 
    virtual CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException) = 0;

public:
    ICSrvCfgReaderAC(Config & root_sec, Logger * use_log, const char * ics_sec)
        : rootSec(root_sec), cfgState(cfgNone), logger(use_log)
    {
        if (ics_sec)
            icsSec = ics_sec;
    }
    virtual ~ICSrvCfgReaderAC()
    { }

    inline CfgState icsCfgState(void) const { return cfgState; }
    inline const char * nmCfgSection(void) const { return icsSec.c_str(); }
    //returns true if there are settings to read
    inline bool hasToRead(void) const
    {
        return (!cfgState || (!icsArg.empty()
                              && (cfgState == ICSrvCfgReaderAC::cfgPartial)));
    }
    //should be used only after readConfig() call
    inline const ICSCfgDeps & Deps(void) const { return icsDeps; }
    //NOTE: argument '*' is a reserved one.
    void addArgument(const std::string & use_arg)
    {
        if (icsArg.empty() || icsArg.begin()->compare("*")) {
            if (!use_arg.compare("*"))
                icsArg.clear();
            icsArg.insert(use_arg);
        }
    }
    //
    void addArguments(CStrSet & use_args)
    {
        if (use_args.empty())
            return;
        if (!icsArg.empty() && !icsArg.begin()->compare("*"))
            return;
        if (use_args.find("*") == use_args.end())
            icsArg.insert(use_args.begin(), use_args.end());
        else
            addArgument(std::string("*"));
    }

    //Returns true if service depends on other ones
    //Clears arguments upon return
    bool readConfig(void * opaque_arg = NULL) throw(ConfigException)
    {
        if (cfgState != ICSrvCfgReaderAC::cfgComplete) {
            if (!cfgState) {
                if (!icsSec.empty() && !rootSec.findSection(nmCfgSection()))
                    throw ConfigException("section is missed: %s", nmCfgSection());
            }
            smsc_log_info(logger, "Reading settings from '%s' ..", nmCfgSection());
            cfgState = parseConfig(opaque_arg);
        } else
            smsc_log_info(logger, "Processed settings of '%s'", nmCfgSection());
        icsArg.clear();
        return !icsDeps.empty();
    }
};

template <class _CfgTArg>
class ICSrvCfgReaderAC_T : public ICSrvCfgReaderAC {
protected:
    std::auto_ptr<_CfgTArg> icsCfg; //structure containing parsed configuration

public:
    ICSrvCfgReaderAC_T(Config & root_sec, Logger * use_log, const char * ics_sec)
        : ICSrvCfgReaderAC(root_sec, use_log, ics_sec), icsCfg(new _CfgTArg())
    { }
    virtual ~ICSrvCfgReaderAC_T()
    { }

    virtual _CfgTArg * rlseConfig(void) { return icsCfg.release(); }
};


template <class _CfgTArg>
class ICSMultiSectionCfgReaderAC_T : public ICSrvCfgReaderAC_T<_CfgTArg> {
protected:
    typedef std::map<std::string, CfgState> SectionRegistry;

    SectionRegistry     secReg;
    std::auto_ptr<XConfigView> _cfgXCV;

    CfgState sectionState(const std::string & nm_sec) const
    {
        SectionRegistry::const_iterator it = secReg.find(nm_sec);
        return (it == secReg.end()) ? ICSrvCfgReaderAC::cfgNone : it->second;
    }

    //Parses section settings. if parsing may yeld various results
    // depending on arguments, section state should be set to
    //ICSrvCfgReaderAC::cfgPartial instead of ICSrvCfgReaderAC::cfgComplete.
    virtual CfgState
        parseSection(XConfigView * cfg_sec, const std::string & nm_sec, void * opaque_arg = NULL)
            throw(ConfigException) = 0;

    // -- ------------------------------------
    // -- ICSrvCfgReaderAC_T interface methods
    // -- ------------------------------------
    //Returns true if service depends on other ones
    //Clears arguments upon return
    CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException)
    {
        if (!_cfgXCV.get())
            _cfgXCV.reset(new XConfigView(rootSec, nmCfgSection()));

        std::auto_ptr<CStrSet> subs(_cfgXCV->getShortSectionNames());

        const CStrSet * nm_lst;
        if (icsArg.empty() || !icsArg.begin()->compare("*")) {
            nm_lst = subs.get();
        } else {
            nm_lst = &icsArg;
            //check for requested sections presence
            for (CStrSet::const_iterator cit = icsArg.begin(); cit != icsArg.end(); ++cit) {
                if (subs->find(*cit) == subs->end())
                    throw ConfigException("subsection is missed: %s", cit->c_str());
            }
        }
        //parse requested sections
        for (CStrSet::const_iterator cit = nm_lst->begin(); cit != nm_lst->end(); ++cit) {
            std::auto_ptr<XConfigView> subsCfg(_cfgXCV->getSubConfig(cit->c_str()));
            ICSrvCfgReaderAC::CfgState subsRes = 
                parseSection(subsCfg.get(), *cit, opaque_arg);
            secReg.insert(SectionRegistry::value_type(*cit, subsRes));
        }

        //check overall state
        CfgState nextState = ICSrvCfgReaderAC::cfgComplete;
        for (CStrSet::const_iterator cit = subs->begin(); cit != subs->end(); ++cit) {
            if (sectionState(*cit) != ICSrvCfgReaderAC::cfgComplete) {
                nextState = ICSrvCfgReaderAC::cfgPartial;
                break;
            }
        }
        return nextState;
    }

public:
    ICSMultiSectionCfgReaderAC_T(Config & root_sec, Logger * use_log, const char * ics_sec)
        : ICSrvCfgReaderAC_T<_CfgTArg>(root_sec, use_log, ics_sec)
    { }
    ~ICSMultiSectionCfgReaderAC_T()
    { }
};

} //inman
} //smsc
#endif /* __INMAN_ICSERVICE_CFG_READER_HPP__ */

