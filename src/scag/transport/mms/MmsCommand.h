#ifndef __SCAG_TRANSPORT_MMS_COMMAND_HPP__
#define __SCAG_TRANSPORT_MMS_COMMAND_HPP__

#include <string>

#include "scag/transport/SCAGCommand.h"
#include "logger/Logger.h"
#include "MmsMsg.h"

namespace scag {
namespace transport {
namespace mms {

namespace status {
  extern const int SUCSESS;
  extern const int CLIENT_ERROR;
  extern const int SERVER_ERROR;
  extern const int VALIDATION_ERROR;
  extern const int UNSUPPORTED_OPERATION;
  extern const int SERVICE_ERROR;
  extern const int SERVICE_UNAVAILABLE;
  extern const int SERVICE_DENIED;
  extern const int ENDPOINT_NOT_REGISTRED;
  extern const int ROUTE_NOT_FOUND;
  extern const int MAX_STATUS_CODE;
}

namespace status_text {
  extern const string SUCSESS;
  extern const string CLIENT_ERROR;
  extern const string SERVER_ERROR;
  extern const string VALIDATION_ERROR;
  extern const string UNSUPPORTED_OPERATION;
  extern const string SERVICE_ERROR;
  extern const string SERVICE_UNAVAILABLE;
  extern const string SERVICE_DENIED;
  extern const string ENDPOINT_NOT_REGISTRED;
  extern const string ROUTE_NOT_FOUND;

  struct StatusHash : public smsc::core::buffers::IntHash<string> {
    StatusHash();
  };
}

using smsc::logger::Logger;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

using scag::sessions::SessionPtr;

//transaction context is not using now
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
  bool createResponse(const MmsMsg* _mms_msg, int status_code);
  bool createResponse(int status_code);
  bool createGenericError(const string& _transaction_id, int status_code, bool is_vasp);

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

