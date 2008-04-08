#ifndef SUA_USER_COMMUNICATION_LIBSUACONNECTINDICATIONEVENT_HPP_HEADER_INCLUDED_B87B5F6D
# define SUA_USER_COMMUNICATION_LIBSUACONNECTINDICATIONEVENT_HPP_HEADER_INCLUDED_B87B5F6D

# include <sua/sua_layer/io_dispatcher/IOEvent.hpp>
# include <sua/communication/LinkId.hpp>
# include <logger/Logger.h>

namespace sua_user_communication {

class LibSuaConnectIndicationEvent : public io_dispatcher::IOEvent {
public:
  LibSuaConnectIndicationEvent(const communication::LinkId& linkId);

  virtual void handle();
private:
  const communication::LinkId _acceptedLinkId;
  smsc::logger::Logger* _logger;
};

} // namespace sua_user_communication



#endif /* SUA_USER_COMMUNICATION_LIBSUACONNECTINDICATIONEVENT_HPP_HEADER_INCLUDED_B87B5F6D */
