#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSUALAYERCOMMIT_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMSUALAYERCOMMIT_HPP__

# include "eyeline/ss7na/sua_gw/lm_subsystem/lm_commands/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_SuaLayer_Commit : public LM_Command {
public:
  virtual std::string executeCommand();
};

}}}}}

#endif
