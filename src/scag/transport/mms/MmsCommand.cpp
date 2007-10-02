#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/SAXParser.hpp>

#include "core/buffers/IntHash.hpp"
#include "MmsCommand.h"
#include "XMLHandlers.h"

namespace scag{
namespace transport{
namespace mms {

namespace status {
  const int SUCSESS                = 1000;
  const int VALIDATION_ERROR       = 4004;
  const int SERVICE_ERROR          = 4005;
  const int SERVICE_UNAVAILABLE    = 4006;
  const int SERVICE_DENIED         = 4007;
  const int ENDPOINT_NOT_REGISTRED = 4010;
  const int ROUTE_NOT_FOUND        = 4011;
  const int MAX_STATUS_CODE        = 4999;
}

namespace status_text {
  const string SUCSESS                = "Success";
  const string VALIDATION_ERROR       = "Validation Error";
  const string SERVICE_ERROR          = "Service Error";
  const string SERVICE_UNAVAILABLE    = "Service Unavailable";
  const string SERVICE_DENIED         = "Service Denied";
  const string ENDPOINT_NOT_REGISTRED = "Endpoint Not Registered";
  const string ROUTE_NOT_FOUND        = "Route Not Found";

  StatusHash::StatusHash() {
    Insert(status::SUCSESS, SUCSESS);
    Insert(status::VALIDATION_ERROR, VALIDATION_ERROR);
    Insert(status::SERVICE_ERROR, SERVICE_ERROR);
    Insert(status::SERVICE_UNAVAILABLE, SERVICE_UNAVAILABLE);
    Insert(status::SERVICE_DENIED, SERVICE_DENIED);
    Insert(status::ENDPOINT_NOT_REGISTRED, ENDPOINT_NOT_REGISTRED);
    Insert(status::ROUTE_NOT_FOUND, ROUTE_NOT_FOUND);
  };

