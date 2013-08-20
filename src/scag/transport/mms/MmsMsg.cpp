#include "util/Exception.hpp"
#include "MmsMsg.h"
#include "util.h"

namespace scag{
namespace transport{
namespace mms {

static const char* DEFAULT_MMS_VERSION = "5.6.0";

namespace mm7_command_name {
  const char* SUBMIT                = "SubmitReq";
  const char* SUBMIT_RESP           = "SubmitRsp";
  const char* DELIVER               = "DeliverReq";
  const char* DELIVER_RESP          = "DeliverRsp";
  const char* CANCEL                = "CancelReq";
  const char* CANCEL_RESP           = "CancelRsp";
  const char* REPLACE               = "ReplaceReq";
  const char* REPLACE_RESP          = "ReplaceRsp";
  const char* DELIVERY_REPORT       = "DeliveryReportReq";
  const char* DELIVERY_REPORT_RESP  = "DeliveryReportRsp";
  const char* READ_REPLY            = "ReadReplyReq";
  const char* READ_REPLY_RESP       = "ReadReplyRsp";
  const char* EXTENDED_CANCEL       = "extendedCancelReq";
  const char* EXTENDED_CANCEL_RESP  = "extendedCancelRsp";    
  const char* EXTENDED_REPLACE      = "extendedReplaceReq";
  const char* EXTENDED_REPLACE_RESP = "extendedReplaceRsp";
  const char* RS_ERROR_RESP         = "RSErrorRsp";
  const char* VASP_ERROR_RESP       = "VASPErrorRsp";
  const char* FAULT                 = "env:Fault";
};


Address::Address():display_only(false), coding_type(0) {}

void Address::setCodingType(const char* type) {
  if (std::strcmp(type, xml::ENCRYPTED) == 0) {
    coding_type = ENCRYPTED;
  }
  if (std::strcmp(type, xml::OBFUSCATED) == 0) {
    coding_type = OBFUSCATED;
  }
}

void Address::setCodingType(uint8_t type) {
  if (ENCRYPTED == type || OBFUSCATED ==type) {
    coding_type = type;
  }
}

uint8_t Address::getCodingType() const {
  return coding_type;
}

void Address::setDisplayOnly(const char* _display_only) {
  display_only = stringToBool(_display_only);
}

void Address::setDisplayOnly(bool _display_only) {
  display_only = _display_only;
}

bool Address::getDisplayOnly() const {
  return display_only;
}

bool Address::isNotSet() const {
  if (value.empty()) {
    return true;
  } else {
    return false;
  }
}

void Address::setValue(const string& _value) {
  value = _value;
}

const string& Address::getValue() const {
  return value;
}

void Address::serialize(DOMDocument* doc, DOMElement* parent, const char* address_type) const {
  if (value.empty() || !address_type) {
    return;
  }
  DOMElement* address_element = doc->createElement(XStr(address_type).unicodeForm());
  if (display_only) {
    address_element->setAttribute(XStr(xml::DISPLAY_ONLY).unicodeForm(), XStr(xml::TRUE).unicodeForm());
  }
  if (ENCRYPTED == coding_type) {
    address_element->setAttribute(XStr(xml::ADDRESS_CODING).unicodeForm(), XStr(xml::ENCRYPTED).unicodeForm());
  }
  if (OBFUSCATED == coding_type) {
    address_element->setAttribute(XStr(xml::ADDRESS_CODING).unicodeForm(), XStr(xml::OBFUSCATED).unicodeForm());
  }
  DOMText* address_value = doc->createTextNode(XStr(value.c_str()).unicodeForm());
  address_element->appendChild(address_value);
  parent->appendChild(address_element);
}

void Address::reset() {
  display_only = false;
  coding_type = 0;
  value = "";
}

void Address::test() const {
  if (isNotSet()) {
    return;
  }
  __trace2__("Address value = %s", value.c_str());
  if (display_only) {
    __trace__("Address is display only");
  }
  __trace2__("Encoding type = %d", coding_type);
}

Address::~Address() {}

bool MultiAddress::isNotSet() const {
  if (number.isNotSet() && short_code.isNotSet() && rfc2822.isNotSet()) {
    return true;
  } else {
    return false;
  }
}

void MultiAddress::serialize(DOMDocument* doc, DOMElement* parent,
                              const char* parent_tag_name) const {
  if (!parent_tag_name || isNotSet()) {
    return;
  }
  DOMElement* address_element = doc->createElement(XStr(parent_tag_name).unicodeForm());
  number.serialize(doc, address_element, xml::NUMBER);
  short_code.serialize(doc, address_element, xml::SHORT_CODE);
  rfc2822.serialize(doc, address_element, xml::RFC2822);
  parent->appendChild(address_element);
}

void MultiAddress::reset() {
  number.reset();
  short_code.reset();
  rfc2822.reset();
};

void MultiAddress::test() const{
  number.test();
  short_code.test();
  rfc2822.test();
}

SingleAddress::SingleAddress():type(NUMBER) {
}

SingleAddress::SingleAddress(const MultiAddress& multi_address) {
  if (!multi_address.number.isNotSet()) {
    address = multi_address.number;
    type = NUMBER;
  }
  if (!multi_address.short_code.isNotSet()) {
    address = multi_address.short_code;
    type = SHORT_CODE;
  }
  if (!multi_address.rfc2822.isNotSet()) {
    address = multi_address.rfc2822;
    type = RFC2822;
  }
}

void SingleAddress::serialize(DOMDocument* doc, DOMElement* parent,
                              const char* parent_tag_name) const {
  if (address.isNotSet() || !parent_tag_name) {
    return;
  }
  const char* address_type = 0;
  switch (type) {
    case NUMBER     : address_type = xml::NUMBER; break;
    case SHORT_CODE : address_type = xml::SHORT_CODE; break;
    case RFC2822    : address_type = xml::RFC2822; break;
    default         : return;
  }
  DOMElement* address_element = doc->createElement(XStr(parent_tag_name).unicodeForm());
  address.serialize(doc, address_element, address_type);
  parent->appendChild(address_element);
}

void SingleAddress::serialize(DOMDocument* doc, DOMElement* parent) const {
  if (address.isNotSet()) {
    return;
  }
  const char* address_type = 0;
  switch (type) {
    case NUMBER     : address_type = xml::NUMBER; break;
    case SHORT_CODE : address_type = xml::SHORT_CODE; break;
    case RFC2822    : address_type = xml::RFC2822; break;
    default         : return;
  }
  address.serialize(doc, parent, address_type);
}

void SingleAddress::test() const {
  address.test();
}


MmsMsg::MmsMsg():command_id(0), mms_version(DEFAULT_MMS_VERSION) {}

MmsMsg::MmsMsg(const string& _transaction_id, uint8_t _command_id)
       :command_id(_command_id), mms_version(DEFAULT_MMS_VERSION) {
  setTransactionId(_transaction_id);
}

void MmsMsg::setCommandId(uint8_t _command_id) {
  command_id = _command_id;
}

uint8_t MmsMsg::getCommandId() const {
  return command_id;
}

void MmsMsg::setTransactionId(const string& _id) {
  transaction_id = _id;
}

const string& MmsMsg::getTransactionId() const{
  return transaction_id;
}

void MmsMsg::setMmsVersion(const string& version) {
  mms_version = version;
}

const string& MmsMsg::getMmsVersion() const{
  return mms_version;
}

const string* MmsMsg::getInfoElement(const char* element_name) const {
  return mms_fields.GetPtr(element_name);
}

void MmsMsg::setInfoElement(const char* name, const string& value) {
  if (name && strcmp(name, "") != 0) {
    mms_fields.Insert(name, value);
  }
}

bool MmsMsg::isMM7Req() const {
  return (command_id >= MM7_SUBMIT && command_id <= MM7_READ_REPLY) 
          ? true : false;
}

bool MmsMsg::isMM4Req() const {
  return (command_id >= MM4_FORWARD && command_id <= MM4_READ_REPLY_REPORT)
          ? true : false;
}

void MmsMsg::test(){
  __trace2__("Command ID = %d", command_id);
  __trace2__("Transaction ID = \'%s\'", transaction_id.c_str());
  char* name = 0;
  std::string value;
  Hash<string> tmp = mms_fields;
  mms_fields.First();
  while(mms_fields.Next(name, value)) {
    __trace2__("Field Name = \'%s\' Field Value = \'%s\'", name, value.c_str());
  }
}

bool MmsMsg::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  XStr envelope_uri(xml::ENVELOPE_URI);
  XStr env_prefix(xml::ENV_PREFIX);
  DOMElement* header = doc->createElementNS(envelope_uri.unicodeForm(),
                                             XStr(xml::HEADER).unicodeForm());
  header->setPrefix(env_prefix.unicodeForm());
  root_element->appendChild(header);
  DOMElement* transact_id = doc->createElementNS(XStr(xml::MM7_URI).unicodeForm(),
                                               XStr(xml::TRANSACTION_ID).unicodeForm());
  transact_id->setPrefix(XStr(xml::MM7_PREFIX).unicodeForm());
  const std::string* ptr = mms_fields.GetPtr(xml::MUST_UNDERSTAND);
  if (ptr) {
    transact_id->setAttributeNS(envelope_uri.unicodeForm(),
                                XStr(xml::MUST_UNDERSTAND).unicodeForm(),
                                XStr(ptr->c_str()).unicodeForm());
  }
  ptr = mms_fields.GetPtr(xml::ENCODING_STYLE);
  if (ptr) {
    transact_id->setAttributeNS(envelope_uri.unicodeForm(),
                                XStr(xml::ENCODING_STYLE).unicodeForm(),
                                XStr(ptr->c_str()).unicodeForm());
  }
  ptr = mms_fields.GetPtr(xml::ACTOR);
  if (ptr) {
    transact_id->setAttributeNS(envelope_uri.unicodeForm(),
                                XStr(xml::ACTOR).unicodeForm(),
                                XStr(ptr->c_str()).unicodeForm());
  }
  header->appendChild(transact_id);
  if (transaction_id.empty()) {
    throw Exception("Serialization Error : Mandatory field \"TransactionID\" not set");
  }
  DOMText* transact_id_val = doc->createTextNode(XStr(transaction_id.c_str()).unicodeForm());
  transact_id->appendChild(transact_id_val);
  DOMElement* body = doc->createElementNS(envelope_uri.unicodeForm(),
                                          XStr(xml::BODY).unicodeForm());
  body->setPrefix(env_prefix.unicodeForm());
  root_element->appendChild(body);
  root_element = body;
  return true;
}

bool MmsMsg::serialize(string& serialized_msg) const {
  XMLCh temp_str[100];
  XMLString::transcode("LS", temp_str, 99);
  DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(temp_str);
  if (impl == NULL) {
    return false;
  } 
  DOMDocument* doc = impl->createDocument(XStr(xml::ENVELOPE_URI).unicodeForm(),
                                           XStr(xml::ENVELOPE).unicodeForm(), 0);
  if (!doc) {
    return false;
  }
  DOMElement* root_element = doc->getDocumentElement();
  root_element->setPrefix(XStr(xml::ENV_PREFIX).unicodeForm());

  try {
    getXMLDocument(doc, root_element);
  } catch (const Exception& e) {
    doc->release();
    throw;
  }
  doc->normalizeDocument();

#if XERCES_VERSION_MAJOR > 2
  DOMLSSerializer* serializer = ((DOMImplementationLS*)impl)->createLSSerializer();

  serializer->setNewLine(XStr("\n").unicodeForm());

  DOMConfiguration* dc = serializer->getDomConfig();
  dc->setParameter(XMLUni::fgDOMWRTFormatPrettyPrint, true);
  std::auto_ptr<DOMErrorHandler> handler(new DOMPrintErrorHandler());
  dc->setParameter(XMLUni::fgDOMErrorHandler, handler.get());
  XMLCh *str = serializer->writeToString(doc);
#else
  DOMWriter* serializer = ((DOMImplementationLS*)impl)->createDOMWriter();

  serializer->setNewLine(XStr("\n").unicodeForm());

  if (serializer->canSetFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true)) {
    serializer->setFeature(XMLUni::fgDOMWRTFormatPrettyPrint, true);
  }

