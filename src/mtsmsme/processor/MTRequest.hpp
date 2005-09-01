#ifndef __SMSC_MTSMSME_PROCESSOR_MTREQUEST_HPP_
#define __SMSC_MTSMSME_PROCESSOR_MTREQUEST_HPP_

#include "Processor.h"

namespace smsc{namespace mtsmsme{namespace processor{
class TSM;
struct MTR: public Request {
  TSM* tsm;
  int invokeId;
  bool mms;
  MTR(TSM* _tsm){tsm = _tsm;}
  void setSendResult(int result);
};

}}}
#endif //__SMSC_MTSMSME_PROCESSOR_MTREQUEST_HPP_
