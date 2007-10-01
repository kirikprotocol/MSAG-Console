#include "MmsFactory.h"
#include "logger/Logger.h"

namespace scag {
namespace transport {
namespace mms {

using smsc::logger::Logger;

Hash<MmsFactory*> MmsFactory::factories;

static const MM7SubmitFactory _MM7SubmitFactory;
static const MM7SubmitRespFactory _MM7SubmitRespFactory;
static const MM7DeliverFactory _MM7DeliverFactory;
static const MM7DeliverRespFactory _MM7DeliverRespFactory;
static const MM7CancelFactory _MM7CancelFactory;
static const MM7CancelRespFactory _MM7CancelRespFactory;
static const MM7ExtendedCancelFactory _MM7ExtendedCancelFactory;
static const MM7ExtendedCancelRespFactory _MM7ExtendedCancelRespFactory;
static const MM7ReplaceFactory _MM7ReplaceFactory;
static const MM7ReplaceRespFactory _MM7ReplaceRespFactory;
static const MM7ExtendedReplaceFactory _MM7ExtendedReplaceFactory;
static const MM7ExtendedReplaceRespFactory _MM7ExtendedReplaceRespFactory;
static const MM7ReadReplyFactory _MM7ReadReplyFactory;
static const MM7ReadReplyRespFactory _MM7ReadReplyRespFactory;
static const MM7DeliveryReportFactory _MM7DeliveryReportFactory;
static const MM7DeliveryReportRespFactory _MM7DeliveryReportRespFactory;
static const GenericResponseFactory _GenericResponseFactory;

void MmsFactory::registerFactory(const char* key, MmsFactory* factory) {
  if (!factories.Exists(key)) {
    factories.Insert(key, factory);
  }
}

MmsMsg* MmsFactory::getMmsMsg(const char* key, const string& transaction_id) {
  MmsFactory* f = 0;
  if (factories.Exists(key)) {
    f = factories.Get(key);
    return f->createMmsMsg(transaction_id); 
  } else {
    smsc_log_warn(Logger::getInstance("mms.fact"),"Factory \'%s\' doesn't exist", key);
    return 0;
  }
}


}//mms
}//transport
}//scag

