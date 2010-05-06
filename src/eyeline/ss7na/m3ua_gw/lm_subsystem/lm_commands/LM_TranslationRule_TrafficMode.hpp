#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULETRAFFICMODE_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULETRAFFICMODE_HPP__

# include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_TrafficMode : public common::lm_subsystem::LM_Command {
public:
  LM_TranslationRule_TrafficMode(const std::string& mode,
                                 common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr)
  {
    if ( mode != "override" && mode != "loadshare" )
      throw common::lm_subsystem::InvalidCommandLineException("LM_TranslationRule_TrafficMode::: invalid argument value=[%s] for traffic-mode command", mode.c_str());
    _mode = mode;
    setId(composeCommandId(_mode));
  }

  virtual std::string executeCommand() {
    _trnMgr.addOperation(this);
    return "OK";
  }

  virtual void updateConfiguration();

  static std::string composeCommandId(const std::string& mode);

  virtual bool replaceable() const { return true; }

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _mode;
};

}}}}}

#endif
