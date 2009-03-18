#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_GOTMESSAGEEVENT_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_GOTMESSAGEEVENT_HPP__

# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/communication/TP.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/IOEvent.hpp>
# include <logger/Logger.h>

namespace eyeline {
namespace sua {
namespace sua_layer {
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

}}}}

#endif
