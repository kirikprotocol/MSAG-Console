#ifndef __SCAG_TRANSPORT_MMS_FACTORY_H__
#define __SCAG_TRANSPORT_MMS_FACTORY_H__

#include <string>

#include "MmsMsg.h"

namespace scag {
namespace transport {
namespace mms {

class MmsFactory {
public:
  MmsFactory() {};
  virtual ~MmsFactory() {};
  virtual MmsMsg* createMmsMsg(const string& transaction_id) = 0;
  static MmsMsg* getMmsMsg(const char* name, const string& transaction_id);
private:
  static Hash<MmsFactory*> factories;
protected:
  static void registerFactory(const char* name, MmsFactory* factory);
};
class MM7SubmitFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7Submit(transaction_id);
  }
  MM7SubmitFactory() {
    registerFactory(mm7_command_name::SUBMIT, this);
  }
};

class MM7SubmitRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7SubmitResp(transaction_id);
  }
  MM7SubmitRespFactory() {
    registerFactory(mm7_command_name::SUBMIT_RESP, this);
  }
};

class MM7DeliverFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7Deliver(transaction_id);
  }
  MM7DeliverFactory() {
    registerFactory(mm7_command_name::DELIVER, this);
  }
};

class MM7DeliverRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7DeliverResp(transaction_id);
  }
  MM7DeliverRespFactory() {
    registerFactory(mm7_command_name::DELIVER_RESP, this);
  }
};

class MM7CancelFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7Cancel(transaction_id);
  }
  MM7CancelFactory() {
    registerFactory(mm7_command_name::CANCEL, this);
  }
};

class MM7CancelRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7CancelResp(transaction_id);
  }
  MM7CancelRespFactory() {
    registerFactory(mm7_command_name::CANCEL_RESP, this);
  }
};

class MM7ExtendedCancelFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ExtendedCancel(transaction_id);
  }
  MM7ExtendedCancelFactory() {
    registerFactory(mm7_command_name::EXTENDED_CANCEL, this);
  }
};

class MM7ExtendedCancelRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ExtendedCancelResp(transaction_id);
  }
  MM7ExtendedCancelRespFactory() {
    registerFactory(mm7_command_name::EXTENDED_CANCEL_RESP, this);
  }
};

class MM7ReplaceFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7Replace(transaction_id);
  }
  MM7ReplaceFactory() {
    registerFactory(mm7_command_name::REPLACE, this);
  }
};

class MM7ReplaceRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ReplaceResp(transaction_id);
  }
  MM7ReplaceRespFactory() {
    registerFactory(mm7_command_name::REPLACE_RESP, this);
  }
};

class MM7ExtendedReplaceFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ExtendedReplace(transaction_id);
  }
  MM7ExtendedReplaceFactory() {
    registerFactory(mm7_command_name::EXTENDED_REPLACE, this);
  }
};

class MM7ExtendedReplaceRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ExtendedReplaceResp(transaction_id);
  }
  MM7ExtendedReplaceRespFactory() {
    registerFactory(mm7_command_name::EXTENDED_REPLACE_RESP, this);
  }
};

class MM7ReadReplyFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ReadReply(transaction_id);
  }
  MM7ReadReplyFactory() {
    registerFactory(mm7_command_name::READ_REPLY, this);
  }
};

class MM7ReadReplyRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ReadReplyResp(transaction_id);
  }
  MM7ReadReplyRespFactory() {
    registerFactory(mm7_command_name::READ_REPLY_RESP, this);
  }
};

class MM7DeliveryReportFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7DeliveryReport(transaction_id);
  }
  MM7DeliveryReportFactory() {
    registerFactory(mm7_command_name::DELIVERY_REPORT, this);
  }
};

class MM7DeliveryReportRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7DeliveryReportResp(transaction_id);
  }
  MM7DeliveryReportRespFactory() {
    registerFactory(mm7_command_name::DELIVERY_REPORT_RESP, this);
  }
};

class GenericResponseFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new GenericResponse(transaction_id);
  }
  GenericResponseFactory() {
    registerFactory(mm7_command_name::FAULT, this);
  }
};

}//mms
}//transport
}//scag

#endif