  const StatusHash statusHash;
};


MmsCommand::MmsCommand():mms_msg(0), logger(0) {
  logger = Logger::getInstance("mms.cmd");
}

MmsCommand::MmsCommand(const char* http_buf, size_t buf_size) {
  MmsCommand();
}

uint8_t MmsCommand::getCommandId() const {

  return mms_msg ? mms_msg->getCommandId() : MMS_UNKNOWN;

}

string MmsCommand::getInfoElement(const char* element_name) const {
  if (!mms_msg) {
    return string("");
  }
  const string* value = mms_msg->getInfoElement(element_name);
  return value ? string(*value) : string("");
}

void MmsCommand::setInfoElement(const char* name, const string& value) {
  if (mms_msg) {
    mms_msg->setInfoElement(name, value);
  }
}

const MmsMsg* MmsCommand::getMmsMsg() const {
  return mms_msg;
}

bool MmsCommand::createMM7Command(const char* soap_envelope, size_t envelope_size) {
  if (!soap_envelope || !envelope_size) {
    smsc_log_error(logger, "createCommand: Empty SOAP Envelope");
    return false;
  }
  if (mms_msg) {
    delete mms_msg;  
    mms_msg = NULL;
  }
  MemBufInputSource memBufIS
  (
    (const XMLByte*) soap_envelope,
    envelope_size,
    "MMS message", //how to set this papmeter with some useful information, like transaction ID
    false
  );
  SAXParser parser;
  parser.setValidationScheme(SAXParser::Val_Always);
  parser.setDoNamespaces(true);
  parser.setDoSchema(true);
  parser.setValidationSchemaFullChecking(true);
  parser.setExternalSchemaLocation(xml::SCHEMA_LOCATION);
  scag::transport::mms::XMLHandler handler;
  parser.setErrorHandler(&handler);
  parser.setDocumentHandler(&handler);
  try {
    parser.parse(memBufIS);
    mms_msg = handler.getMmsMsg();
    transaction_id = handler.getTransactionId();
  }
  catch (const Exception& e) {
    transaction_id = handler.getTransactionId();
    smsc_log_error(logger, "createCommand: %s", e.what());
    return false;
  }
  catch (const OutOfMemoryException& e) {
    StrX strx(e.getMessage());
    smsc_log_error(logger, "createCommand: Out of Memory Exception", strx.localForm());
    return false;
  }
  catch (const XMLException& e) {
    StrX strx(e.getMessage());
    smsc_log_error(logger, "createCommand: XML Exception", strx.localForm());
    return false;
  }
  catch (...) {
    smsc_log_error(logger, "createCommand: Some Exception");
    return false;
  }

  return mms_msg ? true : false;
}

bool MmsCommand::createResponse(const MmsMsg* _mms_msg,
                                const string& status_code, const string& status_text) {
  if (!_mms_msg || (!_mms_msg->isMM7Req() && !_mms_msg->isMM4Req())) {
    return false;
  }
  if (mms_msg) {
    delete mms_msg;
    mms_msg = NULL;
  }
  transaction_id = _mms_msg->getTransactionId();
  mms_msg = _mms_msg->getResponse();
  mms_msg->setInfoElement(xml::STATUS_CODE, status_code);
  mms_msg->setInfoElement(xml::STATUS_TEXT, status_text);
  return true;
}

bool MmsCommand::createResponse(const string& status_code, const string& status_text) {
  if (!mms_msg || (!mms_msg->isMM7Req() && !mms_msg->isMM4Req())) {
    return false;
  }
  MmsMsg *resp_msg = mms_msg->getResponse();
  delete mms_msg;
  mms_msg = resp_msg;
  mms_msg->setInfoElement(xml::STATUS_CODE, status_code);
  mms_msg->setInfoElement(xml::STATUS_TEXT, status_text);
  return true;
}

bool MmsCommand::createRSError(const string& _transaction_id,
                               const string& status_code, const string& status_text) {
  if (mms_msg) {
    delete mms_msg;
    mms_msg = NULL;
  }
  mms_msg = new MM7RSErrorResp(_transaction_id);
  mms_msg->setInfoElement(xml::STATUS_CODE, status_code);
  mms_msg->setInfoElement(xml::STATUS_TEXT, status_text);
  mms_msg->setInfoElement(xml::FAULT_CODE, "env:Client");
  mms_msg->setInfoElement(xml::FAULT_STRING, "Client Error");
  return true;
}

bool MmsCommand::createVASPError(const string& _transaction_id,
                                 const string& status_code, const string& status_text) {
  if (mms_msg) {
    delete mms_msg;
    mms_msg = NULL;
  }
  mms_msg = new MM7VASPErrorResp(_transaction_id);
  mms_msg->setInfoElement(xml::STATUS_CODE, status_code);
  mms_msg->setInfoElement(xml::STATUS_TEXT, status_text);
  mms_msg->setInfoElement(xml::FAULT_CODE, "env:Client");
  mms_msg->setInfoElement(xml::FAULT_STRING, "Client Error");
  return true;
}

bool MmsCommand::createResponse(const MmsMsg* _mms_msg, int status_code) {
  if (!_mms_msg || (!_mms_msg->isMM7Req() && !_mms_msg->isMM4Req())) {
    return false;
  }
  if (mms_msg) {
    delete mms_msg;
    mms_msg = NULL;
  }
  transaction_id = _mms_msg->getTransactionId();
  mms_msg = _mms_msg->getResponse();
  setStatus(status_code);
  return true;
}

bool MmsCommand::createResponse(int status_code) {
  if (!mms_msg || (!mms_msg->isMM7Req() && !mms_msg->isMM4Req())) {
    return false;
  }
  MmsMsg *resp_msg = mms_msg->getResponse();
  delete mms_msg;
  mms_msg = resp_msg;
  setStatus(status_code);
  return true;
}

bool MmsCommand::createRSError(const string& _transaction_id, int status_code) {
  if (mms_msg) {
    delete mms_msg;
    mms_msg = NULL;
  }
  mms_msg = new MM7RSErrorResp(_transaction_id);
  setStatus(status_code);
  mms_msg->setInfoElement(xml::FAULT_CODE, "env:Client");
  mms_msg->setInfoElement(xml::FAULT_STRING, "Client Error");
  return true;
}

bool MmsCommand::createVASPError(const string& _transaction_id, int status_code) {
  if (mms_msg) {
    delete mms_msg;
    mms_msg = NULL;
  }
  mms_msg = new MM7VASPErrorResp(_transaction_id);
  setStatus(status_code);
  mms_msg->setInfoElement(xml::FAULT_CODE, "env:Client");
  mms_msg->setInfoElement(xml::FAULT_STRING, "Client Error");
  return true;
}

bool MmsCommand::serialize(string& serialized_cmd) const {
  if (!mms_msg) {
    smsc_log_error(logger, "serialize: mms_msg=NULL");
    return false;
  }
  try {
    return mms_msg->serialize(serialized_cmd);
  } catch (const Exception& e) {
    smsc_log_error(logger, "serialize: %s", e.what());
    return false;
  }
}

void MmsCommand::setTransactionId(const string& _id) {
  if (mms_msg) {
    mms_msg->setTransactionId(_id);
  }
}

const string& MmsCommand::getTransactionId() const {
  return mms_msg ? mms_msg->getTransactionId() : transaction_id;
}

void MmsCommand::setStatus(int st) {
  if (st < status::SUCSESS || st > status::MAX_STATUS_CODE) {
    smsc_log_warn(logger, "error status code %d", st);
    return;
  }
  char buf[5];
  snprintf(buf, 5, "%04d", st);
  buf[4] = 0;
  mms_msg->setInfoElement(xml::STATUS_CODE, buf);
  string *ptr = status_text::statusHash.GetPtr(st);
  if (ptr) {
    mms_msg->setInfoElement(xml::STATUS_TEXT, *ptr);
  } else {
    smsc_log_warn(logger, "unknown status code %d", st);
  }
}

SessionPtr MmsCommand::getSession() {
  return session;
}

void MmsCommand::setSession(SessionPtr& s) {
  session = s;
}

bool MmsCommand::hasSession() {
  return session.Get();
}

MmsCommand::~MmsCommand() {
  if (mms_msg) {
    delete mms_msg;
  }
}

MmsRequest::MmsRequest():MmsCommand(), dest_port(0) {
}

string MmsRequest::getEndpointId() const {
  if (!mms_msg) {
    return string("");
  }
  const string* ep_id = mms_msg->getEndpointId();
  return ep_id ? string(*ep_id) : string("");
}

void MmsRequest::setEndpointId(const string& endpoint_id) {
  if (mms_msg) {
    mms_msg->setEndpointId(endpoint_id);
  }
}

string MmsRequest::getDestHost() const {
  return dest_host;
}

void MmsRequest::setDestHost(const string& host) {
  dest_host = host;
}

uint32_t MmsRequest::getDestPort() const {
  return dest_port;
}

void MmsRequest::setDestPort(uint32_t port) {
  dest_port = port;
}

}//mms
}//transport
}//scag
