#ifndef __SCAG_MMS_COMMAND_HPP__
#define __SCAG_MMS_COMMAND_HPP__

#include <string>

#include "scag/transport/SCAGCommand.h"
#include "logger/Logger.h"
#include "MmsMsg.h"

namespace scag {
namespace transport {
namespace mms {

namespace status {
  extern const uint32_t SUCSESS;
  extern const uint32_t VALIDATION_ERROR;
  extern const uint32_t SERVICE_ERROR;
  extern const uint32_t SERVICE_UNAVAILABLE;
  extern const uint32_t SERVICE_DENIED;
  extern const uint32_t ENDPOINT_NOT_REGISTRED;
  extern const uint32_t ROUTE_NOT_FOUND;
}

namespace status_text {
  extern const char* SUCSESS;
  extern const char* VALIDATION_ERROR;
  extern const char* SERVICE_ERROR;
  extern const char* SERVICE_UNAVAILABLE;
  extern const char* SERVICE_DENIED;
  extern const char* ENDPOINT_NOT_REGISTRED;
  extern const char* ROUTE_NOT_FOUND;
}

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

using scag::sessions::SessionPtr;

struct TransactionContext {
  int64_t operationId;
  uint32_t serviceId;
  uint32_t providerId;
  uint32_t routeId;
  int status;    
  unsigned int port;
  std::string host;
  int ruleId;
  uint16_t usr;

  TransactionContext() {
    operationId = -1;
    serviceId = 0;
    providerId = 0;
    ruleId = 0;
    routeId = 0;
    usr = 0;
    status = 503;
    port = 0;
  }
};

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
  SessionPtr getSession();
  void setSession(SessionPtr& s);
  bool hasSession();

/*********************************************/  
  const MmsMsg* getMmsMsg() const;
  bool createMM7Command(const char* http_buf, size_t buf_size); 
  bool createResponse(const MmsMsg* _mms_msg,
                      const string& status_code, const string& status_text);
  bool createResponse(const string& status_code, const string& status_text);
  bool createResponse(const MmsMsg* _mms_msg, int status_code);
  bool createResponse(int status_code);
  bool createRSError(const string& transaction_id,
                     const string& status_code, const string& status_text);
  bool createVASPError(const string& transaction_id,
                       const string& status_code, const string& status_text);
  bool createRSError(const string& transaction_id, int status_code);
  bool createVASPError(const string& transaction_id, int status_code);

  bool serialize(string& serilized_cmd) const;
  string getInfoElement(const char* element_name) const;
  void setInfoElement(const char* name, const string& value);
  const string& getTransactionId() const;
  void setTransactionId(const string& _id);
  virtual bool isRequest() { return false; };

  SessionPtr session;
protected:
  void setStatus(int status);
  MmsMsg* mms_msg;
  Logger* logger;
  string transaction_id;
};

class MmsRequest : public MmsCommand {
public:
  MmsRequest();
  ~MmsRequest() {};
  bool isRequest() {
    return true;
  }
  string getEndpointId() const;
  void setEndpointId(const string& endpoint_id);
  uint32_t getDestPort() const;
  void setDestPort(uint32_t _port);
  string getDestHost() const;
  void setDestHost(const string& _host);
private:
  string dest_host;
  uint32_t dest_port;
};

class MmsResponse : public MmsCommand {
public:
  MmsResponse():MmsCommand() {};
  ~MmsResponse() {};
  bool isRequest() {
    return false;
  }
};

}//mms
}//transport
}//scag

#endif

