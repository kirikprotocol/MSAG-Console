#ident "$Id$"

#ifndef __SMSC_MTSMSME_PROCESSOR_PRNTSM_HPP_
#define __SMSC_MTSMSME_PROCESSOR_PRNTSM_HPP_

#include "TSM.hpp"
#include "Component.h"

namespace smsc{namespace mtsmsme{namespace processor{

class PRNTSM : public TSM
{
  public:
    PRNTSM(TrId _ltrid,AC& ac,TCO* _tco);
    ~PRNTSM();
    virtual void BEGIN_received(
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
  private:
    vector<unsigned char> temp_arg;
    uint8_t temp_opcode;
    uint8_t temp_invokeId;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_PRNTSM_HPP_
