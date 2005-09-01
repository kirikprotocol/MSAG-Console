#ifndef __SMSC_MTSMSME_PROCESSOR_TSM_HPP_
#define __SMSC_MTSMSME_PROCESSOR_TSM_HPP_

#include "Message.hpp"
//#include "TCO.hpp"
#include "MTRequest.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

class TCO;

class TSM
{
  enum STATE {
    IDLE = 0,
    IR = 1,
    ACTIVE = 2,
    STOP = 3
  };
  public:
    TSM(TrId _ltrid,AC& ac,TCO* _tco);
    ~TSM();
    void BEGIN_received(
                        uint8_t laddrlen,
                        uint8_t *laddr,
                        uint8_t raddrlen,
                        uint8_t *raddr,
                        TrId rtrid,
                        Message& msg
                       );
    void CONTINUE_received(Message& msg);
    void CONTINUE_Transaction()
    {
    }
    void END_Transaction()
    {
    }
    void sendResponse(int result, int invokeId);

  private:
    TrId ltrid;
    TrId rtrid;
    AC appcntx;
    TCO* tco;
    MTR req;
    STATE st;
    uint8_t laddr[20];
    uint8_t laddrlen;
    uint8_t raddr[20];
    uint8_t raddrlen;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_TSM_HPP_