  std::auto_ptr<DOMErrorHandler> handler(new DOMPrintErrorHandler());
  serializer->setErrorHandler(handler.get());

  XMLCh *str = serializer->writeToString(*doc);
#endif

  if (str != NULL) {
    Convertor::UCS2ToUTF8(str, XMLString::stringLen(str), serialized_msg);
    serialized_msg.replace(serialized_msg.find(xml::UTF_16), 6, xml::UTF_8);

    XMLString::release(&str);
    doc->release();
    serializer->release();
    return true;
  } else {
    XMLString::release(&str);
    doc->release();
    serializer->release();
    return false;
  }
}

MmsMsg::~MmsMsg() {}

MM7GenericVASPReq::MM7GenericVASPReq(const string& _transaction_id, uint8_t _command_id)
                                     :MmsMsg(_transaction_id, _command_id) {
}

void MM7GenericVASPReq::setSenderAddress(const SingleAddress& address) {
  sender_address = address;
}

const SingleAddress& MM7GenericVASPReq::getSenderAddress() const {
  return sender_address;
}

const string* MM7GenericVASPReq::getEndpointId() const {
  return mms_fields.GetPtr(xml::VASP_ID);
}

void MM7GenericVASPReq::setEndpointId(const string& vasp_id) {
  setInfoElement(xml::VASP_ID, vasp_id);
}

bool MM7GenericVASPReq::getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element,
                                              const char* command_name) const {
  MmsMsg::getXMLDocument(doc, root_element);
  DOMElement* req_element = doc->createElementNS(XStr(xml::MM7_URI).unicodeForm(),
                                                 XStr(command_name).unicodeForm());
  root_element->appendChild(req_element);
  root_element = req_element;
  const string* ptr = mms_fields.GetPtr(xml::MM7_VERSION);      						 
  string version = getMmsVersion();
  if (!ptr && version.empty()) {
    throw Exception("Serialization Error : Mandatory Field \"%s\" Not Set",
                     xml::MM7_VERSION);
  }
  if (version.empty() && ptr) {
    version = *ptr;
  }
  xml::addTextNode(doc, root_element, xml::MM7_VERSION, version);
  DOMElement* sender_id = doc->createElement(XStr(xml::SENDER_ID).unicodeForm());
  root_element->appendChild(sender_id);
  ptr = mms_fields.GetPtr(xml::VASP_ID);
  if (ptr) {
    xml::addTextNode(doc, sender_id, xml::VASP_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::VAS_ID);
  if (ptr) {
    xml::addTextNode(doc, sender_id, xml::VAS_ID, *ptr);
  }
  sender_address.serialize(doc, sender_id, xml::SENDER_ADDRESS);
  return true;
}

void MM7GenericVASPReq::test(){
  MmsMsg::test();
  __trace__("Sender Address");
  sender_address.test();
}

MM7GenericVASPReq::~MM7GenericVASPReq() {
}

MM7GenericRSReq::MM7GenericRSReq(const string& _transaction_id, uint8_t _command_id)
                                     :MmsMsg(_transaction_id, _command_id) {
}

void MM7GenericRSReq::setSenderAddress(const SingleAddress& address) {
  sender_address = address;
}

const SingleAddress& MM7GenericRSReq::getSenderAddress() const {
  return sender_address;
}

const string* MM7GenericRSReq::getEndpointId() const {
  return mms_fields.GetPtr(xml::MMS_RS_ID);
}

void MM7GenericRSReq::setEndpointId(const string& rs_id) {
  setInfoElement(xml::MMS_RS_ID, rs_id);
}

bool MM7GenericRSReq::getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element,
                                            const char* command_name) const {
  MmsMsg::getXMLDocument(doc, root_element);
  DOMElement* req_element = doc->createElementNS(XStr(xml::MM7_URI).unicodeForm(),
                                                 XStr(command_name).unicodeForm());
  root_element->appendChild(req_element);
  root_element = req_element;
  const string* ptr = mms_fields.GetPtr(xml::MM7_VERSION);      						 
  string version = getMmsVersion();
  if (!ptr && version.empty()) {
    throw Exception("Serialization Error : Mandatory Field \"%s\" Not Set", xml::MM7_VERSION);
  }
  if (version.empty() && ptr) {
    version = *ptr;
  }
  xml::addTextNode(doc, root_element, xml::MM7_VERSION, version);
  ptr = mms_fields.GetPtr(xml::MMS_RS_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::MMS_RS_ID, *ptr);
  }
  return true;
}

