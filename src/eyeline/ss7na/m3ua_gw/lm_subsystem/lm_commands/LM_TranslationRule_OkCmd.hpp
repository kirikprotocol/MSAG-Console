#ifndef __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEOKCMD_HPP__
# define __EYELINE_SS7NA_M3UAGW_LMSUBSYSTEM_LMCOMMANDS_LMTRANSLATIONRULEOKCMD_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_TranslationRule_OkCmd : public common::lm_subsystem::LM_Command {
public:
  virtual std::string executeCommand() {
    return "OK";
  }
};

}}}}}

#endif
