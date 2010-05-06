#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSCOMMIT_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMCOMMANDS_LMAPPLICATIONSCOMMIT_HPP__

# include "eyeline/ss7na/common/lm_subsystem/LM_Command.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {
namespace lm_commands {

class LM_Applications_Commit : public common::lm_subsystem::LM_Command {
public:
  virtual std::string executeCommand();
};

}}}}}

#endif
