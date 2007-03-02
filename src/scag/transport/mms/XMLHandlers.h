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

#include <scag/util/encodings/Encodings.h>
#include <core/buffers/Hash.hpp>

#include "MmsMsg.h"
#include "MmsFactory.h"

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

static const uint8_t TRANSACTION_ID_TAG_NUMBER = 3;
static const uint8_t COMMAND_NAME_TAG_NUMBER = 5;

class XMLHandler : public HandlerBase {
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
public:
  XMLHandler(MmsFactory _factory);
  ~XMLHandler();
  void startElement(const XMLCh* const qname, AttributeList& attributes);
  void endElement(const XMLCh* const qname);
  void characters(const XMLCh* const ch, const unsigned int len);
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc);
  void fatalError(const SAXParseException& exc);
  void resetErrors();
  MmsMsg* getMmsMsg();
};
/*
class StrX {
  char* fLocalForm;
public:
  StrX(const XMLCh* const toTranscode) {
    fLocalForm = XMLString::transcode(toTranscode);
  }  
  ~StrX() {
    XMLString::release(&fLocalForm);
  }
  const char* localForm() const {
    return fLocalForm;
  }
};

class DOMPrintErrorHandler : public DOMErrorHandler {
  DOMPrintErrorHandler(const DOMErrorHandler&);
  void operator=(const DOMErrorHandler&);
public:
  DOMPrintErrorHandler() {};
  ~DOMPrintErrorHandler() {};
  bool handleError(const DOMError& dom_error) {
    StrX msg(dom_error.getMessage());
    if (dom_error.getSeverity() == DOMError::DOM_SEVERITY_WARNING) {
      __trace2__("DOM Print Warning Message : %s", msg.localForm());
      return true;
    }
    if (dom_error.getSeverity() == DOMError::DOM_SEVERITY_ERROR) {
      __trace2__("DOM Print Error Message : %s", msg.localForm());
      return true;
    }
    __trace2__("DOM Print Fatal Message : %s", msg.localForm());
    return true;
  };
  void resetErrors() {};
};

class XStr {
  XMLCh* unicode_form;
public:
  XStr(const char* const to_transcode) {
    unicode_form = XMLString::transcode(to_transcode);
  }  
  ~XStr() {
    XMLString::release(&unicode_form);
  }
  const XMLCh* unicodeForm() const {
    return unicode_form;
  }
};
*/

}//mms
}//transport
}//scag

#endif
