#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULETRAFFICMODECOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULETRAFFICMODECOMMAND_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_TrafficModeCommand : public common::lm_subsystem::LM_Command {
public:
  explicit LM_TranslationRule_TrafficModeCommand(const std::string& traffic_mode)
  : _trafficMode(traffic_mode)
  {}

  virtual std::string executeCommand();

  void setTranslationRuleName(const std::string& rule_name) {
    _ruleName = rule_name;
  }

private:
  std::string _trafficMode, _ruleName;
};

}}}}}

#endif
