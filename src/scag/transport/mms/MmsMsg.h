#ifndef __SCAG_TRANSPORT_MMS_MSG_H__
#define __SCAG_TRANSPORT_MMS_MSG_H__

#include <string>
#include <vector>
#include <map>

#include "core/buffers/Hash.hpp"
#include "logger/Logger.h"
#include "scag/util/encodings/Encodings.h"

#include "DOMPrintErrorHandler.h"

namespace scag {
namespace transport{
namespace mms{

using std::string;
using std::vector;
using smsc::core::buffers::Hash;
using scag::util::encodings::Convertor;
using smsc::util::Exception;

XERCES_CPP_NAMESPACE_USE


namespace mm7_command_name {
  extern const char* SUBMIT;
  extern const char* SUBMIT_RESP;;
  extern const char* DELIVER;
  extern const char* DELIVER_RESP;
  extern const char* CANCEL;
  extern const char* CANCEL_RESP;
  extern const char* REPLACE;
  extern const char* REPLACE_RESP;
  extern const char* DELIVERY_REPORT;
  extern const char* DELIVERY_REPORT_RESP;
  extern const char* READ_REPLY;
  extern const char* READ_REPLY_RESP;
  extern const char* EXTENDED_CANCEL;
  extern const char* EXTENDED_CANCEL_RESP;  
  extern const char* EXTENDED_REPLACE;
  extern const char* EXTENDED_REPLACE_RESP;
  extern const char* RS_ERROR_RESP;
  extern const char* VASP_ERROR_RESP;
  extern const char* FAULT;
};

enum CommandId {
  MMS_UNKNOWN,               // 0 

  MM7_SUBMIT,                // 1
  MM7_CANCEL,                // 2
  MM7_EXTENDED_CANCEL,       // 3
  MM7_REPLACE,               // 4
  MM7_EXTENDED_REPLACE,      // 5
  MM7_DELIVER,               // 6 
  MM7_DELIVERY_REPORT,       // 7
  MM7_READ_REPLY,            // 8

  MM7_SUBMIT_RESP,           // 9
  MM7_CANCEL_RESP,           // 10
  MM7_EXTENDED_CANCEL_RESP,  // 11  
  MM7_REPLACE_RESP,          // 12
  MM7_EXTENDED_REPLACE_RESP, // 13
  MM7_VASP_ERROR_RESP,       // 14
  MM7_DELIVER_RESP,          // 15
  MM7_DELIVERY_REPORT_RESP,  // 16
  MM7_READ_REPLY_RESP,       // 17
  MM7_RS_ERROR_RESP,         // 18

  MM4_FORWARD,               // 19
  MM4_DELIVERY_REPORT,       // 20
  MM4_READ_REPLY_REPORT,     // 21
  MM4_FORWARD_RESP,          // 22
  MM4_DELIVERY_REPORT_RESP,  // 23
  MM4_READ_REPLY_REPORT_RESP,// 24
                             // 
  MMS_GENERIC_RESP           // 25
};

struct Address {
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
  void setValue(const string& _value);
  const string& getValue() const;
  void serialize(DOMDocument* doc, DOMElement* parent, const char* address_type) const;
  void reset();
  void test() const;
private:
  string value;
  bool display_only;
  uint8_t coding_type;
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
  bool isNotSet() const;
  void serialize(DOMDocument* doc, DOMElement* parent, const char* parent_tag_name) const;
  void reset();
  void test() const;
};

struct SingleAddress {
  uint8_t type;
  Address address;
  SingleAddress();
  SingleAddress(const MultiAddress& multi_address);
  void serialize(DOMDocument* doc, DOMElement* parent, const char* parent_tag_name) const;
  void serialize(DOMDocument* doc, DOMElement* parent) const;
  void test() const;
};

enum RecipientType {
  TO,
  CC,
  BCC
};

struct MmsMsg {
public:
  MmsMsg();
  MmsMsg(const string& _transaction_id, uint8_t _command_id);
  virtual ~MmsMsg();
  bool serialize(string& serilized_msg) const;
  virtual void deserialize(const char*) {};
  virtual void setSenderAddress(const SingleAddress& address) {};
  virtual void setRecipientAddress(const MultiAddress& address,
                                   uint8_t recipient_type = TO) {};
  virtual MmsMsg* getResponse() const { return NULL; }
  virtual const string* getEndpointId() const { return NULL; };
  virtual void setEndpointId(const string& endpointId) {};
  virtual void test();

