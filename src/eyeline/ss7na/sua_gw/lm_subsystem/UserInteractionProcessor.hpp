#ifndef __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_USERINTERACTIONPROCESSOR_HPP__
# define __EYELINE_SS7NA_SUAGW_LMSUBSYSTEM_USERINTERACTIONPROCESSOR_HPP__

# include "core/threads/Thread.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace lm_subsystem {

class UserInteractionProcessor : public smsc::core::threads::Thread {
public:
  virtual ~UserInteractionProcessor() {}
  virtual void stop() = 0;
};

}}}}

#endif