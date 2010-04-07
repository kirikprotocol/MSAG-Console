#ifndef __EYELINE_SS7NA_COMMON_SCCPSAP_MESSAGEHANDLERS_HPP__
# define __EYELINE_SS7NA_COMMON_SCCPSAP_MESSAGEHANDLERS_HPP__

# include "eyeline/ss7na/common/LinkId.hpp"

# include "eyeline/ss7na/libsccp/messages/BindMessage.hpp"
# include "eyeline/ss7na/libsccp/messages/UnbindMessage.hpp"
# include "eyeline/ss7na/libsccp/messages/N_UNITDATA_REQ_Message.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace sccp_sap {

class MessageHandlers {
public:
  virtual ~MessageHandlers() {}

  virtual void handle(const libsccp::BindMessage& message, const LinkId& link_id) = 0;
  virtual void handle(const libsccp::UnbindMessage& message, const LinkId& link_id) = 0;
  virtual void handle(const libsccp::N_UNITDATA_REQ_Message& message, const LinkId& link_id) = 0;
};

}}}}

#endif
