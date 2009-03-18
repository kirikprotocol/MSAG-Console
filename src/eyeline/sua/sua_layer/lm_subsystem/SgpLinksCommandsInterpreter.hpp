#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__

# include <eyeline/sua/sua_layer/lm_subsystem/LM_CommandsInterpreter.hpp>
# include <eyeline/sua/sua_layer/lm_subsystem/lm_commands/LM_SGPLinks_AddLinkCommand.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {

class SgpLinksCommandsInterpreter : public LM_CommandsInterpreter {
public:
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
private:
  void parseAddressListParameter(const std::string& addressParamaterValue,
                                 std::vector<std::string>* addressList,
                                 in_port_t* port);

  lm_commands::LM_SGPLinks_AddLinkCommand* create_addLinkCommand(utilx::StringTokenizer& stringTokenizer);
};

}}}}

#endif
