#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMSUBSYSTEM_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_LMSUBSYSTEM_HPP__

# include <netinet/in.h>
# include <string>

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/runtime_cfg/RuntimeConfig.hpp"

# include "eyeline/ss7na/common/lm_subsystem/LMSubsystem.hpp"
# include "eyeline/ss7na/sua_gw/lm_subsystem/InputCommandProcessor.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class LMSubsystem : public common::lm_subsystem::LMSubsystem,
                    public utilx::Singleton<LMSubsystem*> {
public:
  using common::lm_subsystem::LMSubsystem::initialize;
  void initialize(utilx::runtime_cfg::RuntimeConfig& rconfig);

private:
  LMSubsystem() {}
  friend class utilx::Singleton<LMSubsystem*>;

  InputCommandProcessor _inputCmdProcessor;
};

}}}}

#endif
