#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_MTREQUEST_HPP_
#define __SMSC_MTSMSME_PROCESSOR_MTREQUEST_HPP_

#include "mtsmsme/processor/Processor.h"

namespace smsc{namespace mtsmsme{namespace processor{
class MTFTSM;
struct MTR: public Request {
  MTFTSM* tsm;
  int invokeId;
  bool mms;
  char dstmsc[20];
  MTR(MTFTSM* _tsm){tsm = _tsm;}
  void setSendResult(int result);
};

}}}
#endif //__SMSC_MTSMSME_PROCESSOR_MTREQUEST_HPP_
