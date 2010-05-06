#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULESETLPC_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULESETLPC_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
#include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_SetLpc : public common::lm_subsystem::LM_Command {
public:
  LM_TranslationRule_SetLpc(const std::string& lpc,
                            common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr), _lpc(lpc)
  {
    setId(composeCommandId(_lpc));
  }

  virtual std::string executeCommand() {
    _trnMgr.addOperation(this);
    return "OK";
  }

  virtual void updateConfiguration();

  virtual bool replaceable() const { return true; }

  static std::string composeCommandId(const std::string& lpc);

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _lpc;
};

}}}}}

#endif
