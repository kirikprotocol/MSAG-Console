#ident "$Id$"
#ifndef __SMSC_MTSMSME_PROCESSOR_SCCPSENDER_HPP__
#define __SMSC_MTSMSME_PROCESSOR_SCCPSENDER_HPP__

namespace smsc{namespace mtsmsme{namespace processor{

class SccpSender {
  public:
    virtual void send(uint8_t cdlen,uint8_t *cd,uint8_t cllen,uint8_t *cl,uint16_t ulen,uint8_t *udp) = 0;
};

} /*namespace processor*/ } /*namespace mtsmsme*/ } /*namespace smsc*/
#endif //__SMSC_MTSMSME_PROCESSOR_SCCPSENDER_HPP__
