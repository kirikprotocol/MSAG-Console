#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALINTERFACE_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMLOCALINTERFACE_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_LocalInterface : public common::lm_subsystem::LM_Command {
public:
  LM_LocalInterface(const std::string& local_host, const std::string& local_port,
                    common::lm_subsystem::CmdsTransactionMgr& trn_mgr);

  virtual std::string executeCommand();

  virtual bool replaceable() const { return true; }

  virtual void updateConfiguration();

  static std::string composeCommandId();

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _localHost;
  in_port_t _localPort;
};

}}}}}

#endif
