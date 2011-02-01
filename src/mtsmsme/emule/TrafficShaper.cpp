static char const ident[] = "$Id$";

#include "mtsmsme/emule/TrafficShaper.hpp"

namespace smsc{namespace mtsmsme{namespace processor{

using smsc::mtsmsme::processor::TrafficShaper;
using smsc::util::millisleep;

    void TrafficShaper::adjustdelay()
    {
      if (slowstartmode)
      {
        timeval now;
        if (!slow_start.tv_sec)
          gettimeofday(&slow_start,NULL);
        gettimeofday(&now,NULL);
        if (slow_start.tv_sec + slowstartperiod < now.tv_sec)
        {
          slowstartmode = false;
          delay = 1000000/speed;
        }
        else
        {
          delay = 1000000/(1+(speed-1)*(now.tv_sec-slow_start.tv_sec)/slowstartperiod);
        }
      }
    }
    void TrafficShaper::shape()
    {
      adjustdelay();
      hrtime_t msgproc=gethrtime()-msgstart;
      msgproc/=1000;
      if(delay>msgproc+overdelay)
      {
        int toSleep=delay-msgproc-overdelay;
        msgstart=gethrtime();
        millisleep(toSleep/1000);
        overdelay=(gethrtime()-msgstart)/1000-toSleep;
      }else
      {
        overdelay-=delay-(int)msgproc;
      }
    }
    TrafficShaper::TrafficShaper(SccpSender* _adaptee, int _speed,int _slowstartperiod) :
      adaptee(_adaptee),slowstartmode(false),slowstartperiod(_slowstartperiod),
      overdelay(0)
    {
      if (slowstartperiod) slowstartmode = true;
      slow_start.tv_sec = 0;
      delay = 1000000/_speed;
      speed = _speed;
    }
    void TrafficShaper::send(uint8_t cdlen, uint8_t *cd, uint8_t cllen, uint8_t *cl,
        uint16_t ulen, uint8_t *udp)
    {
      msgstart=gethrtime();
      adaptee->send(cdlen,cd,cllen,cl,ulen,udp);
      shape();
    }
}/*namespace processor*/}/*namespace mtsmsme*/}/*namespace smsc*/
