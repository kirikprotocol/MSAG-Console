#ifndef _SCAG_PVSS_SERVER_CONNECTION_H_
#define _SCAG_PVSS_SERVER_CONNECTION_H_

#include <vector>
#include <string>
#include "logger/Logger.h"

namespace scag2 {
namespace pvss  {

struct DbLog {
  DbLog(){};
  DbLog(smsc::logger::Logger* log, const std::string& msg):log_(log), msg_(msg) {};
  void set(smsc::logger::Logger* log, const std::string& msg) { log_ = log; msg_ = msg; }
  void flush() {
    if (log_ && !msg_.empty()) {
      smsc_log_info(log_, "%s", msg_.c_str());
    }
  }
private:
  smsc::logger::Logger* log_;
  std::string msg_;
};

class Connection {
public:
  virtual void sendResponse(const char* data, uint32_t dataSize, const std::vector<DbLog>* dbLogs) = 0;
  virtual ~Connection() {};
};

}//pvss
}//scag2

#endif
  
 
