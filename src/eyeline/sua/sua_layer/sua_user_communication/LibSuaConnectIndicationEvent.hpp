#ifndef __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_LIBSUACONNECTINDICATIONEVENT_HPP__
# define __EYELINE_SUA_SUALAYER_SUAUSERCOMMUNICATION_LIBSUACONNECTINDICATIONEVENT_HPP__

# include <logger/Logger.h>
# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/IOEvent.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace sua_user_communication {

class LibSuaConnectIndicationEvent : public io_dispatcher::IOEvent {
public:
  LibSuaConnectIndicationEvent(const communication::LinkId& linkId);

  virtual void handle();
private:
  const communication::LinkId _acceptedLinkId;
  smsc::logger::Logger* _logger;
};

}}}}

#endif
