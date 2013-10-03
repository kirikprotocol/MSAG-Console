#ifndef __SCAG_TRANSPORT_MMS_XML_HANDLER_H__
#define __SCAG_TRANSPORT_MMS_XML_HANDLER_H__

#include <string>

#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/sax/SAXParseException.hpp>

#include "core/buffers/Hash.hpp"
#include "logger/Logger.h"
#include "MmsMsg.h"
#include "MmsRouterTypes.h"
#include "util/xml/XercesMigration.h"

namespace scag{
namespace transport{
namespace mms{

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
  void characters(const XMLCh* const ch, const XERCES_UINT len);
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc);
  void fatalError(const SAXParseException& exc);
  MmsMsg* getMmsMsg();
  const string& getTransactionId() const;

private:
  void trimCharacters(string& s);
  void startElementSubmit(const char* name, Hash<string>& attributes);
  void charactersSubmit(const string& value);
  void endElementSubmit(const char* name);
  void startElementDeliver(const char* name, Hash<string>& attributes);
  void charactersDeliver(const string& value);
  void endElementDeliver(const char* name);  
  void startElementCancel(const char* name, Hash<string>& attributes);
  void endElementCancel(const char* name);  
  void startElementExtendedReplace(const char* name, Hash<string>& attributes);
  void startElementDeliveryReport(const char* name, Hash<string>& attributes);
  void endElementDeliveryReport(const char* name);
  void startElementGenericResp(const char* name, Hash<string>& attributes);

private:
  Logger* logger;
  const Locator* locator;
  MmsMsg* mms_msg;
  MultiAddress address;
  string tag_name;
  string transaction_id;
  Hash<string> soap_attributes;
  int tag_number;
  uint8_t command_id;
};


class RouterXMLHandler : public HandlerBase {
public:
  RouterXMLHandler(RouteHash* r);
  ~RouterXMLHandler() {};
  void startElement(const XMLCh* const qname, AttributeList& attributes);
  void endElement(const XMLCh* const qname);
  void characters(const XMLCh* const ch, const unsigned int len) {};
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc);
  void fatalError(const SAXParseException& exc);
private:
  Logger* logger;
  RouteHash* routes_hash;
  MmsRouteInfo route;
  bool source;
};

}//mms
}//transport
}//scag

#endif
