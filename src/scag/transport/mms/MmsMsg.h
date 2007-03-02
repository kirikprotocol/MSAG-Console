#ifndef __SCAG_MMS_MSG_H__
#define __SCAG_MMS_MSG_H__

#include <string>
#include <vector>
#include <map>

#include "util/Exception.hpp"
//#include "scag/transport/SCAGCommand.h"
//#include "core/synchronization/Mutex.hpp"
#include "core/buffers/Hash.hpp"
#include "logger/Logger.h"
#include "sms/sms.h"
#include "scag/util/encodings/Encodings.h"

#include "scag/transport/mms/util.h"
#include "DOMPrintErrorHandler.h"

namespace scag {
namespace transport{
namespace mms{

using std::string;
using std::vector;
using smsc::core::buffers::Hash;
using scag::util::encodings::Convertor;

XERCES_CPP_NAMESPACE_USE

namespace mm7_command_name {
  static const char* SUBMIT                = "SubmitReq";
  static const char* SUBMIT_RESP           = "SubmitRsp";
  static const char* DELIVER               = "DeliverReq";
  static const char* DELIVER_RESP          = "DeliverRsp";
  static const char* CANCEL                = "CancelReq";
  static const char* CANCEL_RESP           = "CancelRsp";
  static const char* REPLACE               = "ReplaceReq";
  static const char* REPLACE_RESP          = "ReplaceRsp";
  static const char* DELIVERY_REPORT       = "DeliveryReportReq";
  static const char* DELIVERY_REPORT_RESP  = "DeliveryReportRsp";
  static const char* READ_REPLY            = "ReadReplyReq";
  static const char* READ_REPLY_RESP       = "ReadReplyRsp";
  static const char* EXTENDED_CANCEL       = "extendedCancelReq";
  static const char* EXTENDED_CANCEL_RESP  = "extendedCancelRsp";    
  static const char* EXTENDED_REPLACE      = "extendedReplaceReq";
  static const char* EXTENDED_REPLACE_RESP = "extendedReplaceRsp";
  static const char* RS_ERROR_RESP         = "RSErrorRsp";
  static const char* VASP_ERROR_RESP       = "VASPErrorRsp";
  static const char* FAULT                 = "env:Fault";
};

//namespace mm7_string_consts {
  //static const char* ENCRYPTED  = "encrypted";
  //static const char* OBFUSCATED = "obfuscated";
//}

enum CommandId {
  MMS_UNKNOWN,               // 0 
  MM7_SUBMIT,                // 1
  MM7_SUBMIT_RESP,           // 2
  MM7_DELIVER,               // 3 
  MM7_DELIVER_RESP,          // 4
  MM7_CANCEL,                // 5
  MM7_CANCEL_RESP,           // 6
  MM7_REPLACE,               // 7
  MM7_REPLACE_RESP,          // 8
  MM7_DELIVERY_REPORT,       // 9
  MM7_DELIVERY_REPORT_RESP,  // 10
  MM7_READ_REPLY,            // 11
  MM7_READ_REPLY_RESP,       // 12
  MM7_EXTENDED_CANCEL,       // 13
  MM7_EXTENDED_CANCEL_RESP,  // 14  
  MM7_EXTENDED_REPLACE,      // 15
  MM7_EXTENDED_REPLACE_RESP, // 16
  MM7_RS_ERROR_RESP,         // 17
  MM7_VASP_ERROR_RESP,       // 18
  MM4_FORWARD,               // 19
  MM4_FORWARD_RESP,          // 20
  MM4_DELIVERY_REPORT,       // 21
  MM4_DELIVERY_REPORT_RESP,  // 22
  MM4_READ_REPLY_REPORT,     // 23
  MM4_READ_REPLY_REPORT_RESP,// 24
  MMS_GENERIC_RESP           // 25
};

struct Address {
private:
  string value;
  bool display_only;
  uint8_t coding_type;
public:
  Address();
  ~Address();
  void setCodingType(const char* type);
  void setCodingType(uint8_t type);
  uint8_t getCodingType() const;
  void setDisplayOnly(const char* _display_only);
  void setDisplayOnly(bool _display_only);
  bool getDisplayOnly() const;
  bool isNotSet() const;
  void setValue(string _value);
  string getValue() const;
  void serialize(DOMDocument* doc, DOMElement* parent, const char* address_type) const;
  void reset();
  void test() ;
};

enum AddressType {
  NUMBER,
  SHORT_CODE,
  RFC2822
};


struct MultiAddress {
  Address number;
  Address short_code;
  Address rfc2822;
  bool isNotSet() const {
    if (number.isNotSet() && short_code.isNotSet() && rfc2822.isNotSet()) {
      return true;
    } else {
      return false;
    }
  }
  void serialize(DOMDocument* doc, DOMElement* parent, const char* parent_tag_name) const {
    if (!parent_tag_name || isNotSet()) {
      return;
    }
    DOMElement* address_element = doc->createElement(XStr(parent_tag_name).unicodeForm());
    number.serialize(doc, address_element, xml::NUMBER);
    short_code.serialize(doc, address_element, xml::SHORT_CODE);
    rfc2822.serialize(doc, address_element, xml::RFC2822);
    parent->appendChild(address_element);
  }
  void reset() {
    number.reset();
    short_code.reset();
    rfc2822.reset();
  };
  void test() {
    number.test();
    short_code.test();
    rfc2822.test();
  }
};

struct SingleAddress {
  uint8_t type;
  Address address;
  SingleAddress():type(NUMBER) {};
  SingleAddress(const MultiAddress& multi_address) {
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
  void serialize(DOMDocument* doc, DOMElement* parent, const char* parent_tag_name) const {
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
  void serialize(DOMDocument* doc, DOMElement* parent) const {
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
  void test() { address.test(); }
  ~SingleAddress() {}; 
};

enum RecipientType {
  TO,
  CC,
  BCC
};

struct MmsMsg {
private:
  uint8_t command_id; 
  string transaction_id;
  string mms_version;
  //string msg_type;
protected:
  Hash<string> mms_fields;  
  virtual bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
public:
  MmsMsg();
  MmsMsg(string _transaction_id, uint8_t _command_id);
  virtual ~MmsMsg();
  string serialize() const;
  virtual void deserialize(const char*) {};
  virtual void addField(const char* name, std::string value);
  virtual void setSenderAddress(SingleAddress address) {};
  virtual void setRecipientAddress(MultiAddress address, uint8_t recipient_type = TO) {};
  virtual void test();
  void setTransactionId(string _transaction_id);
  string getTransactionId() const;
  void setMmsVersion(string version);
  string getMmsVersion() const;
  uint8_t getCommandId() const;
  void setCommandId(uint8_t _command_id);
  void setCommandId(const char* command_name);
  //void setMsgType(string _msgType);
  //string getMsgType() const;
};

struct MM7GenericVASPReq : public MmsMsg {
private:
  SingleAddress sender_address;
protected:
  bool getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element, const char* command_name) const;
public:
  MM7GenericVASPReq(string _transaction_id, uint8_t _command_id);
  virtual ~MM7GenericVASPReq();
  virtual void setSenderAddress(SingleAddress address);
  SingleAddress getSenderAddress() const;	
  virtual void test();
};

struct MM7GenericRSReq : public MmsMsg {
private:
  SingleAddress sender_address;
protected:
  bool getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element, const char* command_name) const;
public:
  MM7GenericRSReq(string _transaction_id, uint8_t _command_id);
  virtual ~MM7GenericRSReq();
  virtual void setSenderAddress(SingleAddress address);
  SingleAddress getSenderAddress() const;	
  virtual void test();
};

enum AddressCoding {
  ENCRYPTED = 1,
  OBFUSCATED
};

enum MessageClass {
  INFORMATIONAL,
  PERSONAL,
  ADVERTISEMENT,
  AUTO
};

enum Priority {
  LOW,
  NORMAL,
  HIGH
};

enum ChargedParty {
  SENDER,
  RCIPIENT,
  BOTH,
  NEITHER,
  THIRDPARTY
};

enum ContentClass {
  TEXT,
  IMAGE_BASIC,
  IMAGE_RICH,
  VIDEO_BASIC,
  MEGAPIXEL,
  CONTENT_BASIC,
  CONTENT_RICH
};

enum DeliveryCondition {
  MMS_CAPABLE, //VASP/VAS intended the MM to be sent to a recipient's terminal that has MMS capabilities
  HPLMN  //VASP/VAS intended the MM to be sent to a recipient's terminal that is within the PLMN(i.e. not roaming) 
};


struct MM7Submit: public MM7GenericVASPReq {
private:
  vector<MultiAddress> to_address; //mandatory
  vector<MultiAddress> cc_address;  
  vector<MultiAddress> bcc_address; 
  //string vasp_id;                  
  //string vas_id; 
  //string service_code; 
  //uint8_t message_class; 
  //time_t date;
  //time_t expiry_date;
  //time_t earliest_delivery_time;
  //bool delivery_report;
  //bool read_reply;
  //bool reply_charging;
  //time_t reply_deadline;
  //uint32_t reply_charging_size;
  //uint8_t priority;
  //string subject;
  //bool allow_adaptations;
  //uint8_t charged_party;
  //bool distribution;
  //bool drm_content;
  //uint8_t content_class;
  //uint32_t delivery_condition;
  //string applic_id;
  //string reply_applic_id;
  //string aux_applic_info;
  //string content_type;  // mandatory MIME header of Attachment
  //string content;  // href:cid attribute links to attachment
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
public:
  MM7Submit(string _transaction_id);
  ~MM7Submit();
  void setRecipientAddress(MultiAddress address, uint8_t recipient_type = TO);
  void test();
};

struct GenericResponse : public MmsMsg {
private:
public:
  GenericResponse(string _transaction_id, uint8_t _command_id):MmsMsg(_transaction_id, _command_id) {};
  GenericResponse(string _transaction_id):MmsMsg(_transaction_id, MMS_GENERIC_RESP) {}; 
  ~GenericResponse() {};
};

struct MM7SubmitResp: public GenericResponse {
private:
  //string message_id;  //conditional : if status indicates success then this contains MMS R/S generated identification
                      //              of submutted message. This ID may be used in subsequent request and reports relating
		      //              to this message
  //uint32_t status_code; //mandatory
  //string status_text;
public:
  MM7SubmitResp(string _transaction_id):GenericResponse(_transaction_id, MM7_SUBMIT_RESP) {};
  ~MM7SubmitResp() {};
};

struct PreviouslySent {
  SingleAddress sent_by;
  string sent_date;
};

struct MM7Deliver: public MM7GenericRSReq {
private:
  vector<MultiAddress> to_address; //mandatory
  vector<MultiAddress> cc_address;  
  vector<MultiAddress> bcc_address; 
  size_t sequence_number;
  std::map<size_t, PreviouslySent> previously_sent;
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
public:
  MM7Deliver(string _transaction_id);
  ~MM7Deliver();
  virtual void setRecipientAddress(MultiAddress address, uint8_t recipient_type = TO);  
  void setSequenceNumber(size_t number);
  void setPreviouslySentBy(SingleAddress address);
  void setPreviouslySentDate(string date);
  void test();
};

struct MM7DeliverResp : public GenericResponse {
private:
public:
  MM7DeliverResp(string _transaction_id):GenericResponse(_transaction_id, MM7_DELIVER_RESP) {};
  ~MM7DeliverResp() {};
};

struct MM7Cancel : public MM7GenericVASPReq {
private:
public:
  MM7Cancel(string _transaction_id);
  ~MM7Cancel();
  //void test();
};

struct MM7CancelResp : public GenericResponse {
private:
public:
  MM7CancelResp(string _transaction_id):GenericResponse(_transaction_id, MM7_CANCEL_RESP) {};
  ~MM7CancelResp() {};
};

struct MM7ExtendedCancel : public MM7GenericVASPReq {
private:
public:
  MM7ExtendedCancel(string _transaction_id);
  ~MM7ExtendedCancel();
  //void test();
};

struct MM7ExtendedCancelResp : public GenericResponse {
private:
public:
  MM7ExtendedCancelResp(string _transaction_id):GenericResponse(_transaction_id, MM7_EXTENDED_CANCEL_RESP) {};
  ~MM7ExtendedCancelResp() {};
};

struct MM7Replace : public MM7GenericVASPReq {
private:
public:
  MM7Replace(string _transaction_id);
  ~MM7Replace();
  //void test();
};

struct MM7ReplaceResp : public GenericResponse {
private:
public:
  MM7ReplaceResp(string _transaction_id):GenericResponse(_transaction_id, MM7_REPLACE_RESP) {};
  ~MM7ReplaceResp() {};
};

struct MM7ExtendedReplace : public MmsMsg {
private:
public:
  MM7ExtendedReplace(string _transaction_id):MmsMsg(_transaction_id, MM7_EXTENDED_REPLACE) {};
  ~MM7ExtendedReplace() {};
};

struct MM7ExtendedReplaceResp : public GenericResponse {
private:
public:
  MM7ExtendedReplaceResp(string _transaction_id):GenericResponse(_transaction_id, MM7_EXTENDED_REPLACE_RESP) {};
  ~MM7ExtendedReplaceResp() {};
};

struct MM7DeliveryReport : public MM7GenericRSReq {
private:
  SingleAddress recipient_address;
public:
  MM7DeliveryReport(string _transaction_id);
  ~MM7DeliveryReport();
  void setRecipientAddress(MultiAddress address, uint8_t recipient_type = TO);
  void test();
};

struct MM7DeliveryReportResp : public GenericResponse {
private:
public:
  MM7DeliveryReportResp(string _transaction_id):GenericResponse(_transaction_id, MM7_DELIVERY_REPORT_RESP) {};
  ~MM7DeliveryReportResp() {};
};

struct MM7ReadReply : public MM7GenericRSReq {
private:
  SingleAddress recipient_address;
public:
  MM7ReadReply(string _transaction_id);
  ~MM7ReadReply();
  void setRecipientAddress(MultiAddress address, uint8_t recipient_type = TO);
  void test();
};

struct MM7ReadReplyResp : public GenericResponse {
private:
public:
  MM7ReadReplyResp(string _transaction_id):GenericResponse(_transaction_id, MM7_READ_REPLY_RESP) {};
  ~MM7ReadReplyResp() {};
};

struct MM7RSErrorResp : public GenericResponse {
private:
public:
  MM7RSErrorResp(string _transaction_id):GenericResponse(_transaction_id, MM7_RS_ERROR_RESP) {};
  ~MM7RSErrorResp() {};
};

struct MM7VASPErrorResp : public GenericResponse {
private:
public:
  MM7VASPErrorResp(string _transaction_id):GenericResponse(_transaction_id, MM7_VASP_ERROR_RESP) {};
  ~MM7VASPErrorResp() {};
};


}//mms
}//transport
}//scag

#endif
