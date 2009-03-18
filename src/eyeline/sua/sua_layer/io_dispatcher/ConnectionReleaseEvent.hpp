#ifndef __EYELINE_SUA_SUALAYER_IODISPATCHER_CONNECTIONRELEASEEVENT_HPP__
# define __EYELINE_SUA_SUALAYER_IODISPATCHER_CONNECTIONRELEASEEVENT_HPP__

# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/IOEvent.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/Connection.hpp>
# include <eyeline/sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
# include <logger/Logger.h>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace io_dispatcher {

class ConnectionReleaseEvent : public io_dispatcher::IOEvent {
public:
  ConnectionReleaseEvent(const communication::LinkId& releasedLinkId);

  virtual void handle();
private:
  communication::LinkId _releasedLinkId;
  smsc::logger::Logger* _logger;
  ConnectMgr& _cMgr;
};

}}}}

#endif
