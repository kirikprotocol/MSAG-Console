#include "MmsCommand.h"

namespace scag{
namespace transport{
namespace mms {

MmsCommand::MmsCommand():mms_msg(0), logger(0) {
  logger = Logger::getInstance("scag.mms");
}

MmsCommand::MmsCommand(const char* http_buf, size_t buf_size) {
  MmsCommand();
}

//int MmsCommand::getServiceId() const {
//
//}

uint8_t MmsCommand::getCommandId() const {

  if (mms_msg) {
    return mms_msg->getCommandId(); 
  } else {
    return MMS_UNKNOWN;
  }
}

std::string MmsCommand::getInfoElement(const char* element_name) const {
  if (mms_msg) {
    return mms_msg->getInfoElement(element_name);
  } else {
    return "";
  }
}

void MmsCommand::setInfoElement(const char* name, std::string value) {
  if (mms_msg) {
    mms_msg->setInfoElement(name, value);
  }
}

const MmsMsg* MmsCommand::getMmsMsg() const {
  return mms_msg;
}

bool MmsCommand::createMM7Command(const char* soap_envelope, size_t envelope_size) {
  if (!soap_envelope || !envelope_size) {
    smsc_log_error(logger, "MmsCommand::createCommand : Empty SOAP Envelope");
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
    if (handler.hadSawErrors()) {
      smsc_log_debug(logger, "MmsCommand::createCommand : XMLHandler had saw Errors!");
      return  false;
    }
  }
  catch (const OutOfMemoryException& e) {
    StrX strx(e.getMessage());
    smsc_log_error(logger, "MmsCommand::createCommand : Out of Memory Exception", strx.localForm());
    return false;
  }
  catch (const XMLException& e) {
    StrX strx(e.getMessage());
    smsc_log_error(logger, "MmsCommand::createCommand : XML Exception", strx.localForm());
    return false;
  }
  catch (...) {
    smsc_log_error(logger, "MmsCommand::createCommand : Some Exception");
    return false;
  }
  if (mms_msg) {
    return true;
  } else {
    return false;
  }
}

bool MmsCommand::createResponse(const MmsMsg* _mms_msg, std::string status_code, std::string status_text) {
  if ((!_mms_msg->isMM7Req() && !_mms_msg->isMM4Req()) || !_mms_msg) {
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
  //mms_msg->addField(xml::STATUS_CODE, status_code);
  //mms_msg->addField(xml::STATUS_TEXT, status_text);
  return true;
}

bool MmsCommand::createRSError(std::string _transaction_id, std::string status_code, std::string status_text) {
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

bool MmsCommand::createVASPError(std::string _transaction_id, std::string status_code, std::string status_text) {
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

std::string MmsCommand::serialize() const {
  if (!mms_msg) {
    return "";
  }
  try {
    return mms_msg->serialize();
  } catch (const Exception& e) {
    smsc_log_error(logger, "MmsCommand::serialize : %s", e.what());
    return "";
  }
}

void MmsCommand::setTransactionId(string _id) {
  if (mms_msg) {
    mms_msg->setTransactionId(_id);
  }
}

std::string MmsCommand::getTransactionId() const {
  if (mms_msg) {
    return mms_msg->getTransactionId();
  } else {
    return transaction_id;
  }
}

//void MmsCommand::addInfoElement(const char* name, string value) {
  //mms_msg->addField(name, value);
//}

MmsCommand::~MmsCommand() {
  if (mms_msg) {
    delete mms_msg;
  }
}

}//mms
}//transport
}//scag
