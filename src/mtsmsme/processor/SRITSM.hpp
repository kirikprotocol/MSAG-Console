#ident "$Id$"

#ifndef __SMSC_MTSMSME_PROCESSOR_SRITSM_HPP_
#define __SMSC_MTSMSME_PROCESSOR_SRITSM_HPP_

#include "TSM.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

class SRITSM : public TSM
{
  public:
    SRITSM(TrId _ltrid,AC& ac,TCO* _tco);
    virtual ~SRITSM();
    virtual void END_received(Message& msg);
};

}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_SRITSM_HPP_
