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
#include "logger/Logger.h"
#include "XMLHandlers.h"
#include "MmsMsg.h"
//#include "MmsParser.h"

//#include "MmsParserNew.hpp"

namespace scag {
namespace transport {
namespace mms {

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

class MmsCommand: public SCAGCommand {
public:
  MmsCommand();
  MmsCommand(const char* http_buf, size_t buf_size);
  virtual ~MmsCommand();
  TransportType getType() const { return scag::transport::MMS; };
  int getServiceId() const { return 0; };
  void setServiceId(int service_id) {};
  int64_t getOperationId() const { return 0; };
  void setOperationId(int64_t op) {};
  uint8_t getCommandId() const;
/*********************************************/  
  const MmsMsg* getMmsMsg() const;
  //bool createMM7Command(string http_buf, size_t buf_size); 
  bool createMM7Command(const char* http_buf, size_t buf_size); 
  bool createResponse(const MmsMsg* _mms_msg, std::string status_code, std::string status_text);
  bool createRSError(std::string transaction_id, std::string status_code, std::string status_text);
  bool createVASPError(std::string transaction_id, std::string status_code, std::string status_text);

  std::string serialize() const;
  std::string getInfoElement(const char* element_name) const;
  void setInfoElement(const char* name, std::string value);
  std::string getTransactionId() const;
  void setTransactionId(string _id);
  //void addInfoElement(const char* name, string value);
private:
  MmsMsg* mms_msg;
  Logger* logger;
  std::string transaction_id;
};

}//mms
}//transport
}//scag

#endif