void MM7GenericRSReq::test() {
  MmsMsg::test();
  __trace__("Sender Address");
  sender_address.test();
}

MM7GenericRSReq::~MM7GenericRSReq() {
}

MM7Submit::MM7Submit(const string& _transaction_id)
                   :MM7GenericVASPReq(_transaction_id, MM7_SUBMIT) {
}

MmsMsg* MM7Submit::getResponse() const {
  MmsMsg* _mms_msg = new MM7SubmitResp(transaction_id);
  _mms_msg->setInfoElement(xml::MESSAGE_ID, "testMessageId_12345");
  return _mms_msg;
}

void MM7Submit::setRecipientAddress(const MultiAddress& address, uint8_t recipient_type) {
  switch(recipient_type) {
    case TO  : to_address.push_back(address); break; 
    case CC  : cc_address.push_back(address); break; 
    case BCC : bcc_address.push_back(address); break; 
  }
}

void MM7Submit::test() {
  MM7GenericVASPReq::test();
  __trace__("TO Address");
  for (int i = 0; i < to_address.size(); ++i) {
    to_address[i].test();
  }
  __trace__("CC Address");
  for (int i = 0; i < cc_address.size(); ++i) {
    cc_address[i].test();
  }
  __trace__("BCC Address");
  for (int i = 0; i < bcc_address.size(); ++i) {
    bcc_address[i].test();
  }
}

