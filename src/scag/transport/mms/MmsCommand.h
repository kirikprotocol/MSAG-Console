#ifndef __SCAG_MMS_COMMAND_HPP__
#define __SCAG_MMS_COMMAND_HPP__

#include <string>
#include <vector>

#include "util/Exception.hpp"
#include "scag/transport/SCAGCommand.h"
#include "core/synchronization/Mutex.hpp"
#include "core/buffers/Hash.hpp"
#include "logger/Logger.h"
#include "sms/sms.h"

#include "scag/transport/mms/util.h"
#include "XMLHandlers.h"
#include "MmsMsg.h"

namespace scag {
namespace transport {
namespace mms {

class MmsCommand: public SCAGCommand {
  MmsMsg* mms_msg;
public:
  MmsCommand();
  virtual ~MmsCommand();
  TransportType getType() const;
  int getServieceId() const;
  void setServieceId();
  int64_t getOperationId() const;
  void setOperationId(int64_t op);
  uint8_t getCommandId() const;
};

}//mms
}//transport
}//scag

#endif
