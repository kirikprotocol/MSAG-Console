#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_TSM_HPP_
#define __SMSC_MTSMSME_PROCESSOR_TSM_HPP_

#include "logger/Logger.h"
#include "mtsmsme/processor/Message.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

using smsc::logger::Logger;
class TCO;
class TsmComletionListener {
  public: virtual void complete(int status) = 0;
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
    virtual void BeginTransaction(TsmComletionListener* listener) = 0;
    virtual void BEGIN_received(
                        uint8_t laddrlen,
                        uint8_t *laddr,
                        uint8_t raddrlen,
                        uint8_t *raddr,
                        TrId rtrid,
                        Message& msg
                       ) = 0;
    virtual void CONTINUE_received(uint8_t cdlen,
                                   uint8_t *cd, /* called party address */
                                   uint8_t cllen,
                                   uint8_t *cl, /* calling party address */
                                   Message& msg) = 0;

    virtual void END_received(Message& msg) = 0;
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
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_TSM_HPP_
