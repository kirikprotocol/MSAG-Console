#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDAPPLICATIONCOMMAND_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEADDAPPLICATIONCOMMAND_HPP__

# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_AddApplicationCommand : public LM_Command {
public:
  explicit LM_TranslationRule_AddApplicationCommand(const std::string& appIdValue);
  virtual std::string executeCommand();
  void setTranslationRuleName(const std::string& ruleName);
private:
  std::string _appId, _ruleName;
};

}}}}}

#endif
