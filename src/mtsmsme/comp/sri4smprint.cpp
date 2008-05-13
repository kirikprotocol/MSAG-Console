static char const ident[] = "$Id$";
#include "mtsmsme/processor/TCO.hpp"
#include "logger/Logger.h"
#include "mtsmsme/processor/ACRepo.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"


//#include "util.hpp"
//#include "mtsmsme/processor/ACRepo.hpp"
//using std::vector;
//using smsc::mtsmsme::comp::UpdateLocationMessage;
//using smsc::mtsmsme::processor::util::dump;
//using smsc::mtsmsme::processor::TrId;
/* 
 * Example of SRI4SM
 *
 * 0000000    62 47 48 04 b9 11 00 00 6b 1e 28 1c 06 07 00 11
 * 0000020    86 05 01 01 01 a0 11 60 0f 80 02 07 80 a1 09 06
 * 0000040    07 04 00 00 01 00 14 02 6c 1f a1 1d 02 01 00 02
 * 0000060    01 2d 30 15 80 07 91 97 31 89 95 84 f9 81 01 ff
 * 0000100    82 07 91 97 31 89 96 99 f9
 *
 */

using smsc::mtsmsme::processor::TCO;
using smsc::mtsmsme::processor::TSM;
using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;
using smsc::logger::Logger;
using smsc::mtsmsme::processor::SccpSender;
TCO* mtsms = 0;
static Logger* logger = 0;

class SccpSenderImpl: public SccpSender {
  public:
    virtual void send(uint8_t cdlen,uint8_t *cd,uint8_t cllen,uint8_t *cl,uint16_t ulen,uint8_t *udp)
    {
      smsc_log_debug(logger, "fake sccp sender has pushed message to network");
    };
};
static SccpSenderImpl* sccpsender = 0;
using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;

int main(int argc, char* argv[])
{
    uint8_t cl[] = {2,2,2,2,2};
    uint8_t cd[] = {3,3,3,3,3};
    Logger::Init();
    logger = Logger::getInstance("smsc.mtsms");
    mtsms = new TCO(10,191);
    sccpsender = new SccpSenderImpl();
    mtsms->setSccpSender(sccpsender);
    TSM* tsm = 0;
    tsm = mtsms->TC_BEGIN(shortMsgGatewayContext_v2);
    if (tsm)
    {
      SendRoutingInfoForSMReq* inv = new SendRoutingInfoForSMReq("79139859489", true, "79139869999");
      tsm->TInvokeReq(45, *inv);
      tsm->TBeginReq(sizeof(cd)/sizeof(uint8_t), cd, sizeof(cl)/sizeof(uint8_t), cl);
    }
//  UpdateLocationMessage msg;
//  TrId id; id.size=4; id.buf[0] = 0x29; id.buf[1] = 0x00; id.buf[2] = 0x01; id.buf[3] = 0xB3;
//  msg.setOTID(id);
//  msg.setComponent("250130124323100","1979139860001","1979139860001");
//  vector<unsigned char> ulmsg;
//  msg.encode(ulmsg);
//  printf("UpdateLocation[%d]={%s}",ulmsg.size(),dump(ulmsg.size(),&ulmsg[0]).c_str());
  //smsc::mtsmsme::processor::SuperTestFunction();
}