  void setTransactionId(const string& _transaction_id);
  const string& getTransactionId() const;
  void setMmsVersion(const string& version);
  const string& getMmsVersion() const;
  uint8_t getCommandId() const;
  void setCommandId(uint8_t _command_id);
  const string* getInfoElement(const char* element_name) const;
  void setInfoElement(const char* name, const string& value);
  bool isMM7Req() const;
  bool isMM4Req() const;

protected:
  uint8_t command_id; 
  string transaction_id;
  string mms_version;
  Hash<string> mms_fields;  
  virtual bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
};

struct MM7GenericVASPReq : public MmsMsg {
public:
  MM7GenericVASPReq(const string& _transaction_id, uint8_t _command_id);
  virtual ~MM7GenericVASPReq();
  virtual void setSenderAddress(const SingleAddress& address);
  const SingleAddress& getSenderAddress() const; 	
  const string* getEndpointId() const;
  void setEndpointId(const string& vasp_id);
  virtual void test() ;
protected:
  bool getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element,
                             const char* command_name) const;
private:
  SingleAddress sender_address;
};

struct MM7GenericRSReq : public MmsMsg {
public:
  MM7GenericRSReq(const string& _transaction_id, uint8_t _command_id);
  virtual ~MM7GenericRSReq();
  virtual void setSenderAddress(const SingleAddress& address);
  const SingleAddress& getSenderAddress() const;	
  const string* getEndpointId() const;
  void setEndpointId(const string& rs_id);
  virtual void test();
protected:
  SingleAddress sender_address;
  bool getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element, const char* command_name) const;
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
public:
  MM7Submit(const string& _transaction_id);
  ~MM7Submit();
  void setRecipientAddress(const MultiAddress& address, uint8_t recipient_type = TO);
  void test();
  MmsMsg* getResponse() const;
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
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
};

struct GenericResponse : public MmsMsg {
public:
  GenericResponse(const string& _transaction_id, uint8_t _command_id)
                  :MmsMsg(_transaction_id, _command_id) {};
  GenericResponse(const string& _transaction_id)
                  :MmsMsg(_transaction_id, MMS_GENERIC_RESP) {}; 
  ~GenericResponse() {};
protected:
  virtual bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
  bool getGenericXMLDocument(DOMDocument* doc, DOMElement*& root_element, const char* command_name) const;
};

struct MM7SubmitResp: public GenericResponse {
public:
  MM7SubmitResp(const string& _transaction_id)
                :GenericResponse(_transaction_id, MM7_SUBMIT_RESP) {};
  ~MM7SubmitResp() {};
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
//private:
  //string message_id;  //conditional : if status indicates success then this contains MMS R/S generated identification
                      //              of submutted message. This ID may be used in subsequent request and reports relating
		      //              to this message
  //uint32_t status_code; //mandatory
  //string status_text;
};

struct PreviouslySent {
  SingleAddress sent_by;
  string sent_date;
};

struct MM7Deliver: public MM7GenericRSReq {
public:
  MM7Deliver(const string& _transaction_id);
  ~MM7Deliver();
  virtual void setRecipientAddress(const MultiAddress& address, 
                                   uint8_t recipient_type = TO);  
  void setSequenceNumber(size_t number);
  void setPreviouslySentBy(const SingleAddress& address);
  void setPreviouslySentDate(const string& date);
  MmsMsg* getResponse() const;
  void test();
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
private:
  vector<MultiAddress> to_address; //mandatory
  vector<MultiAddress> cc_address;  
  vector<MultiAddress> bcc_address; 
  size_t sequence_number;
  std::map<size_t, PreviouslySent> previously_sent;
};

struct MM7DeliverResp : public GenericResponse {
public:
  MM7DeliverResp(const string& _transaction_id)
                 :GenericResponse(_transaction_id, MM7_DELIVER_RESP) {};
  ~MM7DeliverResp() {};
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
private:
};

struct MM7Cancel : public MM7GenericVASPReq {
public:
  MM7Cancel(const string& _transaction_id);
  ~MM7Cancel();
  MmsMsg* getResponse() const;
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
};

struct MM7CancelResp : public GenericResponse {
public:
  MM7CancelResp(const string& _transaction_id)
                :GenericResponse(_transaction_id, MM7_CANCEL_RESP) {};
  ~MM7CancelResp() {};
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
    return GenericResponse::getGenericXMLDocument(doc, root_element,
                                                  mm7_command_name::CANCEL_RESP);
  }
};

