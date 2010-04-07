#ifndef __EYELINE_SS7NA_M3UAGW_EXCEPTION_HPP__
# define __EYELINE_SS7NA_M3UAGW_EXCEPTION_HPP__

# include <sys/types.h>
# include "util/Exception.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/SCCPMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {

class SCCPFailureException : public smsc::util::Exception {
public:
  SCCPFailureException(const sccp::messages::SCCPMessage& response_msg,
                       common::return_cause_value_t cause)
  : _responseMsg(response_msg), _cause(cause)
  {}
  const sccp::messages::SCCPMessage& getResponseMsg() const { return _responseMsg; }

  common::return_cause_value_t getFailureCode() const { return _cause; }

private:
  const sccp::messages::SCCPMessage& _responseMsg;
  common::return_cause_value_t _cause;
};

}}}

#endif
