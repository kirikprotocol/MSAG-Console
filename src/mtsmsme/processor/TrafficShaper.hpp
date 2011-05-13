#ident "$Id$"

#ifndef __SMSC_MTSMSME_EMULE_TRAFFICSHAPER_HPP_
#define __SMSC_MTSMSME_EMULE_TRAFFICSHAPER_HPP_

#include <inttypes.h>
#include "util/sleep.h"
#include "mtsmsme/processor/SccpSender.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

using smsc::mtsmsme::processor::SccpSender;

class TrafficShaper: public SccpSender {
  public:
    TrafficShaper(SccpSender* adaptee, int speed,int slowstartperiod);
    void send(uint8_t cdlen, uint8_t *cd, uint8_t cllen, uint8_t *cl, uint16_t ulen, uint8_t *udp);
  private:
    SccpSender* adaptee;
    int delay;
    int overdelay;
    hrtime_t msgstart;
    bool slowstartmode;
    int slowstartperiod; //in seconds
    int speed;
    struct timeval slow_start;
    void adjustdelay();
    void shape();
};
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
#endif __SMSC_MTSMSME_EMULE_TRAFFICSHAPER_HPP_
