#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSADDLINK_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSADDLINK_HPP__

# include <vector>
# include <netinet/in.h>
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/utilx/runtime_cfg/CompositeParameter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_SGPLinks_AddLink : public common::lm_subsystem::LM_Command {
public:
  LM_SGPLinks_AddLink(const std::string& link_id,
                      const std::vector<std::string>& local_addr_list, in_port_t local_port,
                      const std::vector<std::string>& remote_addr_list, in_port_t remote_port,
                      common::lm_subsystem::CmdsTransactionMgr& trn_mgr);

  virtual std::string executeCommand();

  virtual void updateConfiguration();

  static std::string composeCommandId(const std::string& link_id);

private:
  void setAddresses(utilx::runtime_cfg::CompositeParameter* link_param,
                    const std::string& composite_param_name,
                    const std::string& param_name,
                    const std::vector<std::string>& values);

  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _linkId;
  std::vector<std::string> _localAddressList, _remoteAddressList;
  in_port_t _localPort, _remotePort;
};

}}}}}

#endif
