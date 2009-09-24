static char const ident[] = "$Id$";
#include "mtsmsme/processor/TCO.hpp"
#include "logger/Logger.h"
#include "mtsmsme/processor/ACRepo.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"


using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;
using smsc::logger::Logger;
using smsc::mtsmsme::processor::SccpSender;
static Logger* logger = 0;

class SccpSenderImpl: public SccpSender {
  public:
    virtual void send(uint8_t cdlen,uint8_t *cd,uint8_t cllen,uint8_t *cl,uint16_t ulen,uint8_t *udp)
    {
      smsc_log_debug(logger, "fake sccp sender has pushed message to network");
    };
};
using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;

int main(int argc, char* argv[])
{
    uint8_t cl[] = {2,2,2,2,2};
    uint8_t cd[] = {3,3,3,3,3};
    Logger::Init();
    logger = Logger::getInstance("smsc.mtsms");
    TCO* mtsms = new TCO(10);
    SccpSender* sccpsender = new SccpSenderImpl();
    mtsms->setSccpSender(sccpsender);
    TSM* tsm = 0;
    tsm = mtsms->TC_BEGIN(shortMsgGatewayContext_v2);
    if (tsm)
    {
      SendRoutingInfoForSMReq* inv = new SendRoutingInfoForSMReq("79139859489", true, "79139869999");
      tsm->TInvokeReq(1, 45, *inv);
      tsm->TBeginReq((uint8_t)(sizeof(cd)/sizeof(uint8_t)), cd, (uint8_t)(sizeof(cl)/sizeof(uint8_t)), cl);
    }
}
