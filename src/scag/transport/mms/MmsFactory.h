#ifndef __SCAG_MMS_FACTORY_H__
#define __SCAG_MMS_FACTORY_H__

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
  static void initFactories();
private:
  static void registerFactory(const char* name, MmsFactory* factory);
private:
  static Hash<MmsFactory*> factories;
};

class MM7SubmitFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7Submit(transaction_id);
  }
};

class MM7SubmitRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7SubmitResp(transaction_id);
  }
};

class MM7DeliverFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7Deliver(transaction_id);
  }
};

class MM7DeliverRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7DeliverResp(transaction_id);
  }
};

class MM7CancelFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7Cancel(transaction_id);
  }
};

class MM7CancelRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7CancelResp(transaction_id);
  }
};

class MM7ExtendedCancelFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ExtendedCancel(transaction_id);
  }
};

class MM7ExtendedCancelRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ExtendedCancelResp(transaction_id);
  }
};

class MM7ReplaceFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7Replace(transaction_id);
  }
};

class MM7ReplaceRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ReplaceResp(transaction_id);
  }
};

class MM7ExtendedReplaceFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ExtendedReplace(transaction_id);
  }
};

class MM7ExtendedReplaceRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ExtendedReplaceResp(transaction_id);
  }
};

class MM7ReadReplyFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ReadReply(transaction_id);
  }
};

class MM7ReadReplyRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7ReadReplyResp(transaction_id);
  }
};

class MM7DeliveryReportFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7DeliveryReport(transaction_id);
  }
};

class MM7DeliveryReportRespFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new MM7DeliveryReportResp(transaction_id);
  }
};

class GenericResponseFactory : public MmsFactory {
public:
  virtual MmsMsg * createMmsMsg(const string &transaction_id) {
    return new GenericResponse(transaction_id);
  }
};

}//mms
}//transport
}//scag

#endif
