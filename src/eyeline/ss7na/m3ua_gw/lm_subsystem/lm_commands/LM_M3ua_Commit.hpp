#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMM3UACOMMIT_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMM3UACOMMIT_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_M3ua_Commit : public common::lm_subsystem::LM_Command {
public:
  explicit LM_M3ua_Commit(common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr)
  {}

  virtual std::string executeCommand();
private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
};

}}}}}

#endif
