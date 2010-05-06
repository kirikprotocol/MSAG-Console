#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINK_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSGPLINKSREMOVELINK_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_SGPLinks_RemoveLink : public common::lm_subsystem::LM_Command {
public:
  LM_SGPLinks_RemoveLink(const std::string& link_id,
                         common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _trnMgr(trn_mgr), _linkId(link_id)
  {
    setId(composeCommandId(_linkId));
  }

  virtual std::string executeCommand();

  virtual void updateConfiguration();

  static std::string composeCommandId(const std::string& link_id);

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _linkId;
};

}}}}}

#endif
