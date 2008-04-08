#ifndef __SUALAYER_IODISPATCHER_CONNECTIONRELEASEEVENT_HPP__
# define __SUALAYER_IODISPATCHER_CONNECTIONRELEASEEVENT_HPP__ 1

# include <sua/communication/LinkId.hpp>
# include <sua/sua_layer/io_dispatcher/IOEvent.hpp>
# include <sua/sua_layer/io_dispatcher/Connection.hpp>
# include <sua/sua_layer/io_dispatcher/ConnectMgr.hpp>
# include <logger/Logger.h>

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

}

#endif
