#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_GOTMESSAGEEVENT_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_GOTMESSAGEEVENT_HPP__

# include "logger/Logger.h"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/common/TP.hpp"
# include "eyeline/ss7na/common/io_dispatcher/IOEvent.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

class GotMessageEvent : public IOEvent {
public:
  GotMessageEvent(TP* transportPacket, const LinkId& curLinkId);

  virtual void handle();
private:
  TP* _tp;
  LinkId _curLinkId;
  smsc::logger::Logger* _logger;
};

}}}}

#endif