bool MM7Submit::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MM7GenericVASPReq::getGenericXMLDocument(doc, root_element, mm7_command_name::SUBMIT);
  if (to_address.empty() && cc_address.empty() && bcc_address.empty()) {
    throw Exception("Serialization Error : Recipient Address Not Set");
  }
  DOMElement* recipients = doc->createElement(XStr(xml::RECIPIENTS).unicodeForm());
  typedef std::vector<MultiAddress>::const_iterator CI;
  for (CI p = to_address.begin(); p != to_address.end(); ++p) {
    (*p).serialize(doc, recipients, xml::TO);
  }
  for (CI p = cc_address.begin(); p != cc_address.end(); ++p) {
    (*p).serialize(doc, recipients, xml::CC);
  }
  for (CI p = bcc_address.begin(); p != bcc_address.end(); ++p) {
    (*p).serialize(doc, recipients, xml::BCC);
  }
  root_element->appendChild(recipients);
  const string* ptr = mms_fields.GetPtr(xml::SERVICE_CODE);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::SERVICE_CODE, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::LINKED_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::LINKED_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::MESSAGE_CLASS);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::MESSAGE_CLASS, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::TIME_STAMP);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::TIME_STAMP, *ptr);
  }
  const string* size = mms_fields.GetPtr(xml::REPLY_CHARGING_SIZE);
  const string* deadline = mms_fields.GetPtr(xml::REPLY_DEADLINE);
  if (size || deadline) {
    DOMElement* charging = doc->createElement(XStr(xml::REPLY_CHARGING).unicodeForm());
    if (size) {
      charging->setAttribute(XStr(xml::REPLY_CHARGING_SIZE).unicodeForm(), XStr(size->c_str()).unicodeForm());
    }
    if (deadline) {
      charging->setAttribute(XStr(xml::REPLY_DEADLINE).unicodeForm(), XStr(deadline->c_str()).unicodeForm());
    }
    root_element->appendChild(charging);
  }
  ptr = mms_fields.GetPtr(xml::EARLIEST_DELIVERY_TIME);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::EARLIEST_DELIVERY_TIME, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::EXPIRY_DATE);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::EXPIRY_DATE, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::DELIVERY_REPORT);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::DELIVERY_REPORT, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::READ_REPLY);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::READ_REPLY, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::PRIORITY);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::PRIORITY, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::SUBJECT);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::SUBJECT, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::CHARGED_PARTY);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::CHARGED_PARTY, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::CHARGED_PARTY_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::CHARGED_PARTY_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::DISTRIBUTION_INDICATOR);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::DISTRIBUTION_INDICATOR, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::DELIVERY_CONDITION);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::DELIVERY_CONDITION, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::REPLY_APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::REPLY_APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::AUX_APPLIC_INFO);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::AUX_APPLIC_INFO, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::CONTENT_CLASS);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::CONTENT_CLASS, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::DRM_CONTENT);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::DRM_CONTENT, *ptr);
  }
  const string* href = mms_fields.GetPtr(xml::HREF);
  const string* allow = mms_fields.GetPtr(xml::ALLOW_ADAPTATIONS);
  if (href || allow) {
    DOMElement* content = doc->createElement(XStr(xml::CONTENT).unicodeForm());
    if (href) {
      content->setAttribute(XStr(xml::HREF).unicodeForm(), XStr(href->c_str()).unicodeForm());
    }
    if (allow) {
      content->setAttribute(XStr(xml::ALLOW_ADAPTATIONS).unicodeForm(), XStr(allow->c_str()).unicodeForm());
    }
    root_element->appendChild(content);
  }
  return true;
}

MM7Submit::~MM7Submit() {}

MM7Deliver::MM7Deliver(const string& _transaction_id)
            :MM7GenericRSReq(_transaction_id, MM7_DELIVER), sequence_number(0) {
}

MmsMsg* MM7Deliver::getResponse() const {
  MmsMsg* _mms_msg = new MM7DeliverResp(transaction_id);
  _mms_msg->setInfoElement(xml::SERVICE_CODE, "testServiceCode_12345");
  return _mms_msg;
}

