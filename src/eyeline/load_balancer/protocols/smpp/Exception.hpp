#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_INVALIDMESSAGEPARAMEXCEPTION_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_INVALIDMESSAGEPARAMEXCEPTION_HPP__

# include "util/Exception.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

class InvalidMessageParamException : public smsc::util::Exception {
public:
  InvalidMessageParamException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}}}

#endif
