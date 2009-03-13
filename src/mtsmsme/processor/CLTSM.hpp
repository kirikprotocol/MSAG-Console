#ident "$Id$"

#ifndef __SMSC_MTSMSME_PROCESSOR_CLTSM_HPP_
#define __SMSC_MTSMSME_PROCESSOR_CLTSM_HPP_

#include "TSM.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

class CLTSM : public TSM
{
  public:
    CLTSM(TrId _ltrid,AC& ac,TCO* _tco);
    virtual ~CLTSM();
    virtual void BEGIN(Message& msg);
    virtual void CONTINUE_received(uint8_t cdlen,
                                   uint8_t *cd, /* called party address */
                                   uint8_t cllen,
                                   uint8_t *cl, /* calling party address */
                                   Message& msg);
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_CLTSM_HPP_
