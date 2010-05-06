#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALIPCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALIPCOMMAND_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_LocalIpCommand : public common::lm_subsystem::LM_Command {
public:
  LM_LocalIpCommand(const std::string& local_host,
                    common::lm_subsystem::CmdsTransactionMgr& trn_mgr);
  virtual std::string executeCommand();
private:
  std::string _localHost;
};

}}}}}

#endif
