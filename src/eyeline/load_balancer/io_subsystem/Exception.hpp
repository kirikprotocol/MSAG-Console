#ifndef __EYELINE_LOADBALANCER_IOSUBSYSTEM_EXCEPTION_HPP__
# define __EYELINE_LOADBALANCER_IOSUBSYSTEM_EXCEPTION_HPP__

# include "util/Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace io_subsystem {

class DieSessionException : public smsc::util::Exception {
public:
  DieSessionException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}}

#endif
