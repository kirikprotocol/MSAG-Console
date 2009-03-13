#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_TSM_HPP_
#define __SMSC_MTSMSME_PROCESSOR_TSM_HPP_

#include "logger/Logger.h"
#include "mtsmsme/processor/Message.hpp"
#include "mtsmsme/processor/ACRepo.hpp"
#include "mtsmsme/comp/Component.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

using smsc::logger::Logger;
using smsc::mtsmsme::comp::CompIF;
using smsc::mtsmsme::processor::TrId;
using smsc::mtsmsme::processor::Message;
using smsc::mtsmsme::processor::AC;

class TCO;
class TsmComletionListener {
  public: virtual void complete(int status) = 0;
};
struct TSMSTAT {
  TSMSTAT():objcount(0),objcreated(0),objdeleted(0){}
  uint32_t objcount;
  uint32_t objcreated;
  uint32_t objdeleted;
};
class TSM
{
  public:
    enum STATE {
      IDLE = 0,
      IR = 1,
      ACTIVE = 2,
      STOP = 3
    };
  public:
    TSM(TrId _ltrid,AC& ac,TCO* _tco);
    virtual ~TSM();
    virtual void setCompletionListener(TsmComletionListener* listener);
    virtual void BEGIN(Message& msg);
    void BEGIN(
                        uint8_t laddrlen,
                        uint8_t *laddr,
                        uint8_t raddrlen,
                        uint8_t *raddr,
                        TrId rtrid,
                        Message& msg
                       );
    virtual void CONTINUE_received(uint8_t cdlen,
                                   uint8_t *cd, /* called party address */
                                   uint8_t cllen,
                                   uint8_t *cl, /* calling party address */
                                   Message& msg);

    virtual void END_received(Message& msg);
    virtual void TBeginReq(uint8_t  cdlen,
                           uint8_t* cd,        /* called party address */
                           uint8_t  cllen,
                           uint8_t* cl        /* calling party address */);
    virtual void TEndReq();

    virtual void TInvokeReq(int8_t invokeId, uint8_t opcode, CompIF& arg);
    virtual void TResultLReq(uint8_t invokeId, uint8_t opcode, CompIF& arg);
    static void getCounters(TSMSTAT&);
  protected:

    TrId ltrid;
    TrId rtrid;
    AC appcntx;
    TCO* tco;
    STATE st;
    uint8_t laddr[20];
    uint8_t laddrlen;
    uint8_t raddr[20];
    uint8_t raddrlen;
    Logger* logger;
    TsmComletionListener* listener;
  private:
    vector<unsigned char> internal_arg;
    uint8_t internal_opcode;
    uint8_t internal_invokeId;
    struct timeval start_ts;
    static uint32_t objcount;
    static uint32_t objcreated;
    static uint32_t objdeleted;
    uint32_t objnumber;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_TSM_HPP_
