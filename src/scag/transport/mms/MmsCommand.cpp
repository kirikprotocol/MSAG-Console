#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/SAXParser.hpp>

#include "MmsCommand.h"
#include "XMLHandlers.h"

namespace scag{
namespace transport{
namespace mms {

namespace status {
  const uint32_t SUCSESS                = 1000;
  const uint32_t VALIDATION_ERROR       = 4004;
  const uint32_t SERVICE_ERROR          = 4005;
  const uint32_t SERVICE_UNAVAILABLE    = 4006;
  const uint32_t SERVICE_DENIED         = 4007;
  const uint32_t ENDPOINT_NOT_REGISTRED = 4010;
  const uint32_t ROUTE_NOT_FOUND        = 4011;
}

namespace status_text {
  const char* SUCSESS                = "Success";
  const char* VALIDATION_ERROR       = "Validation Error";
  const char* SERVICE_ERROR          = "Service Error";
  const char* SERVICE_UNAVAILABLE    = "Service Unavailable";
  const char* SERVICE_DENIED         = "Service Denied";
  const char* ENDPOINT_NOT_REGISTRED = "Endpoint Not Regisetred";
  const char* ROUTE_NOT_FOUND        = "Route Not Found";
}


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
  switch (st) {
  case status::SUCSESS : 
    mms_msg->setInfoElement(xml::STATUS_CODE, "1000");
    mms_msg->setInfoElement(xml::STATUS_TEXT, status_text::SUCSESS);
    return;
  case status::VALIDATION_ERROR :
     mms_msg->setInfoElement(xml::STATUS_CODE, "4004");
     mms_msg->setInfoElement(xml::STATUS_TEXT, status_text::VALIDATION_ERROR);
     return;
  case status::SERVICE_ERROR :
     mms_msg->setInfoElement(xml::STATUS_CODE, "4005");
     mms_msg->setInfoElement(xml::STATUS_TEXT, status_text::SERVICE_ERROR);
     return;
  case status::SERVICE_UNAVAILABLE :
     mms_msg->setInfoElement(xml::STATUS_CODE, "4006");
     mms_msg->setInfoElement(xml::STATUS_TEXT, status_text::SERVICE_UNAVAILABLE);
     return;
  case status::SERVICE_DENIED :
     mms_msg->setInfoElement(xml::STATUS_CODE, "4007");
     mms_msg->setInfoElement(xml::STATUS_TEXT, status_text::SERVICE_DENIED);
     return;
  case status::ENDPOINT_NOT_REGISTRED :
     mms_msg->setInfoElement(xml::STATUS_CODE, "4010");
     mms_msg->setInfoElement(xml::STATUS_TEXT, status_text::ENDPOINT_NOT_REGISTRED);
     return;
  case status::ROUTE_NOT_FOUND :
     mms_msg->setInfoElement(xml::STATUS_CODE, "4011");
     mms_msg->setInfoElement(xml::STATUS_TEXT, status_text::ROUTE_NOT_FOUND);
     return;
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
