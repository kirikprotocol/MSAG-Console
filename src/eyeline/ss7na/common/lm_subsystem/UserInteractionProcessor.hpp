#ifndef __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_USERINTERACTIONPROCESSOR_HPP__
# define __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_USERINTERACTIONPROCESSOR_HPP__

# include "core/threads/Thread.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

class UserInteractionProcessor : public smsc::core::threads::Thread {
public:
  virtual ~UserInteractionProcessor() {}
  virtual void stop() = 0;
};

}}}}

#endif
