#ifndef __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_TYPES_HPP__
# define __EYELINE_SS7NA_COMMON_LMSUBSYSTEM_TYPES_HPP__

# include "core/synchronization/Mutex.hpp"
# include "core/buffers/RefPtr.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace lm_subsystem {

class LM_CommandsInterpreter;
typedef smsc::core::buffers::RefPtr<LM_CommandsInterpreter, smsc::core::synchronization::Mutex> lm_commands_interpreter_refptr_t;

class LM_Command;
typedef smsc::core::buffers::RefPtr<LM_Command, smsc::core::synchronization::Mutex> lm_commands_refptr_t;
//typedef LM_Command* lm_commands_refptr_t;

}}}}

#endif /* TYPES_HPP_ */