struct MM7ExtendedCancel : public MM7GenericVASPReq {
public:
  MM7ExtendedCancel(const string& _transaction_id);
  ~MM7ExtendedCancel();
  MmsMsg* getResponse() const;
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
};

struct MM7ExtendedCancelResp : public GenericResponse {
public:
  MM7ExtendedCancelResp(const string& _transaction_id)
                       :GenericResponse(_transaction_id, MM7_EXTENDED_CANCEL_RESP) {};
  ~MM7ExtendedCancelResp() {};
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
};

struct MM7Replace : public MM7GenericVASPReq {
public:
  MM7Replace(const string& _transaction_id);
  ~MM7Replace();
  MmsMsg* getResponse() const;
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
};

struct MM7ReplaceResp : public GenericResponse {
public:
  MM7ReplaceResp(const string& _transaction_id)
                 :GenericResponse(_transaction_id, MM7_REPLACE_RESP) {};
  ~MM7ReplaceResp() {};
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
    return GenericResponse::getGenericXMLDocument(doc, root_element,
                                                  mm7_command_name::REPLACE_RESP);
  }
};

struct MM7ExtendedReplace : public MmsMsg {
public:
  MM7ExtendedReplace(const string& _transaction_id);
  ~MM7ExtendedReplace();
  MmsMsg* getResponse() const;
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
};

struct MM7ExtendedReplaceResp : public GenericResponse {
public:
  MM7ExtendedReplaceResp(const string& _transaction_id)
                         :GenericResponse(_transaction_id, MM7_EXTENDED_REPLACE_RESP) {};
  ~MM7ExtendedReplaceResp() {};
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
};

struct MM7DeliveryReport : public MM7GenericRSReq {
public:
  MM7DeliveryReport(const string& _transaction_id);
  ~MM7DeliveryReport();
  void setRecipientAddress(const MultiAddress& address, uint8_t recipient_type = TO);
  MmsMsg* getResponse() const;
  void test();
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
private:
  SingleAddress recipient_address;
};

struct MM7DeliveryReportResp : public GenericResponse {
public:
  MM7DeliveryReportResp(const string& _transaction_id)
                        :GenericResponse(_transaction_id, MM7_DELIVERY_REPORT_RESP) {};
  ~MM7DeliveryReportResp() {};
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
    return GenericResponse::getGenericXMLDocument(doc, root_element,
                                                 mm7_command_name::DELIVERY_REPORT_RESP);
  }
};

struct MM7ReadReply : public MM7GenericRSReq {
public:
  MM7ReadReply(const string& _transaction_id);
  ~MM7ReadReply();
  void setRecipientAddress(const MultiAddress& address, uint8_t recipient_type = TO);
  MmsMsg* getResponse() const;
  void test();
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const;
private:
  SingleAddress recipient_address;
};

struct MM7ReadReplyResp : public GenericResponse {
public:
  MM7ReadReplyResp(const string& _transaction_id)
                   :GenericResponse(_transaction_id, MM7_READ_REPLY_RESP) {};
  ~MM7ReadReplyResp() {};
protected:
  bool getXMLDocument(DOMDocument* doc, DOMElement*& root_element) const {
    return GenericResponse::getGenericXMLDocument(doc, root_element,
                                                   mm7_command_name::READ_REPLY_RESP);
  }
};

struct MM7RSErrorResp : public GenericResponse {
public:
  MM7RSErrorResp(const string& _transaction_id)
                 :GenericResponse(_transaction_id, MM7_RS_ERROR_RESP) {};
  ~MM7RSErrorResp() {};
};

struct MM7VASPErrorResp : public GenericResponse {
public:
  MM7VASPErrorResp(const string& _transaction_id)
                   :GenericResponse(_transaction_id, MM7_VASP_ERROR_RESP) {};
  ~MM7VASPErrorResp() {};
};


}//mms
}//transport
}//scag

#endif

