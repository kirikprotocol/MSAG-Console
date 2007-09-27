#include "MmsFactory.h"

namespace scag {
namespace transport {
namespace mms {

Hash<MmsFactory*> MmsFactory::factories;

void MmsFactory::registerFactory(const char* key, MmsFactory* factory) {
  if (!factories.Exists(key)) {
    factories.Insert(key, factory);
  }
}

void MmsFactory::initFactories() {
  registerFactory(mm7_command_name::SUBMIT, new MM7SubmitFactory());
  registerFactory(mm7_command_name::SUBMIT_RESP, new MM7SubmitRespFactory());
  registerFactory(mm7_command_name::DELIVER, new MM7DeliverFactory());
  registerFactory(mm7_command_name::DELIVER_RESP, new MM7DeliverRespFactory());
  registerFactory(mm7_command_name::CANCEL, new MM7CancelFactory());
  registerFactory(mm7_command_name::CANCEL_RESP, new MM7CancelRespFactory());
  registerFactory(mm7_command_name::EXTENDED_CANCEL, new MM7ExtendedCancelFactory());
  registerFactory(mm7_command_name::EXTENDED_CANCEL_RESP,
                   new MM7ExtendedCancelRespFactory());
  registerFactory(mm7_command_name::REPLACE, new MM7ReplaceFactory());
  registerFactory(mm7_command_name::REPLACE_RESP, new MM7ReplaceRespFactory());
  registerFactory(mm7_command_name::EXTENDED_REPLACE, new MM7ExtendedReplaceFactory());
  registerFactory(mm7_command_name::EXTENDED_REPLACE_RESP,
                   new MM7ExtendedReplaceRespFactory());
  registerFactory(mm7_command_name::READ_REPLY, new MM7ReadReplyFactory());
  registerFactory(mm7_command_name::READ_REPLY_RESP, new MM7ReadReplyRespFactory());
  registerFactory(mm7_command_name::DELIVERY_REPORT, new MM7DeliveryReportFactory());
  registerFactory(mm7_command_name::DELIVERY_REPORT_RESP,
                   new MM7DeliveryReportRespFactory());
  registerFactory(mm7_command_name::FAULT, new GenericResponseFactory());
}
   
MmsMsg* MmsFactory::getMmsMsg(const char* key, const string& transaction_id) {
  MmsFactory* f = factories.Exists(key) ? factories.Get(key) : 0;
  return f ? f->createMmsMsg(transaction_id) : 0; 
}


}//mms
}//transport
}//scag
