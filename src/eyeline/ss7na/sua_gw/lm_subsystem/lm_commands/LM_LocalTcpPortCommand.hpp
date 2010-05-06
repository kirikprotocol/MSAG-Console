#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALTCPPORTCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALTCPPORTCOMMAND_HPP__

# include <netinet/in.h>
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_LocalTcpPortCommand : public common::lm_subsystem::LM_Command {
public:
  LM_LocalTcpPortCommand(const std::string& local_port,
                         common::lm_subsystem::CmdsTransactionMgr& trn_mgr);
  virtual std::string executeCommand();
private:
  in_port_t _localPort;
};

}}}}}

#endif