bool MM7Deliver::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MM7GenericRSReq::getGenericXMLDocument(doc, root_element, mm7_command_name::DELIVER);
  if (to_address.empty() && cc_address.empty() && bcc_address.empty()) {
    throw Exception("Serialization Error : Recipient Address Not Set");
  }
  const string* ptr = mms_fields.GetPtr(xml::VASP_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::VASP_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::VAS_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::VAS_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::LINKED_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::LINKED_ID, *ptr);
  }
  MM7GenericRSReq::getSenderAddress().serialize(doc, root_element, xml::SENDER);
  DOMElement* recipients = doc->createElement(XStr(xml::RECIPIENTS).unicodeForm());
  typedef std::vector<MultiAddress>::const_iterator CI;
  for (CI p = to_address.begin(); p != to_address.end(); ++p) {
    (*p).serialize(doc, recipients, xml::TO);
  }
  for (CI p = cc_address.begin(); p != cc_address.end(); ++p) {
    (*p).serialize(doc, recipients, xml::CC);
  }
  for (CI p = bcc_address.begin(); p != bcc_address.end(); ++p) {
    (*p).serialize(doc, recipients, xml::BCC);
  }
  root_element->appendChild(recipients);
  if (!previously_sent.empty()) {
    DOMElement* sent_by = doc->createElement(XStr(xml::PREVIOUSLY_SENT_BY).unicodeForm());
    DOMElement* sent_date = doc->createElement(XStr(xml::PREVIOUSLY_SENT_DATE).unicodeForm());
    typedef std::map<size_t, PreviouslySent>::const_iterator CI;
    const uint8_t buf_size = 6;
    char sn_buf[buf_size];
    for (CI p = previously_sent.begin(); p != previously_sent.end(); ++p) {
      snprintf(sn_buf, buf_size, "%d", p->first);
      XStr number(sn_buf);
      XStr sequence(xml::SEQUENCE);
      if (!p->second.sent_by.address.isNotSet()) {
        DOMElement* user_agent = doc->createElement(XStr(xml::USER_AGENT).unicodeForm());
	user_agent->setAttribute(sequence.unicodeForm(), number.unicodeForm());
	p->second.sent_by.serialize(doc, user_agent);
	sent_by->appendChild(user_agent);
      }
      if (!p->second.sent_date.empty()) {
        DOMElement* date_time = doc->createElement(XStr(xml::DATE_TIME).unicodeForm());
	date_time->setAttribute(sequence.unicodeForm(), number.unicodeForm());
	DOMText* date_value = doc->createTextNode(XStr(p->second.sent_date.c_str()).unicodeForm());
	date_time->appendChild(date_value);
	sent_date->appendChild(date_time);
      }
    }
    root_element->appendChild(sent_by);
    root_element->appendChild(sent_date);
  }
  ptr = mms_fields.GetPtr(xml::SENDER_SPI);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::SENDER_SPI, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::RECIPIENT_SPI);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::RECIPIENT_SPI, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::TIME_STAMP);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::TIME_STAMP, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::REPLAY_CHARGING_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::REPLAY_CHARGING_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::PRIORITY);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::PRIORITY, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::SUBJECT);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::SUBJECT, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::REPLY_APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::REPLY_APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::AUX_APPLIC_INFO);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::AUX_APPLIC_INFO, *ptr);
  }
  const string* ua_prof = mms_fields.GetPtr(xml::UA_PROF);
  const string* ua_time_stamp = mms_fields.GetPtr(xml::UA_TIME_STAMP);
  if (ua_prof || ua_time_stamp) {
    DOMElement* ua = doc->createElement(XStr(xml::UA_CAPABILITIES).unicodeForm());
    if (ua_prof) {
      ua->setAttribute(XStr(xml::UA_PROF).unicodeForm(), XStr(ua_prof->c_str()).unicodeForm());
    }
    if (ua_time_stamp) {
      ua->setAttribute(XStr(xml::TIME_STAMP).unicodeForm(), XStr(ua_time_stamp->c_str()).unicodeForm());
    }
    root_element->appendChild(ua);
  }
  const string* href = mms_fields.GetPtr(xml::HREF);
  const string* allow = mms_fields.GetPtr(xml::ALLOW_ADAPTATIONS);
  if (href || allow) {
    DOMElement* content = doc->createElement(XStr(xml::CONTENT).unicodeForm());
    if (href) {
      content->setAttribute(XStr(xml::HREF).unicodeForm(), XStr(href->c_str()).unicodeForm());
    }
    if (allow) {
      content->setAttribute(XStr(xml::ALLOW_ADAPTATIONS).unicodeForm(), XStr(allow->c_str()).unicodeForm());
    }
    root_element->appendChild(content);
  }
  return true; 
}
void MM7Deliver::setRecipientAddress(const MultiAddress& address, uint8_t recipient_type) {
  switch(recipient_type) {
    case TO  : to_address.push_back(address); break; 
    case CC  : cc_address.push_back(address); break; 
    case BCC : bcc_address.push_back(address); break; 
  }
}

void MM7Deliver::test() {
  MM7GenericRSReq::test();
  __trace__("TO Address");
  for (int i = 0; i < to_address.size(); ++i) {
    to_address[i].test();
  }
  __trace__("CC Address");
  for (int i = 0; i < cc_address.size(); ++i) {
    cc_address[i].test();
  }
  __trace__("BCC Address");
  for (int i = 0; i < bcc_address.size(); ++i) {
    bcc_address[i].test();
  }
  typedef std::map<size_t, PreviouslySent>::iterator I;
  for (I p = previously_sent.begin(); p != previously_sent.end(); ++p) {
    __trace2__("%d Date : %s Address :", p->first, p->second.sent_date.c_str());
    p->second.sent_by.test();
  } 
}
void MM7Deliver::setSequenceNumber(size_t number) {
  sequence_number = number;
}

void MM7Deliver::setPreviouslySentBy(const SingleAddress& address) {
  previously_sent[sequence_number].sent_by = address;
}

void MM7Deliver::setPreviouslySentDate(const string& date) {
  previously_sent[sequence_number].sent_date = date;
}

MM7Deliver::~MM7Deliver() {}

MM7Cancel::MM7Cancel(const string& _transaction_id)
          :MM7GenericVASPReq(_transaction_id, MM7_CANCEL) {};

MmsMsg* MM7Cancel::getResponse() const {
  return new MM7CancelResp(transaction_id);
}

bool MM7Cancel::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MM7GenericVASPReq::getGenericXMLDocument(doc, root_element, mm7_command_name::CANCEL);
  const string* ptr = mms_fields.GetPtr(xml::MESSAGE_ID);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_Cancel_REQ : Message ID Not Set");
  }
  xml::addTextNode(doc, root_element, xml::MESSAGE_ID, *ptr);
  ptr = mms_fields.GetPtr(xml::APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::REPLY_APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::REPLY_APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::AUX_APPLIC_INFO);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::AUX_APPLIC_INFO, *ptr);
  }
  return true;
}

MM7Cancel::~MM7Cancel() {};

MM7ExtendedCancel::MM7ExtendedCancel(const string& _transaction_id)
                  :MM7GenericVASPReq(_transaction_id, MM7_EXTENDED_CANCEL) {};

MmsMsg* MM7ExtendedCancel::getResponse() const {
  return new MM7ExtendedCancelResp(transaction_id);
}

bool MM7ExtendedCancel::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MM7GenericVASPReq::getGenericXMLDocument(doc, root_element, mm7_command_name::EXTENDED_CANCEL);
  const string* ptr = mms_fields.GetPtr(xml::CANCEL_ID);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_ExtendedCancel_REQ : Cancel ID Not Set");
  }
  xml::addTextNode(doc, root_element, xml::CANCEL_ID, *ptr);
  return true;
}

