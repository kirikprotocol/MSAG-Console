#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_SGPLINKSCOMMANDSINTERPRETER_HPP__

# include "eyeline/ss7na/common/lm_subsystem/CmdsTransactionMgr.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"
# include "eyeline/ss7na/common/lm_subsystem/LM_CommandsInterpreter.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {

class SgpLinksCommandsInterpreter : public common::lm_subsystem::LM_CommandsInterpreter {
public:
  virtual interpretation_result interpretCommandLine(utilx::StringTokenizer& string_tokenizer);
  virtual std::string getPromptString() const { return "eyeline m3ua(config sgp-links)# "; }

private:
  void parseAddressListParameter(const std::string& addr_param_value,
                                 std::vector<std::string>* addr_list,
                                 in_port_t* port);

  common::lm_subsystem::LM_Command* create_addLinkCommand(utilx::StringTokenizer& string_tokenizer);
  common::lm_subsystem::LM_Command* create_removeLinkCommand(utilx::StringTokenizer& string_tokenizer);

  std::string trimSpaces(const std::string& value);

  common::lm_subsystem::CmdsTransactionMgr _trnMgr;
};

}}}}

#endif
