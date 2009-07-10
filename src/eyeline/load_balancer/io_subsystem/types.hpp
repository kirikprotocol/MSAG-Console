#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_TYPES_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_TYPES_HPP__

# include <sys/types.h>
# include <map>
# include <string>

# include "core/buffers/RefPtr.hpp"
# include "core/synchronization/Mutex.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class Link;
class LinkSet;

typedef smsc::core::buffers::RefPtr<Link, smsc::core::synchronization::Mutex> LinkRefPtr;
typedef smsc::core::buffers::RefPtr<LinkSet, smsc::core::synchronization::Mutex> LinkSetRefPtr;

}}}

#endif
