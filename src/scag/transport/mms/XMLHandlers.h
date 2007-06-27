#ifndef __SCAG_XML_HANDLER_H__
#define __SCAG_XML_HANDLER_H__

#include <cstdio>
#include <ctype.h>
#include <cctype>
#include <cstring>
#include <string.h>

#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>

#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/SAXParseException.hpp>


#include <scag/util/encodings/Encodings.h>
#include <core/buffers/Hash.hpp>

#include "MmsMsg.h"
#include "MmsFactory.h"
#include "logger/Logger.h"

//#include "DOMPrintErrorHandler.h"

namespace scag{
namespace transport{
namespace mms{

/*
namespace xml {
  static const char* TRUE = "true";
  static const char* FALSE = "false";
};
*/

XERCES_CPP_NAMESPACE_USE

XERCES_CPP_NAMESPACE_BEGIN
class AttributeList;
XERCES_CPP_NAMESPACE_END

using scag::util::encodings::Convertor;
using smsc::core::buffers::Hash;
using smsc::logger::Logger;

static const uint8_t TRANSACTION_ID_TAG_NUMBER = 3;
static const uint8_t COMMAND_NAME_TAG_NUMBER = 5;

class XMLHandler : public HandlerBase {

public:
  XMLHandler();
  ~XMLHandler();
  void startElement(const XMLCh* const qname, AttributeList& attributes);
  void endElement(const XMLCh* const qname);
  void characters(const XMLCh* const ch, const unsigned int len);
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc);
  void fatalError(const SAXParseException& exc);
  void resetErrors();
  bool hadSawErrors() const;
  MmsMsg* getMmsMsg();
  std::string getTransactionId() const;

private:
  std::string trimCharacters(const std::string& s);
  void startElementSubmit(const char* name, Hash<std::string>& attributes);
  void charactersSubmit(std::string value);
  void endElementSubmit(const char* name);
  void startElementDeliver(const char* name, Hash<std::string>& attributes);
  void charactersDeliver(std::string value);
  void endElementDeliver(const char* name);  
  void startElementCancel(const char* name, Hash<std::string>& attributes);
  void endElementCancel(const char* name);  
  void startElementExtendedReplace(const char* name, Hash<std::string>& attributes);
  void startElementDeliveryReport(const char* name, Hash<std::string>& attributes);
  void endElementDeliveryReport(const char* name);
  void startElementGenericResp(const char* name, Hash<std::string>& attributes);

private:
  Logger* logger;
  const Locator* locator;
  MmsMsg* mms_msg;
  MmsFactory factory;
  MultiAddress address;
  std::string tag_name;
  std::string transaction_id;
  Hash<std::string> soap_attributes;
  int tag_number;
  uint8_t command_id;
  bool fSawErrors;
};

}//mms
}//transport
}//scag

#endif
