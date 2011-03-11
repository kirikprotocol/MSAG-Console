#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_MOFTSM_HPP__
#define __SMSC_MTSMSME_PROCESSOR_MOFTSM_HPP__

#include "mtsmsme/processor/TSM.hpp"

namespace smsc{namespace mtsmsme{namespace processor{
class MOFTSM;
class MOFTSMListener {
  public:
    virtual void cont(MOFTSM* tsm,uint16_t arg) = 0;
    virtual void end(MOFTSM* tsm,uint16_t arg) = 0;
};
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
    void END_received(Message& msg);
    void sendResponse(int result, int invokeId);
    void addMOFTSMListener(MOFTSMListener* listener, uint16_t arg)
    {
      molistener = listener;
      molistener_arg = arg;
    }

  private:
    MOFTSMListener* molistener;
    uint16_t molistener_arg;
//    MTR req;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_MOFTSM_HPP__
