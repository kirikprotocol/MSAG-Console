#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_TYPES_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_TYPES_HPP__

# include <sys/types.h>
# include <map>
# include <string>

# include "core/buffers/RefPtr.hpp"
# include "core/synchronization/Mutex.hpp"

# include "eyeline/load_balancer/io_subsystem/Link.hpp"
# include "eyeline/load_balancer/io_subsystem/LinkSet.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

typedef smsc::core::buffers::RefPtr<Link, smsc::core::synchronization::Mutex> LinkRefPtr;
typedef smsc::core::buffers::RefPtr<LinkSet, smsc::core::synchronization::Mutex> LinkSetRefPtr;

}}}

#endif
