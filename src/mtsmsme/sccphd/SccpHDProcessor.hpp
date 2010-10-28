#ident "$Id$"
#ifndef __SMSC_MTSMS_SCCPHDPROC_HPP__
#define __SMSC_MTSMS_SCCPHDPROC_HPP__

#include "mtsmsme/processor/Processor.h"
#include "mtsmsme/processor/HLRImpl.hpp"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/processor/SccpSender.hpp"

namespace smsc {
namespace mtsmsme {
namespace processor {

class SccpHDProcessor : public RequestProcessor,SccpSender {
  public:
    virtual void setRequestSender(RequestSender* sender);
    void configure(int user_id, int ssn, Address& msc, Address& vlr,
        Address& hlr);
    void configure(int user_id, int ssn,char* cpmgr, char* instlist,
        Address& msc, Address& vlr, Address& hlr);
    virtual void send(uint8_t cdlen,uint8_t *cd,uint8_t cllen,uint8_t *cl,uint16_t ulen,uint8_t *udp);
    virtual HLROAM* getHLROAM();
    virtual int Run();
    virtual void Stop();
    TCO* getCoordinator();
    SccpHDProcessor();
    SccpHDProcessor(TCO* _coordinator, SubscriberRegistrator* _registrator);
    ~SccpHDProcessor();
  private:
    RequestSender* sender;
    TCO* coordinator;
    SubscriberRegistrator* registrator;
};

}//namespace pr
}//namespace mtsmsme
}//namespace smsc
#endif
