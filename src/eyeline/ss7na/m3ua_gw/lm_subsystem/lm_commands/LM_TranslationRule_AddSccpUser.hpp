#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDSCCPUSER_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDSCCPUSER_HPP__

# include <list>

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/common/lm_subsystem/Exception.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_AddSccpUser : public common::lm_subsystem::LM_Command {
public:
  LM_TranslationRule_AddSccpUser(const std::string& route_id,
                                 const std::string& user_id,
                                 common::lm_subsystem::CmdsTransactionMgr& trn_mgr)
  : _routeId(route_id), _trnMgr(trn_mgr)
  {
    setId(composeCommandId());
    _userIdList.push_back(user_id);
  }

  virtual std::string executeCommand() {
    return "OK";
  }

  virtual void updateConfiguration();

  void setTrafficMode(const std::string& value) {
    if ( value != "override" &&
         value != "loadshare")
      throw  common::lm_subsystem::InvalidCommandLineException("LM_TranslationRule_AddSccpUser::setTrafficMode::: invalid mode value='%s'",
                                                               value.c_str());
    _trafficMode = value;
  }

  void addSccpUserValue(const std::string& value) {
    _userIdList.push_back(value);
  }

  static std::string composeCommandId();

private:
  std::string _routeId, _trafficMode;
  common::lm_subsystem::CmdsTransactionMgr& _trnMgr;
  std::list<std::string> _userIdList;
};

}}}}}

#endif
