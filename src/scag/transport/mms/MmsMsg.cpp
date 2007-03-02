#include "MmsMsg.h"

namespace scag{
namespace transport{
namespace mms {

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

void Address::setValue(string _value) {
  value = _value;
}

string Address::getValue() const {
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

void Address::test() {
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

MmsMsg::MmsMsg():command_id(0) {}

MmsMsg::MmsMsg(string _transaction_id, uint8_t _command_id):command_id(_command_id) {
  setTransactionId(_transaction_id);
  //setCommandId(_command_id);
}

void MmsMsg::setCommandId(const char* command_name) {
  if (std::strcmp(command_name, mm7_command_name::SUBMIT) == 0) {
    command_id = MM7_SUBMIT;
  }
  if (std::strcmp(command_name, mm7_command_name::SUBMIT_RESP) == 0) {
    command_id = MM7_SUBMIT_RESP;
  }
  if (std::strcmp(command_name, mm7_command_name::DELIVER) == 0) {
    command_id = MM7_DELIVER;
  }
  if (std::strcmp(command_name, mm7_command_name::DELIVER_RESP) == 0) {
    command_id = MM7_DELIVER_RESP;
  }
  if (std::strcmp(command_name, mm7_command_name::REPLACE) == 0) {
    command_id = MM7_REPLACE;
  }
  if (std::strcmp(command_name, mm7_command_name::REPLACE_RESP) == 0) {
    command_id = MM7_REPLACE_RESP;
  }
  if (std::strcmp(command_name, mm7_command_name::CANCEL) == 0) {
    command_id = MM7_CANCEL;
  }
  if (std::strcmp(command_name, mm7_command_name::CANCEL_RESP) == 0) {
    command_id = MM7_CANCEL_RESP;
  }
  if (std::strcmp(command_name, mm7_command_name::EXTENDED_REPLACE) == 0) {
    command_id = MM7_EXTENDED_REPLACE;
  }
  if (std::strcmp(command_name, mm7_command_name::EXTENDED_REPLACE_RESP) == 0) {
    command_id = MM7_EXTENDED_REPLACE_RESP;
  }
  if (std::strcmp(command_name, mm7_command_name::EXTENDED_CANCEL) == 0) {
    command_id = MM7_EXTENDED_CANCEL;
  }
  if (std::strcmp(command_name, mm7_command_name::EXTENDED_CANCEL_RESP) == 0) {
    command_id = MM7_EXTENDED_CANCEL_RESP;
  }
  if (std::strcmp(command_name, mm7_command_name::DELIVERY_REPORT) == 0) {
    command_id = MM7_DELIVERY_REPORT;
  }
  if (std::strcmp(command_name, mm7_command_name::DELIVERY_REPORT_RESP) == 0) {
    command_id = MM7_DELIVERY_REPORT_RESP;
  }
  if (std::strcmp(command_name, mm7_command_name::READ_REPLY) == 0) {
    command_id = MM7_READ_REPLY;
  }
  if (std::strcmp(command_name, mm7_command_name::READ_REPLY_RESP) == 0) {
    command_id = MM7_READ_REPLY_RESP;
  }
}

void MmsMsg::setCommandId(uint8_t _command_id) {
  command_id = _command_id;
}

uint8_t MmsMsg::getCommandId() const {
  return command_id;
}

void MmsMsg::setTransactionId(string id) {
  transaction_id = id;
}

string MmsMsg::getTransactionId() const{
  return transaction_id;
}

void MmsMsg::setMmsVersion(string version) {
  mms_version = version;
}

string MmsMsg::getMmsVersion() const{
  return mms_version;
}

void MmsMsg::addField(const char* name, string value) {
  if (name && strcmp(name, "") != 0) {
    mms_fields.Insert(name, value);
  }
}

void MmsMsg::test() {
  __trace2__("Command ID = %d", command_id);
  __trace2__("Transaction ID = %s", transaction_id.c_str());
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
  DOMElement* header = doc->createElementNS(envelope_uri.unicodeForm(), XStr(xml::HEADER).unicodeForm());
  header->setPrefix(env_prefix.unicodeForm());
  root_element->appendChild(header);
  DOMElement* transact_id = doc->createElementNS(XStr(xml::MM7_URI).unicodeForm(), XStr(xml::TRANSACTION_ID).unicodeForm());
  transact_id->setPrefix(XStr(xml::MM7_PREFIX).unicodeForm());
  const std::string* ptr = mms_fields.GetPtr(xml::MUST_UNDERSTAND);
  if (ptr) {
    transact_id->setAttributeNS(envelope_uri.unicodeForm(), XStr(xml::MUST_UNDERSTAND).unicodeForm(), XStr(ptr->c_str()).unicodeForm());
  }
  ptr = mms_fields.GetPtr(xml::ENCODING_STYLE);
  if (ptr) {
    transact_id->setAttributeNS(envelope_uri.unicodeForm(), XStr(xml::ENCODING_STYLE).unicodeForm(), XStr(ptr->c_str()).unicodeForm());
  }
  ptr = mms_fields.GetPtr(xml::ACTOR);
  if (ptr) {
    transact_id->setAttributeNS(envelope_uri.unicodeForm(), XStr(xml::ACTOR).unicodeForm(), XStr(ptr->c_str()).unicodeForm());
  }
  header->appendChild(transact_id);
  if (transaction_id.empty()) {
    __trace__("Serialization Error : Mandatory field \"Transaction ID\" not set");
    return false;
  }
  DOMText* transact_id_val = doc->createTextNode(XStr(transaction_id.c_str()).unicodeForm());
  transact_id->appendChild(transact_id_val);
  DOMElement* body = doc->createElementNS(envelope_uri.unicodeForm(), XStr(xml::BODY).unicodeForm());
  body->setPrefix(env_prefix.unicodeForm());
  root_element->appendChild(body);
  root_element = body;
  return true;
}

string MmsMsg::serialize() const {
  string serialized_msg;
  XMLCh temp_str[100];
  XMLString::transcode("LS", temp_str, 99);
  DOMImplementation* impl = DOMImplementationRegistry::getDOMImplementation(temp_str);
  if (impl == NULL) {
    return serialized_msg;
  } 
  DOMDocument* doc = impl->createDocument(XStr(xml::ENVELOPE_URI).unicodeForm(), XStr(xml::ENVELOPE).unicodeForm(), 0);
  DOMElement* root_element = doc->getDocumentElement();
  root_element->setPrefix(XStr(xml::ENV_PREFIX).unicodeForm());
  
  if (!getXMLDocument(doc, root_element)) {
    doc->release();
    return serialized_msg;
  }
  
  DOMWriter* serializer = ((DOMImplementationLS*)impl)->createDOMWriter();
  std::auto_ptr<DOMErrorHandler> handler(new DOMPrintErrorHandler());
  serializer->setErrorHandler(handler.get());
  //serializer->setEncoding(XStr("UTF-8").unicodeForm());
  XMLCh *str = serializer->writeToString(*doc);
  if (str != NULL) {
    Convertor::UCS2ToUTF8(str, XMLString::stringLen(str), serialized_msg);
  }
  XMLString::release(&str);
  doc->release();
  serializer->release();
  return serialized_msg;
}

MmsMsg::~MmsMsg() {}

MM7GenericVASPReq::MM7GenericVASPReq(string _transaction_id, uint8_t _command_id)
                                     :MmsMsg(_transaction_id, _command_id) {
}

void MM7GenericVASPReq::setSenderAddress(SingleAddress address) {
  sender_address = address;
}

SingleAddress MM7GenericVASPReq::getSenderAddress() const {
  return sender_address;
}

bool MM7GenericVASPReq::getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element,
                                              const char* command_name) const {
  if (!MmsMsg::getXMLDocument(doc, root_element)) {
    return false;
  }
  DOMElement* req_element = doc->createElementNS(XStr(xml::MM7_URI).unicodeForm(),
                                                 XStr(command_name).unicodeForm());
  root_element->appendChild(req_element);
  root_element = req_element;
  const string* ptr = mms_fields.GetPtr(xml::MM7_VERSION);      						 
  string version = getMmsVersion();
  if (!ptr && version.empty()) {
    __trace2__("Serialization Error : Mandatory Field \"%s\" Not Set", xml::MM7_VERSION);
    return false;
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

void MM7GenericVASPReq::test() {
  MmsMsg::test();
  __trace__("Sender Address");
  sender_address.test();
}

MM7GenericVASPReq::~MM7GenericVASPReq() {
}

MM7GenericRSReq::MM7GenericRSReq(string _transaction_id, uint8_t _command_id)
                                     :MmsMsg(_transaction_id, _command_id) {
}

void MM7GenericRSReq::setSenderAddress(SingleAddress address) {
  sender_address = address;
}

SingleAddress MM7GenericRSReq::getSenderAddress() const {
  return sender_address;
}

bool MM7GenericRSReq::getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element,
                                            const char* command_name) const {
  if (!MmsMsg::getXMLDocument(doc, root_element)) {
    return false;
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

MM7Submit::MM7Submit(string _transaction_id):MM7GenericVASPReq(_transaction_id, MM7_SUBMIT) {
}

void MM7Submit::setRecipientAddress(MultiAddress address, uint8_t recipient_type) {
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
  if (!MM7GenericVASPReq::getGenericXMLDocument(doc, root_element, mm7_command_name::SUBMIT)) {
    return false;
  }
  if (to_address.empty() && cc_address.empty() && bcc_address.empty()) {
    __trace__("Serialization Error : Recipient Address Not Set");
    return false;
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

MM7Deliver::MM7Deliver(string _transaction_id):MM7GenericRSReq(_transaction_id, MM7_DELIVER), sequence_number(0) {
}

bool MM7Deliver::getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
  if (!MM7GenericRSReq::getGenericXMLDocument(doc, root_element, mm7_command_name::DELIVER)) {
    return false;
  }
  if (to_address.empty() && cc_address.empty() && bcc_address.empty()) {
    __trace__("Serialization Error : Recipient Address Not Set");
    return false;
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
    typedef std::map<size_t, PreviouslySent>::const_iterator CI;
    for (CI p = previously_sent.begin(); p != previously_sent.end(); ++p) {
      //__trace2__("%d Date : %s Address :", p->first, p->second.sent_date.c_str());
      //p->second.sent_by.test();
    }
  } 
}
void MM7Deliver::setRecipientAddress(MultiAddress address, uint8_t recipient_type) {
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

void MM7Deliver::setPreviouslySentBy(SingleAddress address) {
  previously_sent[sequence_number].sent_by = address;
}

void MM7Deliver::setPreviouslySentDate(string date) {
  previously_sent[sequence_number].sent_date = date;
}

MM7Deliver::~MM7Deliver() {}

MM7Cancel::MM7Cancel(string _transaction_id):MM7GenericVASPReq(_transaction_id, MM7_CANCEL) {};

MM7Cancel::~MM7Cancel() {};

MM7ExtendedCancel::MM7ExtendedCancel(string _transaction_id)
                  :MM7GenericVASPReq(_transaction_id, MM7_EXTENDED_CANCEL) {};

MM7ExtendedCancel::~MM7ExtendedCancel() {};

MM7Replace::MM7Replace(string _transaction_id):MM7GenericVASPReq(_transaction_id, MM7_REPLACE) {};

MM7Replace::~MM7Replace() {};

MM7DeliveryReport::MM7DeliveryReport(string _transaction_id)
                  :MM7GenericRSReq(_transaction_id, MM7_DELIVERY_REPORT) {}

void MM7DeliveryReport::setRecipientAddress(MultiAddress address, uint8_t recipient_type) {
  recipient_address = address;
}

void MM7DeliveryReport::test() {
  MM7GenericRSReq::test();
  __trace__("Recipient Address");
  recipient_address.test();
}

MM7DeliveryReport::~MM7DeliveryReport() {}

MM7ReadReply::MM7ReadReply(string _transaction_id):MM7GenericRSReq(_transaction_id, MM7_READ_REPLY) {}

void MM7ReadReply::setRecipientAddress(MultiAddress address, uint8_t recipient_type) {
  recipient_address = address;
}

void MM7ReadReply::test() {
  MM7GenericRSReq::test();
  __trace__("Recipient Address");
  recipient_address.test();
}

MM7ReadReply::~MM7ReadReply() {}


}//mms
}//transport
}//scag