MM7ExtendedCancel::~MM7ExtendedCancel() {};

MM7Replace::MM7Replace(const string& _transaction_id)
            :MM7GenericVASPReq(_transaction_id, MM7_REPLACE) {};

MmsMsg* MM7Replace::getResponse() const {
  return new MM7ReplaceResp(transaction_id);
}

bool MM7Replace::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MM7GenericVASPReq::getGenericXMLDocument(doc, root_element, mm7_command_name::REPLACE);
  const string* ptr = mms_fields.GetPtr(xml::MESSAGE_ID);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_Replace_REQ : Message ID Not Set");
  }
  xml::addTextNode(doc, root_element, xml::MESSAGE_ID, *ptr);

  ptr = mms_fields.GetPtr(xml::SERVICE_CODE);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::SERVICE_CODE, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::TIME_STAMP);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::TIME_STAMP, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::READ_REPLY);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::READ_REPLY, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::EARLIEST_DELIVERY_TIME);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::EARLIEST_DELIVERY_TIME, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::DISTRIBUTION_INDICATOR);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::DISTRIBUTION_INDICATOR, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::CONTENT_CLASS);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::CONTENT_CLASS, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::DRM_CONTENT);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::DRM_CONTENT, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::REPLY_APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::REPLY_APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::AUX_APPLIC_INFO);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::AUX_APPLIC_INFO, *ptr);
  }
  const string* href = mms_fields.GetPtr(xml::HREF);
  const string* allow = mms_fields.GetPtr(xml::ALLOW_ADAPTATIONS);
  if (href || allow) {
    DOMElement* content = doc->createElement(XStr(xml::CONTENT).unicodeForm());
    if (href) {
      content->setAttribute(XStr(xml::HREF).unicodeForm(), XStr(href->c_str()).unicodeForm());
    }
    if (allow) {
      content->setAttribute(XStr(xml::ALLOW_ADAPTATIONS).unicodeForm(), XStr(allow->c_str()).unicodeForm());
    }
    root_element->appendChild(content);
  }
  return true;
}

MM7Replace::~MM7Replace() {};

MM7ExtendedReplace::MM7ExtendedReplace(const string& _transaction_id)
                   :MmsMsg(_transaction_id, MM7_EXTENDED_REPLACE) {};

MmsMsg* MM7ExtendedReplace::getResponse() const {
  MmsMsg* _mms_msg = new MM7ExtendedReplaceResp(transaction_id);
  _mms_msg->setInfoElement(xml::MESSAGE_ID, "testMessageId_12345");
  return _mms_msg;
}

bool MM7ExtendedReplace::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MmsMsg::getXMLDocument(doc, root_element);
  DOMElement* req_element = doc->createElementNS(XStr(xml::MM7_URI).unicodeForm(),
                                                 XStr(mm7_command_name::EXTENDED_REPLACE).unicodeForm());
  root_element->appendChild(req_element);
  root_element = req_element;
  const string* ptr = mms_fields.GetPtr(xml::MM7_VERSION);      						 
  string version = getMmsVersion();
  if (!ptr && version.empty()) {
    throw Exception("Serialization Error : Mandatory Field \"%s\" Not Set", xml::MM7_VERSION);
  }
  if (version.empty() && ptr) {
    version = *ptr;
  }
  xml::addTextNode(doc, root_element, xml::MM7_VERSION, version);
  ptr = mms_fields.GetPtr(xml::VASP_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::VASP_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::VAS_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::VAS_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::SERVICE_CODE);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::SERVICE_CODE, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::REPLACE_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::REPLACE_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::TIME_STAMP);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::TIME_STAMP, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::EARLIEST_DELIVERY_TIME);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::EARLIEST_DELIVERY_TIME, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::EXPIRY_DATE);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::EXPIRY_DATE, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::READ_REPLY);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::READ_REPLY, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::DELIVERY_REPORT);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::DELIVERY_REPORT, *ptr);
  }
  const string* href = mms_fields.GetPtr(xml::HREF);
  const string* allow = mms_fields.GetPtr(xml::ALLOW_ADAPTATIONS);
  if (href || allow) {
    DOMElement* content = doc->createElement(XStr(xml::CONTENT).unicodeForm());
    if (href) {
      content->setAttribute(XStr(xml::HREF).unicodeForm(), XStr(href->c_str()).unicodeForm());
    }
    if (allow) {
      content->setAttribute(XStr(xml::ALLOW_ADAPTATIONS).unicodeForm(), XStr(allow->c_str()).unicodeForm());
    }
    root_element->appendChild(content);
  }
  return true;
}

MM7ExtendedReplace::~MM7ExtendedReplace() {};

MM7DeliveryReport::MM7DeliveryReport(const string& _transaction_id)
                  :MM7GenericRSReq(_transaction_id, MM7_DELIVERY_REPORT) {}

MmsMsg* MM7DeliveryReport::getResponse() const {
  return new MM7DeliveryReportResp(transaction_id);
}

void MM7DeliveryReport::setRecipientAddress(const MultiAddress& address,
                                            uint8_t recipient_type) {
  recipient_address = address;
}

