#ifndef IO_DISPATCHER_GOTMESSAGEEVENT_HPP_HEADER_INCLUDED_B87B1A1D
# define IO_DISPATCHER_GOTMESSAGEEVENT_HPP_HEADER_INCLUDED_B87B1A1D

# include <sua/communication/LinkId.hpp>
# include <sua/communication/TP.hpp>
# include <sua/sua_layer/io_dispatcher/IOEvent.hpp>
# include <logger/Logger.h>

namespace io_dispatcher {

class GotMessageEvent : public IOEvent {
public:
  GotMessageEvent(communication::TP* transportPacket, const communication::LinkId& curLinkId);

  virtual void handle();
private:
  communication::TP* _tp;
  communication::LinkId _curLinkId;
  smsc::logger::Logger* _logger;
};

} // namespace io_dispatcher

#endif /* IO_DISPATCHER_GOTMESSAGEEVENT_HPP_HEADER_INCLUDED_B87B1A1D */
