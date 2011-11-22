#ifndef __SMSC_MCISME_ADMIN_MCASERVERPROTOCOLHANDLER_HPP__
#define __SMSC_MCISME_ADMIN_MCASERVERPROTOCOLHANDLER_HPP__ 1
#include "messages/FlushStats.hpp"
#include "messages/GetStats.hpp"
#include "messages/GetRunStats.hpp"
#include "messages/GetSched.hpp"
#include "messages/GetSchedList.hpp"
#include "messages/GetProfile.hpp"
#include "messages/SetProfile.hpp"

namespace smsc {
namespace mcisme {
namespace admin {

class MCAServerProtocolHandler {
public:
  virtual void handle(const messages::FlushStats& msg) = 0;
  virtual void handle(const messages::GetStats& msg) = 0;
  virtual void handle(const messages::GetRunStats& msg) = 0;
  virtual void handle(const messages::GetSched& msg) = 0;
  virtual void handle(const messages::GetSchedList& msg) = 0;
  virtual void handle(const messages::GetProfile& msg) = 0;
  virtual void handle(const messages::SetProfile& msg) = 0;
};

}
}
}

#endif