bool MM7DeliveryReport::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MM7GenericRSReq::getGenericXMLDocument(doc, root_element,
                                         mm7_command_name::DELIVERY_REPORT);
  const string* ptr = mms_fields.GetPtr(xml::MESSAGE_ID);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_DeliveryReport_REQ : MessageID Not Set");
  }
  xml::addTextNode(doc, root_element, xml::MESSAGE_ID, *ptr);
  if (recipient_address.address.isNotSet()) {
    throw Exception("Serialization Error of MM7_DeliveryReport_REQ : Recipient Address Not Set");
  }
  recipient_address.serialize(doc, root_element, xml::RECIPIENT);
  if (sender_address.address.isNotSet()) {
    throw Exception("Serialization Error of MM7_DeliveryReport_REQ : Sender Address Not Set");
  }
  sender_address.serialize(doc, root_element, xml::SENDER);
  ptr = mms_fields.GetPtr(xml::DATE);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_DeliveryReport_REQ : Date Not Set");
  }
  xml::addTextNode(doc, root_element, xml::DATE, *ptr);
  ptr = mms_fields.GetPtr(xml::MM_STATUS);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_DeliveryReport_REQ : MMStatus Not Set");
  }
  xml::addTextNode(doc, root_element, xml::MM_STATUS, *ptr);
  ptr = mms_fields.GetPtr(xml::MM_STATUS_EXTENSION);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::MM_STATUS_EXTENSION, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::STATUS_TEXT);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::STATUS_TEXT, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::REPLY_APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::REPLY_APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::AUX_APPLIC_INFO);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::AUX_APPLIC_INFO, *ptr);
  }
  const string* ua_prof = mms_fields.GetPtr(xml::UA_PROF);
  const string* ua_time_stamp = mms_fields.GetPtr(xml::UA_TIME_STAMP);
  if (ua_prof || ua_time_stamp) {
    DOMElement* ua = doc->createElement(XStr(xml::UA_CAPABILITIES).unicodeForm());
    if (ua_prof) {
      ua->setAttribute(XStr(xml::UA_PROF).unicodeForm(), XStr(ua_prof->c_str()).unicodeForm());
    }
    if (ua_time_stamp) {
      ua->setAttribute(XStr(xml::TIME_STAMP).unicodeForm(), XStr(ua_time_stamp->c_str()).unicodeForm());
    }
    root_element->appendChild(ua);
  }
  return true;
}

void MM7DeliveryReport::test() {
  MM7GenericRSReq::test();
  __trace__("Recipient Address");
  recipient_address.test();
}

MM7DeliveryReport::~MM7DeliveryReport() {}

MM7ReadReply::MM7ReadReply(const string& _transaction_id)
             :MM7GenericRSReq(_transaction_id, MM7_READ_REPLY) {}

MmsMsg* MM7ReadReply::getResponse() const {
  return new MM7ReadReplyResp(transaction_id);
}

void MM7ReadReply::setRecipientAddress(const MultiAddress& address,
                                       uint8_t recipient_type) {
  recipient_address = address;
}

bool MM7ReadReply::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MM7GenericRSReq::getGenericXMLDocument(doc, root_element, mm7_command_name::READ_REPLY);
  const string* ptr = mms_fields.GetPtr(xml::MESSAGE_ID);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_ReadReply_REQ : Message ID Not Set");
  }
  xml::addTextNode(doc, root_element, xml::MESSAGE_ID, *ptr);
  if (recipient_address.address.isNotSet()) {
    throw Exception("Serialization Error of MM7_ReadReply_REQ : Recipient Address Not Set");
  }
  recipient_address.serialize(doc, root_element, xml::RECIPIENT);
  if (sender_address.address.isNotSet()) {
    throw Exception("Serialization Error of MM7_ReadReply_REQ : Sender Address Not Set");
  }
  sender_address.serialize(doc, root_element, xml::SENDER);
  ptr = mms_fields.GetPtr(xml::TIME_STAMP);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_ReadReply_REQ : TimeStamp Not Set");
  }
  xml::addTextNode(doc, root_element, xml::TIME_STAMP, *ptr);
  ptr = mms_fields.GetPtr(xml::MM_STATUS);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_ReadReply_REQ : MMStatus Not Set");
  }
  xml::addTextNode(doc, root_element, xml::MM_STATUS, *ptr);
  ptr = mms_fields.GetPtr(xml::STATUS_TEXT);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::STATUS_TEXT, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::REPLY_APPLIC_ID);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::REPLY_APPLIC_ID, *ptr);
  }
  ptr = mms_fields.GetPtr(xml::AUX_APPLIC_INFO);
  if (ptr) {
    xml::addTextNode(doc, root_element, xml::AUX_APPLIC_INFO, *ptr);
  }
  return true;
}

void MM7ReadReply::test(){
  MM7GenericRSReq::test();
  __trace__("Recipient Address");
  recipient_address.test();
}

MM7ReadReply::~MM7ReadReply() {}

