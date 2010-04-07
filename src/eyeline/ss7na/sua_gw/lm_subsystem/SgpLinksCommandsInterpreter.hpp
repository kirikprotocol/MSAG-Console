#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/sua_gw/lm_subsystem/LM_CommandsInterpreter.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_SGPLinks_AddLinkCommand.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
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
