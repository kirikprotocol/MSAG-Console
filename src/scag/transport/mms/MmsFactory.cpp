#include "MmsFactory.h"

namespace scag {
namespace transport {
namespace mms {

MmsFactory_::MmsFactory_() {};

MmsMsg* MmsFactory_::createMM7Msg(const char* command_name, const string& transaction_id) {
  if (std::strcmp(command_name, mm7_command_name::SUBMIT) == 0) {
    return new MM7Submit(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::SUBMIT_RESP) == 0) {
    return new MM7SubmitResp(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::DELIVER) == 0) {
    return new MM7Deliver(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::DELIVER_RESP) == 0) {
    return new MM7DeliverResp(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::CANCEL) == 0) {
    return new MM7Cancel(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::CANCEL_RESP) == 0) {
    return new MM7CancelResp(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::EXTENDED_CANCEL) == 0) {
    return new MM7ExtendedCancel(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::EXTENDED_CANCEL_RESP) == 0) {
    return new MM7ExtendedCancelResp(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::REPLACE) == 0) {
    return new MM7Replace(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::REPLACE_RESP) == 0) {
    return new MM7ReplaceResp(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::EXTENDED_REPLACE) == 0) {
    return new MM7ExtendedReplace(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::EXTENDED_REPLACE_RESP) == 0) {
    return new MM7ExtendedReplaceResp(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::DELIVERY_REPORT) == 0) {
    return new MM7DeliveryReport(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::DELIVERY_REPORT_RESP) == 0) {
    return new MM7DeliveryReportResp(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::READ_REPLY) == 0) {
    return new MM7ReadReply(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::READ_REPLY_RESP) == 0) {
    return new MM7ReadReplyResp(transaction_id);
  }
  if (std::strcmp(command_name, mm7_command_name::FAULT) == 0) {
    return new GenericResponse(transaction_id);
  }
  return NULL;
}

MmsFactory_::~MmsFactory_() {};

Hash<MmsFactory*> MmsFactory::factories;

void MmsFactory::initFactories() {
}
   
void MmsFactory::deleteFactories() {
  //if (factories) {
    //factories->Empty();
  //}
}

MmsMsg* MmsFactory::getMmsMsg(const char* key, const string& transaction_id) {
  MmsFactory* f = factories.Exists(key) ? factories.Get(key) : 0;
  return f ? f->createMmsMsg(transaction_id) : 0; 
}


}//mms
}//transport
}//scag
