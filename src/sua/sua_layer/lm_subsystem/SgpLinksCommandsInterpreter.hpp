#ifndef __SUA_SUALAYER_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__

# include <sua/sua_layer/lm_subsystem/LM_CommandsInterpreter.hpp>
# include <sua/sua_layer/lm_subsystem/lm_commands/LM_SGPLinks_AddLinkCommand.hpp>
namespace lm_subsystem {

class SgpLinksCommandsInterpreter : public LM_CommandsInterpreter {
public:
  virtual std::pair<lm_commands_refptr_t, lm_commands_interpreter_refptr_t> interpretCommandLine(utilx::StringTokenizer& stringTokenizer);
  virtual std::string getPromptString() const;
private:
  void parseAddressListParameter(const std::string& addressParamaterValue,
                                 std::vector<std::string>* addressList,
                                 in_port_t* port);

  lm_commands::LM_SGPLinks_AddLinkCommand* create_addLinkCommand(utilx::StringTokenizer& stringTokenizer);
};

}

#endif