bool GenericResponse::getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element, const char* command_name) const {
  uint8_t cmd_id = getCommandId();
  if (cmd_id != MM7_VASP_ERROR_RESP && cmd_id != MM7_RS_ERROR_RESP) {
    MmsMsg::getXMLDocument(doc, root_element);
  }
  DOMElement* resp_element = doc->createElementNS(XStr(xml::MM7_URI).unicodeForm(),
                                                 XStr(command_name).unicodeForm());
  root_element->appendChild(resp_element);
  root_element = resp_element;
  const string* ptr = mms_fields.GetPtr(xml::MM7_VERSION);      						 
  string version = getMmsVersion();
  if (!ptr && version.empty()) {
    throw Exception("Serialization Error : Mandatory Field \"%s\" Not Set", xml::MM7_VERSION);
  }
  if (version.empty() && ptr) {
    version = *ptr;
  }
  xml::addTextNode(doc, root_element, xml::MM7_VERSION, version);
  DOMElement* status = doc->createElement(XStr(xml::STATUS).unicodeForm());
  root_element->appendChild(status);
  ptr = mms_fields.GetPtr(xml::STATUS_CODE);      						 
  if (!ptr) {
    throw Exception("Serialization Error of MM7_Response : Mandatory Field \"%s\" Not Set", xml::STATUS_CODE);
  }
  xml::addTextNode(doc, status, xml::STATUS_CODE, *ptr);
  ptr = mms_fields.GetPtr(xml::STATUS_TEXT);      						 
  if (!ptr) {
    throw Exception("Serialization Error of MM7_Response : Mandatory Field \"%s\" Not Set", xml::STATUS_TEXT);
  }
  xml::addTextNode(doc, status, xml::STATUS_TEXT, *ptr);
  root_element->appendChild(status);
  return true;
}
bool GenericResponse::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  uint8_t cmd_id = getCommandId();
  if (cmd_id != MM7_VASP_ERROR_RESP && cmd_id != MM7_RS_ERROR_RESP) {
    return true;
  }
  MmsMsg::getXMLDocument(doc, root_element);
  DOMElement* fault = doc->createElementNS(XStr(xml::ENVELOPE_URI).unicodeForm(), XStr(xml::FAULT).unicodeForm());
  fault->setPrefix(XStr(xml::ENV_PREFIX).unicodeForm());
  root_element->appendChild(fault);
  const string* ptr = mms_fields.GetPtr(xml::FAULT_CODE);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_Response : Mandatory Field \"%s\" Not Set", xml::FAULT_CODE);
  }
  xml::addTextNode(doc, fault, xml::FAULT_CODE, *ptr);
  ptr = mms_fields.GetPtr(xml::FAULT_STRING);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_Response : Mandatory Field \"%s\" Not Set", xml::FAULT_STRING);
  }
  xml::addTextNode(doc, fault, xml::FAULT_STRING, *ptr);
  DOMElement* detail = doc->createElement(XStr(xml::DETAIL).unicodeForm());
  fault->appendChild(detail);
  if (MM7_VASP_ERROR_RESP == cmd_id) {
    return GenericResponse::getGenericXMLDocument(doc, detail, mm7_command_name::VASP_ERROR_RESP);
  }
  if (MM7_RS_ERROR_RESP == cmd_id) {
    return GenericResponse::getGenericXMLDocument(doc, detail, mm7_command_name::RS_ERROR_RESP);
  }

  return false;
}
bool MM7SubmitResp::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  GenericResponse::getGenericXMLDocument(doc, root_element, mm7_command_name::SUBMIT_RESP);
  const string* ptr = mms_fields.GetPtr(xml::MESSAGE_ID);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_Submit_RESP : Mandatory Field \"%s\" Not Set", xml::MESSAGE_ID);
  }
  xml::addTextNode(doc, root_element, xml::MESSAGE_ID, *ptr);
  return true; 
}
bool MM7DeliverResp::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  GenericResponse::getGenericXMLDocument(doc, root_element, mm7_command_name::DELIVER_RESP);
  const string* ptr = mms_fields.GetPtr(xml::SERVICE_CODE);
  if (!ptr) {
    throw Exception("Serialization Error of MM7_Deliver_RESP : Mandatory Field \"%s\" Not Set", xml::SERVICE_CODE);
  }
  xml::addTextNode(doc, root_element, xml::SERVICE_CODE, *ptr);
  return true; 
}
bool MM7ExtendedCancelResp::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MmsMsg::getXMLDocument(doc, root_element);
  DOMElement* resp_element = doc->createElementNS(XStr(xml::MM7_URI).unicodeForm(),
                                               XStr(mm7_command_name::EXTENDED_CANCEL_RESP).unicodeForm());
  root_element->appendChild(resp_element);
  root_element = resp_element;
  const string* ptr = mms_fields.GetPtr(xml::MM7_VERSION);
  string version = getMmsVersion();
  if (!ptr && version.empty()) {
    throw Exception("Serialization Error of MM7_ExtendedÑancel_RESP : Mandatory Field \"%s\" Not Set", xml::MM7_VERSION);
  }
  if (version.empty() && ptr) {
    version = *ptr;
  }
  xml::addTextNode(doc, root_element, xml::MM7_VERSION, version);
  DOMElement* status = doc->createElement(XStr(xml::STATUS).unicodeForm());
  root_element->appendChild(status);
  ptr = mms_fields.GetPtr(xml::STATUS_CODE);      						 
  if (!ptr) {
    throw Exception("Serialization Error of MM7_ExtendedÑancel_RESP : Mandatory Field \"%s\" Not Set", xml::STATUS_CODE);
  }
  xml::addTextNode(doc, status, xml::STATUS_CODE, *ptr);
  root_element->appendChild(status);
  return true;
}
bool MM7ExtendedReplaceResp::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  MmsMsg::getXMLDocument(doc, root_element);
  DOMElement* resp_element = doc->createElementNS(XStr(xml::MM7_URI).unicodeForm(),
                                               XStr(mm7_command_name::EXTENDED_REPLACE_RESP).unicodeForm());
  root_element->appendChild(resp_element);
  root_element = resp_element;
  const string* ptr = mms_fields.GetPtr(xml::MM7_VERSION);
  string version = getMmsVersion();
  if (!ptr && version.empty()) {
    throw Exception("Serialization Error of MM7_ExtendedReplce_RESP : Mandatory Field \"%s\" Not Set", xml::MM7_VERSION);
  }
  if (version.empty() && ptr) {
    version = *ptr;
  }
  xml::addTextNode(doc, root_element, xml::MM7_VERSION, version);
  ptr = mms_fields.GetPtr(xml::MESSAGE_ID);      						 
  if (!ptr) {
    throw Exception("Serialization Error of MM7_ExtendedReplce_RESP : Mandatory Field \"%s\" Not Set", xml::MESSAGE_ID);
  }
  xml::addTextNode(doc, root_element, xml::MESSAGE_ID, *ptr);
  DOMElement* status = doc->createElement(XStr(xml::STATUS).unicodeForm());
  root_element->appendChild(status);
  ptr = mms_fields.GetPtr(xml::STATUS_CODE);      						 
  if (!ptr) {
    throw Exception("Serialization Error of MM7_ExtendedReplce_RESP : Mandatory Field \"%s\" Not Set", xml::STATUS_CODE);
  }
  xml::addTextNode(doc, status, xml::STATUS_CODE, *ptr);
  //ptr = mms_fields.GetPtr(xml::STATUS_TEXT);      						 
  //if (!ptr) {
    //throw Exception("Serialization Error of MM7_ExtendedReplce_RESP : Mandatory Field \"%s\" Not Set", xml::STATUS_TEXT);
    //__trace2__("Serialization Error of MM7_ExtendedReplce_RESP : Mandatory Field \"%s\" Not Set", xml::STATUS_TEXT);
    //return false;
  //}
  //xml::addTextNode(doc, status, xml::STATUS_TEXT, *ptr);
  root_element->appendChild(status);
  return true;
}
}//mms
}//transport
}//scag
