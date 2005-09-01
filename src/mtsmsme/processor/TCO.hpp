#ifndef __SMSC_MTSMSME_PROCESSOR_TCO_HPP_
#define __SMSC_MTSMSME_PROCESSOR_TCO_HPP_

#include <sys/types.h>
#include "core/buffers/XHash.hpp"
#include <list>
#include "Processor.h"
#include "decode.hpp"
#include "encode.hpp"
#include "Message.hpp"
#include "TSM.hpp"

namespace smsc{namespace mtsmsme{namespace processor{
/*
 * Transaction sublayer facade
 */
using namespace smsc::mtsmsme::processor::decode;
using namespace smsc::mtsmsme::processor::encode;
using std::list;

class TSM;

struct TrIdHash{
  static inline unsigned int CalcHash(TrId id){
    return ((id.buf[3]<<24)+(id.buf[2]<<16)+(id.buf[1]<<8)+(id.buf[0]<<0));
  }
};

class TCO
{
  public:
    TCO(int TrLimit);
    ~TCO();
    void NUNITDATA(uint8_t cdlen,
                   uint8_t *cd, /* called party address */
                   uint8_t cllen,
                   uint8_t *cl, /* calling party address */
                   uint16_t ulen,
                   uint8_t *udp /* user data */
                  );
    void TR_CONTINUE(TrId trid);
    void setRequestSender(RequestSender* sender);
    void SCCPsend(uint8_t cdlen,
                  uint8_t *cd,
                  uint8_t cllen,
                  uint8_t *cl,
                  uint16_t ulen,
                  uint8_t *udp);
    void TSMStopped(TrId);
    Decoder decoder;
    Encoder encoder;
    RequestSender *sender;
  private:
    XHash<TrId,TSM*,TrIdHash> tsms;
    list<TrId> tridpool;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_TCO_HPP_
