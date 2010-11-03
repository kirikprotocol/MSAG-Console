/* ************************************************************************** *
 * INMan Configurable Services: configuration parsing classes
 * ************************************************************************** */
#ifndef __INMAN_ICSERVICE_CFG_READER_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_ICSERVICE_CFG_READER_HPP__

#include "util/config/XCFManager.hpp"
#include "util/config/XCFView.hpp"
#include "inman/services/ICSrvDefs.hpp"

namespace smsc  {
namespace inman {

using smsc::util::config::XCFManager;
using smsc::util::config::Config;
using smsc::util::config::XConfigView;
using smsc::util::config::CStrSet;
using smsc::util::config::ConfigException;

struct ICSCfgArgs {
  std::string _nmSec; //name of service configuration section
  CStrSet     _args;  //set of arguments for configuration parsing

  ICSCfgArgs()
  { }
};

typedef std::map<ICSUId, ICSCfgArgs> ICSArgsMap;

//Inman Configurable Services Configuration Dependencies
class ICSCfgDeps {
protected:
    ICSArgsMap  icsArgs;

    ICSArgsMap::iterator getNode(ICSUId uid)
    {
        ICSArgsMap::iterator it = icsArgs.find(uid);
        if (it == icsArgs.end()) {
            std::pair<ICSArgsMap::iterator, bool> res =
                icsArgs.insert(ICSArgsMap::value_type(uid, ICSCfgArgs()));
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
    bool empty(void) const { return icsArgs.empty(); }
    //
    const ICSArgsMap & Map(void) const { return icsArgs; }
    //
    const ICSArgsMap::value_type * LookUp(ICSUId uid) const
    {
        ICSArgsMap::const_iterator it = icsArgs.find(uid);
        return (it == icsArgs.end()) ? NULL : it.operator->();
    }

    void insert(ICSUId uid) { getNode(uid); }
    //
    void insert(const ICSIdsSet & ics_ids)
    {
        for (ICSIdsSet::const_iterator it = ics_ids.begin(); it != ics_ids.end(); ++it)
            getNode(*it);
    }
    void insert(ICSUId uid, const std::string & ics_arg)
    {
        getNode(uid)->second._args.insert(ics_arg);
    }
    //
    void insert(ICSUId uid, CStrSet & ics_arg)
    {
        getNode(uid)->second._args.insert(ics_arg.begin(), ics_arg.end());
    }
    void insert(ICSUId uid, const CStrSet & ics_arg)
    {
        CStrSet & args = getNode(uid)->second._args;
        for (CStrSet::const_iterator it = ics_arg.begin(); it != ics_arg.end(); ++it) {
            args.insert(*it);
        }
    }
    //NOTE: zero value of 'nm_sec' forves the section name reset
    void setSection(ICSUId uid, const char * nm_sec)
    {
      if (nm_sec)
        getNode(uid)->second._nmSec = nm_sec;
      else
        getNode(uid)->second._nmSec.clear();
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

    CfgState icsCfgState(void) const { return cfgState; }
    const char * nmCfgSection(void) const { return icsSec.c_str(); }
    //returns true if there are settings to read
    bool hasToRead(void) const
    {
        return (!cfgState || (!icsArg.empty()
                              && (cfgState == ICSrvCfgReaderAC::cfgPartial)));
    }
    //should be used only after readConfig() call
    const ICSCfgDeps & Deps(void) const { return icsDeps; }

    //Adds arguments which customize config parsing
    //NOTE: argument '*' is a reserved one.
    void addArgument(const std::string & use_arg)
    {
        if (icsArg.empty() || icsArg.begin()->compare("*")) {
            if (!use_arg.compare("*"))
                icsArg.clear();
            icsArg.insert(use_arg);
        }
    }
    //Adds arguments which customize config parsing
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

    void addArguments(const CStrSet & use_args)
    {
        if (use_args.empty())
            return;
        if (!icsArg.empty() && !icsArg.begin()->compare("*"))
            return;
        if (use_args.find("*") == use_args.end()) {
            icsArg.insert(use_args.begin(), use_args.end());
            for (CStrSet::const_iterator it = use_args.begin();
                                          it != use_args.end(); ++it ) {
              icsArg.insert(*it);
            }
        }
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


class ICSMultiSectionCfgReaderAC : public ICSrvCfgReaderAC {
protected:
    struct CfgParsingResult {
      ICSrvCfgReaderAC::CfgState  cfgState;
      void *                      opaqueRes; //some public result of section parsing

      CfgParsingResult(ICSrvCfgReaderAC::CfgState parsing_state = cfgNone)
        : cfgState(parsing_state), opaqueRes(0)
      { }
      CfgParsingResult(const CfgParsingResult * use_res)
      {
        if (use_res) {
          cfgState = use_res->cfgState;
          opaqueRes = use_res->opaqueRes;
        } else {
          cfgState = ICSrvCfgReaderAC::cfgNone;
          opaqueRes = 0;
        }
      }
    };
    typedef std::map<std::string, CfgParsingResult> SectionRegistry;

    SectionRegistry     secReg;
    std::auto_ptr<XConfigView> _cfgXCV;

    const CfgParsingResult * sectionState(const std::string & nm_sec) const
    {
      SectionRegistry::const_iterator it = secReg.find(nm_sec);
      return (it == secReg.end()) ? NULL : &(it->second);
    }
    ICSrvCfgReaderAC::CfgState registerSection(const std::string & nm_sec,
                                               const CfgParsingResult & parsing_res)
    {
      secReg.insert(SectionRegistry::value_type(nm_sec, parsing_res));
      return parsing_res.cfgState;
    }

    //Parses section settings. if parsing may yeld various results
    //depending on arguments, section state should be set to
    //ICSrvCfgReaderAC::cfgPartial instead of ICSrvCfgReaderAC::cfgComplete.
    //NOTE: function MUST register parsing result in SectionRegistry upon return!
    virtual ICSrvCfgReaderAC::CfgState
        parseSection(XConfigView * cfg_sec, const std::string & nm_sec, void * opaque_arg = NULL)
            throw(ConfigException) = 0;

    // -- ------------------------------------
    // -- ICSrvCfgReaderAC interface methods
    // -- ------------------------------------
    //Returns true if service depends on other ones
    //Clears arguments upon return
    ICSrvCfgReaderAC::CfgState parseConfig(void * opaque_arg = NULL) throw(ConfigException);

public:
    ICSMultiSectionCfgReaderAC(Config & root_sec, Logger * use_log, const char * ics_sec)
        : ICSrvCfgReaderAC(root_sec, use_log, ics_sec)
    { }
    virtual ~ICSMultiSectionCfgReaderAC()
    { }
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
class ICSMultiSectionCfgReaderAC_T : public ICSMultiSectionCfgReaderAC {
protected:
  std::auto_ptr<_CfgTArg> icsCfg; //structure containing parsed configuration

public:
    ICSMultiSectionCfgReaderAC_T(Config & root_sec, Logger * use_log, const char * ics_sec)
        : ICSMultiSectionCfgReaderAC(root_sec, use_log, ics_sec), icsCfg(new _CfgTArg())
    { }
    virtual ~ICSMultiSectionCfgReaderAC_T()
    { }

    virtual _CfgTArg * rlseConfig(void) { return icsCfg.release(); }
};


//Parses ICService loadUp parameter and loads required service
//loadUp parameter has format: "CfgSectionName" = "LoadUpValue"
//LoadUpValue has one of two forms:
//  "[uid:] icsUid_name" or "dll: so_lib_name"
extern ICSUId 
  processICSLoadUp(const char * nm_sec, const char * value, Logger * use_log)
      /* throw(ConfigException)*/;

} //inman
} //smsc
#endif /* __INMAN_ICSERVICE_CFG_READER_HPP__ */

