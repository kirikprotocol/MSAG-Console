#ifndef __EYELINE_TCAP_EXCEPTION_HPP__
# define __EYELINE_TCAP_EXCEPTION_HPP__

# include <util/Exception.hpp>

namespace eyeline {
namespace tcap {

class UnknownDialogueException : public smsc::util::Exception {
public:
  UnknownDialogueException(const char * fmt, ...)
    : Exception()
  {
    SMSC_UTIL_EX_FILL(fmt);
  }
};

}}

#endif
