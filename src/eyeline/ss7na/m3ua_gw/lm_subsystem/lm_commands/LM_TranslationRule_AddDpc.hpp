#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDDPC_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDDPC_HPP__

# include <list>
# include <sys/errno.h>

# include "eyeline/ss7na/common/types.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"
# include "eyeline/utilx/strtol.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_AddDpc : public common::lm_subsystem::LM_Command {
public:
  LM_TranslationRule_AddDpc(const std::string& route_id,
                            const std::string& dpc,
                            common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _routeId(route_id), _trnMgr(trn_mgr), _lpc(0)
  {
    setId(composeCommandId());
    common::point_code_t dpcVal = static_cast<common::point_code_t>(utilx::strtol(dpc.c_str(), NULL, 10));
    if ( !dpcVal && errno )
      throw  common::lm_subsystem::InvalidCommandLineException("LM_TranslationRule_AddDpc::LM_TranslationRule_AddDpc::: invalid value for dpc='%s'",
                                                               dpc.c_str());

    _dpcList.push_back(dpcVal);
  }

  virtual std::string executeCommand() {
    return "OK";
  }

  virtual void updateConfiguration();

  void setTrafficMode(const std::string& value) {
    if ( value != "override" &&
         value != "loadshare")
      throw  common::lm_subsystem::InvalidCommandLineException("LM_TranslationRule_AddDpc::setTrafficMode::: invalid mode value='%s'",
                                                               value.c_str());
    _trafficMode = value;
  }

  void addDpcValue(const std::string& value) {
    common::point_code_t dpc = static_cast<common::point_code_t>(utilx::strtol(value.c_str(), NULL, 10));
    if ( !dpc && errno )
      throw  common::lm_subsystem::InvalidCommandLineException("LM_TranslationRule_AddDpc::addDpcValue::: invalid value for dpc='%s'",
                                                               value.c_str());

    _dpcList.push_back(dpc);
  }

  void setLpcValue(const std::string& value) {
    _lpc = static_cast<common::point_code_t>(utilx::strtol(value.c_str(), NULL, 10));
    if ( !_lpc && errno )
      throw  common::lm_subsystem::InvalidCommandLineException("LM_TranslationRule_AddDpc::setLpcValue::: invalid value for lpc='%s'",
                                                               value.c_str());
  }
  static std::string composeCommandId();

private:
  std::string _routeId, _trafficMode;
  common::point_code_t _lpc;
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::list<common::point_code_t> _dpcList;
};

}}}}}

#endif
