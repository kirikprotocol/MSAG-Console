#ident "$Id$"

#ifndef __SMSC_MTSMSME_PROCESSOR_LUTSM_HPP_
#define __SMSC_MTSMSME_PROCESSOR_LUTSM_HPP_

#include "TSM.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

class ULTSM : public TSM
{
  public:
    ULTSM(TrId _ltrid,AC& ac,TCO* _tco);
    ULTSM(TrId _ltrid,AC& ac,TCO* _tco, const char* _imsi, const char* _msc,const char* _vlr, const char* _mgt);
    virtual ~ULTSM();
    virtual void BEGIN(Message& msg);
    virtual void CONTINUE_received(uint8_t cdlen,
                                   uint8_t *cd, /* called party address */
                                   uint8_t cllen,
                                   uint8_t *cl, /* calling party address */
                                   Message& msg);

    virtual void END_received(Message& msg);
    virtual void TInvokeReq(int8_t invokeId, uint8_t opcode, CompIF& arg);
    virtual void TBeginReq(uint8_t  cdlen,
                           uint8_t* cd,        /* called party address */
                           uint8_t  cllen,
                           uint8_t* cl        /* calling party address */);
    virtual void expiredwdtimer(uint32_t secret);
  private:
    //vector<unsigned char> ulmsg;
    vector<unsigned char> temp_arg;
    uint8_t temp_opcode;
    uint8_t temp_invokeId;
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_LUTSM_HPP_
