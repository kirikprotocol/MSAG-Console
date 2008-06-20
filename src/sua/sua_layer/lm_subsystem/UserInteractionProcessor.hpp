#ifndef __SUA_SUALAYER_LMSUBSYSTEM_USERINTERACTIONPROCESSOR_HPP__
# define __SUA_SUALAYER_LMSUBSYSTEM_USERINTERACTIONPROCESSOR_HPP__

# include <core/threads/Thread.hpp>

namespace lm_subsystem {

class UserInteractionProcessor : public smsc::core::threads::Thread {
public:
  virtual ~UserInteractionProcessor() {}
  virtual void stop() = 0;
};

}

#endif
