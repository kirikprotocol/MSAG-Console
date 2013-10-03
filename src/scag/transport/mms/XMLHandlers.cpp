#include "scag/util/encodings/Encodings.h"
#include "util/Exception.hpp"
#include "XMLHandlers.h"
#include "MmsFactory.h"

namespace scag {
namespace transport {
namespace mms {

using smsc::util::Exception;

XMLHandler::XMLHandler():tag_number(0), command_id(MMS_UNKNOWN), mms_msg(0), logger(0) {
  logger = Logger::getInstance("mms.xml");
}

void XMLHandler::startElement(const XMLCh* const qname, AttributeList& attributes) {
  ++tag_number;
  StrX tag(qname);
  tag_name = tag.localForm();
  if (COMMAND_NAME_TAG_NUMBER == tag_number) {
    mms_msg = MmsFactory::getMmsMsg(tag.localForm(), transaction_id);
    if (mms_msg) {
      command_id = mms_msg->getCommandId();
      char* name = 0;
      string value;
      soap_attributes.First();
      while (soap_attributes.Next(name, value)) {
        mms_msg->setInfoElement(name, value);
      }
    }
    return;
  }
  Hash<string> attr;
  size_t len = attributes.getLength();
  for (int i = 0; i < len; ++i) {
    XMLCh const * xml_name = attributes.getName(i);
    XMLCh const * xml_value = attributes.getValue(i);
    string name, value;
    //scag::util::encodings::Convertor::UCS2ToUTF8(xml_name, XMLString::stringLen(xml_name), name);
    Convertor::UCS2ToUTF8(xml_name, XMLString::stringLen(xml_name), name);
    Convertor::UCS2ToUTF8(xml_value, XMLString::stringLen(xml_value), value);
    attr.Insert(name.c_str(), value);
  }
  if (strcmp(tag.localForm(), xml::MM7_TRANSACTION_ID) == 0) {
    soap_attributes = attr;
  }
  switch (command_id) {
    case MM7_READ_REPLY       :
    case MM7_EXTENDED_CANCEL  : 
    case MM7_REPLACE          :
    case MM7_CANCEL           : startElementCancel(tag.localForm(), attr); break;
    case MM7_EXTENDED_REPLACE : startElementExtendedReplace(tag.localForm(), attr); break;
    case MM7_DELIVERY_REPORT  : startElementDeliveryReport(tag.localForm(), attr); break;
    case MM7_SUBMIT           : startElementSubmit(tag.localForm(), attr); break;
    case MM7_DELIVER          : startElementDeliver(tag.localForm(), attr); break;
    case MMS_GENERIC_RESP     : startElementGenericResp(tag.localForm(), attr); break;
    //case MM7_RS_ERROR_RESP    : 
    //case MM7_VASP_ERROR_RESP  : break;
  }
}

void XMLHandler::startElementSubmit(const char* name, Hash<std::string>& attributes) {
  startElementCancel(name, attributes);
  if (strcmp(name, xml::REPLY_CHARGING) == 0 || strcmp(name, xml::CONTENT) == 0) {
    char* name = 0;
    string value;
    attributes.First();
    while (attributes.Next(name, value)) {
      mms_msg->setInfoElement(name, value);
    }
  }
}

void XMLHandler::startElementDeliver(const char* name, Hash<std::string>& attributes) {
  startElementSubmit(name, attributes);
  if (strcmp(name, xml::DATE_TIME) == 0 || strcmp(name, xml::USER_AGENT) == 0) {
    const string* ptr = attributes.GetPtr(xml::SEQUENCE);
    if (ptr) {
      dynamic_cast<MM7Deliver *>(mms_msg)->setSequenceNumber(atoi(ptr->c_str()));
    }
  }
  if (strcmp(name, xml::UA_CAPABILITIES) == 0) {
    const string* ptr = attributes.GetPtr(xml::UA_PROF);
    if (ptr) {
      mms_msg->setInfoElement(xml::UA_PROF, *ptr);
    }
    ptr = attributes.GetPtr(xml::TIME_STAMP);
    if (ptr) {
      mms_msg->setInfoElement(xml::UA_TIME_STAMP, *ptr);
    }
  }

}

void XMLHandler::startElementCancel(const char* name, Hash<std::string>& attributes) {
  if (strcmp(name, xml::NUMBER) == 0) {
    const string* ptr = attributes.GetPtr(xml::DISPLAY_ONLY);
    if (ptr) {
      address.number.setDisplayOnly(ptr->c_str());
    }
    ptr = attributes.GetPtr(xml::ADDRESS_CODING);
    if (ptr) {
      address.number.setCodingType(ptr->c_str());
    }
    return;
  }
  if (strcmp(name, xml::SHORT_CODE) == 0) {
    const string* ptr = attributes.GetPtr(xml::DISPLAY_ONLY);
    if (ptr) {
      address.short_code.setDisplayOnly(ptr->c_str());
    }
    ptr = attributes.GetPtr(xml::ADDRESS_CODING);
    if (ptr) {
      address.short_code.setCodingType(ptr->c_str());
    }
    return;
  }
  if (strcmp(name, xml::RFC2822) == 0) {
    const string* ptr = attributes.GetPtr(xml::DISPLAY_ONLY);
    if (ptr) {
      address.rfc2822.setDisplayOnly(ptr->c_str());
    }
    ptr = attributes.GetPtr(xml::ADDRESS_CODING);
    if (ptr) {
      address.rfc2822.setCodingType(ptr->c_str());
    }
    return;
  }
}

void XMLHandler::startElementExtendedReplace(const char* name, Hash<std::string>& attributes) {
  startElementCancel(name, attributes);
  if (strcmp(name, xml::CONTENT) == 0) {
    char* name = 0;
    string value;
    attributes.First();
    while (attributes.Next(name, value)) {
      mms_msg->setInfoElement(name, value);
    }
  }
}

void XMLHandler::startElementDeliveryReport(const char* name, Hash<std::string>& attributes) {
  startElementCancel(name, attributes);
  if (strcmp(name, xml::UA_CAPABILITIES) == 0) {
    const string* ptr = attributes.GetPtr(xml::UA_PROF);
    if (ptr) {
      mms_msg->setInfoElement(xml::UA_PROF, *ptr);
    }
    ptr = attributes.GetPtr(xml::TIME_STAMP);
    if (ptr) {
      mms_msg->setInfoElement(xml::UA_TIME_STAMP, *ptr);
    }
  }
}

void XMLHandler::startElementGenericResp(const char* name, Hash<std::string>& attributes) {
  if (strcmp(name, mm7_command_name::RS_ERROR_RESP) == 0) {
    mms_msg->setCommandId(MM7_RS_ERROR_RESP);
  }
  if (strcmp(name, mm7_command_name::VASP_ERROR_RESP) == 0) {
    mms_msg->setCommandId(MM7_VASP_ERROR_RESP);
  }
}

void XMLHandler::characters(const XMLCh* const ch, const XERCES_UINT len) {
  string value;
  //XMLString::trim(ch);
  Convertor::UCS2ToUTF8(ch, len, value);
  trimCharacters(value);
  if (TRANSACTION_ID_TAG_NUMBER == tag_number && tag_name.compare(xml::MM7_TRANSACTION_ID) == 0){
    transaction_id = value;
    return;
  }
  switch (command_id) {
    case MMS_UNKNOWN : return;
    //case MM7_EXTENDED_REPLACE: return;
    case MM7_DELIVERY_REPORT  :
    case MM7_EXTENDED_CANCEL  :
    case MM7_READ_REPLY       :
    case MM7_DELIVER          :
    case MM7_REPLACE          :
    case MM7_CANCEL           :
    case MM7_SUBMIT           : charactersSubmit(value); break;
    //case MM7_RS_ERROR_RESP    : 
    //case MM7_VASP_ERROR_RESP  : break;
    case MM7_EXTENDED_REPLACE : mms_msg->setInfoElement(tag_name.c_str(), value); break;
    default                   : mms_msg->setInfoElement(tag_name.c_str(), value); break;
  }
  tag_name = "";
}

void XMLHandler::charactersSubmit(const string& value) {
  if (tag_name.compare(xml::NUMBER) == 0) {
    address.number.setValue(value);
    return;
  }
  if (tag_name.compare(xml::SHORT_CODE) == 0) {
    address.short_code.setValue(value);
    return;
  }
  if (tag_name.compare(xml::RFC2822) == 0) {
    address.rfc2822.setValue(value);
    return;
  }
  if (command_id == MM7_DELIVER && tag_name.compare(xml::DATE_TIME) == 0) {
    MM7Deliver* deliver_msg = dynamic_cast<MM7Deliver*>(mms_msg);
    deliver_msg->setPreviouslySentDate(value);
    return;
  }
  mms_msg->setInfoElement(tag_name.c_str(), value);
}

void XMLHandler::endElement(const XMLCh* const qname) {
  StrX name(qname);
  switch (command_id) {
    case MM7_EXTENDED_CANCEL : 
    case MM7_REPLACE         :
    case MM7_CANCEL          : endElementCancel(name.localForm()); break;
    case MM7_DELIVERY_REPORT :
    case MM7_READ_REPLY      : endElementDeliveryReport(name.localForm()); break;
    //case MM7_RS_ERROR_RESP   :
    //case MM7_VASP_ERROR_RESP : break;
    case MM7_SUBMIT          : endElementSubmit(name.localForm()); break;
    case MM7_DELIVER         : endElementDeliver(name.localForm()); break;
  }
}

void XMLHandler::endElementSubmit(const char* name) {
  
  if (std::strcmp(name, xml::TO) == 0) {
    mms_msg->setRecipientAddress(address);
    //MM7Submit* submit_msg = dynamic_cast<MM7Submit*>(mms_msg);
    //submit_msg->setRecipientAddress(address);
    address.reset();
    return;
  }
  if (strcmp(name, xml::CC) == 0) {
    mms_msg->setRecipientAddress(address, CC);
    address.reset();
    return;
  }
  if (strcmp(name, xml::BCC) == 0) {
    mms_msg->setRecipientAddress(address, BCC);
    address.reset();
    return;
  }
  if (strcmp(name, xml::SENDER_ADDRESS) == 0) {
    mms_msg->setSenderAddress(address);
    address.reset();
    return;
  }
}

void XMLHandler::endElementDeliver(const char* name) {
  endElementSubmit(name);
  if (std::strcmp(name, xml::SENDER) == 0) {
    mms_msg->setSenderAddress(address);
    address.reset();
    return;
  }
  if (std::strcmp(name, xml::USER_AGENT) == 0) {
    dynamic_cast<MM7Deliver*>(mms_msg)->setPreviouslySentBy(address);
    address.reset();
    return;
  }
}

void XMLHandler::endElementCancel(const char* name) {
  if (std::strcmp(name, xml::SENDER_ADDRESS) == 0) {
    mms_msg->setSenderAddress(address);
    address.reset();
  }
}

void XMLHandler::endElementDeliveryReport(const char* name) {
  if (strcmp(name, xml::SENDER) == 0) {
    mms_msg->setSenderAddress(address);
    address.reset();
  }
  if (strcmp(name, xml::RECIPIENT) == 0) {
    mms_msg->setRecipientAddress(address);
    address.reset();
    return;
  }
}

void XMLHandler::warning(const SAXParseException& exc) {
  StrX msg(exc.getMessage());
  smsc_log_warn(logger, "SAX Parse Warning : %s", msg.localForm());
}

void XMLHandler::error(const SAXParseException& exc) {
  StrX msg(exc.getMessage());
  StrX system_id(exc.getSystemId());
  throw Exception("SAX Parse Error in \"%s\" line:%d column:%d \n MESSAGE : %s",
             system_id.localForm(), exc.getLineNumber(), exc.getColumnNumber(),  msg.localForm());
}

void XMLHandler::fatalError(const SAXParseException& exc) {
  StrX msg(exc.getMessage());
  StrX system_id(exc.getSystemId());
  throw Exception("SAX Parse Fatal Eerror in \"%s\" line:%d column:%d \n MESSAGE : %s",
             system_id.localForm(), exc.getLineNumber(), exc.getColumnNumber(),  msg.localForm());
}

void XMLHandler::trimCharacters(string& s) {
  string::iterator start = s.begin();
  string::iterator end = s.end() - 1;
  while (start != s.end() && isspace(*start)) ++start;
  while (end != start && isspace(*end)) --end;
  if (end - start >= 0) {
    s.erase(s.begin(), start);
    s.erase(end + 1, s.end());
  } else {
    s.erase();
  }
}

MmsMsg* XMLHandler::getMmsMsg() {
  MmsMsg* _mms_msg = NULL;
  if (command_id && mms_msg) {
    _mms_msg = mms_msg;
  }
  mms_msg = 0;
  return _mms_msg;
}

const string& XMLHandler::getTransactionId() const {
  return transaction_id;
}

XMLHandler::~XMLHandler() {
  if (mms_msg) {
    delete mms_msg;
  }
}

RouterXMLHandler::RouterXMLHandler(RouteHash* r) {
  logger = Logger::getInstance("mms.xml");
  routes_hash = r;
}

void RouterXMLHandler::startElement(const XMLCh *const name, AttributeList &attrs) {
  StrX XMLQName(name);
  const char *qname = XMLQName.localForm();
  if (strcmp(qname, "route") == 0) {
    StrX s = attrs.getValue("name");
    route.name = s.localForm();
    route.srcId = StrX(attrs.getValue("srcEndpointId")).localForm();
    StrX s1 = attrs.getValue("id");
    route.id = atoi(s1.localForm());
    if(!route.id)
        throw Exception("Invalid XML route id: name = %s, id = %s", s.localForm(), s1.localForm());
    StrX s2 = attrs.getValue("serviceId");
    route.service_id = atoi(s2.localForm());
    if(!route.service_id)
        throw Exception("Invalid XML service id: name = %s, id = %s, service_id=%s", s.localForm(), s1.localForm(), s2.localForm());
    route.enabled = stringToBool(StrX(attrs.getValue("enabled")).localForm(), true);
    return;
  }
  if (strcmp(qname, "source") == 0) {
    source = true;
    //route.sourceId = StrX(attrs.getValue("value")).localForm();
    return;
  }
  if (strcmp(qname, "destination") == 0) {
    source = false;
    route.destId = StrX(attrs.getValue("endpointId")).localForm();
    return;
  }
  if (strcmp(qname, "mask") == 0) {
    string mask = StrX(attrs.getValue("value")).localForm();
    if (source) {
      route.srcMask = mask;
    } else {
      route.destMask = mask;
    }
  }
}

void RouterXMLHandler::endElement(const XMLCh *const name) {
  StrX XMLQName(name);
  const char *qname = XMLQName.localForm();
  if (strcmp(qname, "route") == 0) {
    routes_hash->Insert(route.srcId.c_str(), route);
  }
}

void RouterXMLHandler::error(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    throw Exception("Error at file %s, line %d, char %d Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void RouterXMLHandler::fatalError(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    throw Exception("Fatal Error at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

void RouterXMLHandler::warning(const SAXParseException& e)
{
    StrX fname(e.getSystemId());
    StrX msg(e.getMessage());
    smsc_log_error(logger, "Warning at file %s, line %d, char %d   Message: %s",fname.localForm(),e.getLineNumber(),e.getColumnNumber(),msg.localForm());
}

}//mms
}//transport
}//scag
