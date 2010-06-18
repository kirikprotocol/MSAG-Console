#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_TCO_HPP_
#define __SMSC_MTSMSME_PROCESSOR_TCO_HPP_

#include <sys/types.h>
#include <list>
#include "core/buffers/XHash.hpp"
#include "core/synchronization/Mutex.hpp"
#include "mtsmsme/processor/SccpSender.hpp"
#include "mtsmsme/processor/Processor.h"
#include "mtsmsme/processor/encode.hpp"
#include "mtsmsme/processor/TSM.hpp"

namespace smsc{namespace mtsmsme{namespace processor{
/*
 * Transaction sublayer facade
 */
using namespace smsc::mtsmsme::processor::encode;
using smsc::core::synchronization::Mutex;
using smsc::mtsmsme::processor::SccpSender;
using std::list;

class TSM;
class SccpUser {
  public:
    virtual void NUNITDATA(
        uint8_t cdlen, uint8_t *cd, /* called party address  */
        uint8_t cllen, uint8_t *cl, /* calling party address */
        uint16_t ulen, uint8_t *udp /* user data             */) = 0;
    virtual void NNOTICE(
        uint8_t cdlen, uint8_t *cd, /* called party address  */
        uint8_t cllen, uint8_t *cl, /* calling party address */
        uint16_t ulen, uint8_t *udp /* user data             */) = 0;
};
struct TrIdHash{
  static inline unsigned int CalcHash(TrId id){
    return ((id.buf[3]<<24)+(id.buf[2]<<16)+(id.buf[1]<<8)+(id.buf[0]<<0));
  }
};

class TCO: public SccpUser
{
  public:
    TCO(int TrLimit);
    ~TCO();
    TSM* TC_BEGIN(AC& appcntx);
    void NUNITDATA(
        uint8_t cdlen, uint8_t *cd, /* called party address  */
        uint8_t cllen, uint8_t *cl, /* calling party address */
        uint16_t ulen, uint8_t *udp /* user data             */);
    virtual void NNOTICE(
        uint8_t cdlen, uint8_t *cd, /* called party address  */
        uint8_t cllen, uint8_t *cl, /* calling party address */
        uint16_t ulen, uint8_t *udp /* user data             */);
    void setRequestSender(RequestSender* sender);
    void setSccpSender(SccpSender* sender);
    void SCCPsend(uint8_t cdlen, uint8_t *cd,
                  uint8_t cllen, uint8_t *cl,
                  uint16_t ulen, uint8_t *udp);
    void TSMStopped(TrId);
    void setAdresses(Address& msc, Address& vlr, Address& hlr);
    void setHLROAM(HLROAM* hlr);
    void dlgcleanup();
    void startwdtimer(int seconds,TrId ltrid, uint32_t secret);
    HLROAM* getHLROAM();
    Encoder encoder;
    RequestSender *sender;
    AddressValue hlrnumber;
    AddressValue mscnumber;
    list<TrId> tridpool;
  private:
    AddressValue vlrnumber;
    void fixCalledAddress(uint8_t cdlen, uint8_t *cd);
    XHash<TrId,TSM*,TrIdHash> tsms;
    Mutex tridpool_mutex;
    HLROAM* hlr;
    SccpSender *sccpsender = 0;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_TCO_HPP_
