#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_EXCEPTIONS_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_EXCEPTIONS_HPP__

# include <util/Exception.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

class ProtocolException : public smsc::util::Exception {
public:
  ProtocolException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}}}

#endif
