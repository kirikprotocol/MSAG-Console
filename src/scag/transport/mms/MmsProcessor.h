#ifndef SCAG_TRANSPORT_MMS_PROCESSOR
#define SCAG_TRANSPORT_MMS_PROCESSOR

#include <string>

#include "MmsCommand.h"
#include "MmsContext.h"

namespace scag { namespace transport { namespace mms {

class MmsProcessor {
public:
  virtual int processRequest(MmsRequest &request) = 0;
  virtual int processResponse(MmsResponse &response) = 0;
  virtual int statusResponse(MmsResponse &response, bool delivered = true) = 0;
  static MmsProcessor& Instance();
  static void Init(const string& cfg);
protected:
  virtual ~MmsProcessor() {};
};

}//mms
}//transport
}//scag

#endif
