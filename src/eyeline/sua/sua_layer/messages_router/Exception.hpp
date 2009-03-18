#ifndef __EYELINE_SUA_SUALAYER_MESSAGESROUTER_EXCEPTION_HPP__
# define __EYELINE_SUA_SUALAYER_MESSAGESROUTER_EXCEPTION_HPP__

# include <util/Exception.hpp>
# include <eyeline/sua/communication/types.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace messages_router {

class TranslationFailure : public smsc::util::Exception {
public:
  TranslationFailure(communication::return_cause_value_t retCauseVal, const char * fmt, ...)
    : Exception(), _retCauseVal(retCauseVal)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }

  communication::return_cause_value_t getFailureCode() const {
    return _retCauseVal;
  }
private:
  communication::return_cause_value_t _retCauseVal;
};

}}}}

#endif
