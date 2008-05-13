#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_MTFTSM_HPP_
#define __SMSC_MTSMSME_PROCESSOR_MTFTSM_HPP_

#include "mtsmsme/processor/TSM.hpp"
//#include "mtsmsme/processor/Message.hpp"
#include "mtsmsme/processor/MTRequest.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

class MTFTSM: public TSM
{
  public:
    MTFTSM(TrId _ltrid,AC& ac,TCO* _tco);
    ~MTFTSM();
    void BEGIN_received(
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
    void CONTINUE_Transaction()
    {
    }
    void END_Transaction()
    {
    }
    void sendResponse(int result, int invokeId);

  private:
    MTR req;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_MTFTSM_HPP_
