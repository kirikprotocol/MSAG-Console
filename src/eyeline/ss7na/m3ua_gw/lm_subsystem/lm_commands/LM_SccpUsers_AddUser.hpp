#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSCCPUSERSADDUSER_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMSCCPUSERSADDUSER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_SccpUsers_AddUser : public common::lm_subsystem::LM_Command {
public:
  LM_SccpUsers_AddUser(const std::string& user_id,
                       common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _userId(user_id), _trnMgr(trn_mgr)
  {
    smsc_log_debug(_logger, "LM_SccpUsers_AddUser:: new object=0x%p", this);
    setId(composeCommandId(_userId));
  }

  virtual std::string executeCommand();

  virtual void updateConfiguration();

  static std::string composeCommandId(const std::string& user_id);

private:
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::string _userId;
};

}}}}}

#endif
