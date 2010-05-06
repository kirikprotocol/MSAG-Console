#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_SGPLinks_AddLinkCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class SgpLinksCommandsInterpreter : public common::lm_subsystem::LM_CommandsInterpreter {
public:
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& string_tokenizer);
  virtual std::string getPromptString() const;
private:
  void parseAddressListParameter(const std::string& addr_param_value,
                                 std::vector<std::string>* addr_list,
                                 in_port_t* port);

  lm_commands::LM_SGPLinks_AddLinkCommand* create_addLinkCommand(utilx::StringTokenizer& string_tokenizer);
};

}}}}

#endif
