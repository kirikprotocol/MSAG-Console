#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_MOFTSM_HPP__
#define __SMSC_MTSMSME_PROCESSOR_MOFTSM_HPP__

#include "mtsmsme/processor/TSM.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

class MOFTSM: public TSM
{
  public:
    MOFTSM(TrId _ltrid,AC& ac,TCO* _tco);
    virtual ~MOFTSM();
    virtual void BEGIN(Message& msg);
    virtual void CONTINUE_received(uint8_t cdlen,
                                   uint8_t *cd, /* called party address */
                                   uint8_t cllen,
                                   uint8_t *cl, /* calling party address */
                                   Message& msg);

    void CONTINUE_Transaction() {}
    void END_Transaction() {}
    void sendResponse(int result, int invokeId);

  private:
//    MTR req;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_MOFTSM_HPP__
