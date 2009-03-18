#ifndef __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_USERINTERACTIONPROCESSOR_HPP__
# define __EYELINE_SUA_SUALAYER_LMSUBSYSTEM_USERINTERACTIONPROCESSOR_HPP__

# include <core/threads/Thread.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace lm_subsystem {

class UserInteractionProcessor : public smsc::core::threads::Thread {
public:
  virtual ~UserInteractionProcessor() {}
  virtual void stop() = 0;
};

}}}}

#endif
