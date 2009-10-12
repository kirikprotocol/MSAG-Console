static char const ident[] = "$Id$";
#include "mtsmsme/comp/tests/tst2.hpp"
CPPUNIT_TEST_SUITE_REGISTRATION(AmericaTestFixture);
void AmericaTestFixture::SccpSenderImpl::send(uint8_t cdlen,uint8_t *cd,uint8_t cllen,uint8_t *cl,uint16_t ulen,uint8_t *udp)
{
        smsc_log_debug(AmericaTestFixture::logger, "fake sccp sender has pushed message to network");
}
void AmericaTestFixture::setUp()
{
  using smsc::logger::Logger;
  Logger::Init();
  logger = Logger::getInstance("all");
  sender = new SccpSenderImpl();
}
void AmericaTestFixture::tearDown()
{
  using smsc::logger::Logger;
  delete(sender);
  Logger::Shutdown();
}
#include "mtsmsme/comp/UpdateLocation.hpp"
#include "mtsmsme/processor/util.hpp"
void AmericaTestFixture::updateLocation_arg_encoding()
{
  unsigned char etalon[] = {
    0x62, 0x56, 0x48, 0x04, 0x29, 0x00, 0x01, 0xB3,
    0x6B, 0x1E, 0x28, 0x1C, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xA0, 0x11, 0x60,
    0x0F, 0x80, 0x02, 0x07, 0x80, 0xA1, 0x09, 0x06,
    0x07, 0x04, 0x00, 0x00, 0x01, 0x00, 0x01, 0x03,
    0x6C, 0x2E, 0xA1, 0x2C, 0x02, 0x01, 0x01, 0x02,
    0x01, 0x02, 0x30, 0x24, 0x04, 0x08, 0x52, 0x10,
    0x03, 0x21, 0x34, 0x32, 0x01, 0xF0, 0x81, 0x08,
    0x91, 0x91, 0x97, 0x31, 0x89, 0x06, 0x00, 0xF1,
    0x04, 0x08, 0x91, 0x91, 0x97, 0x31, 0x89, 0x06,
    0x00, 0xF1, 0xA6, 0x04, 0x80, 0x02, 0x05, 0x80
  };
  unsigned char bad[] = {
    0x62, 0x56, 0x48, 0x04, 0x29, 0x00, 0x01, 0xB3,
    0x6B, 0x1E, 0x28, 0x1C, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xA0, 0x11, 0x60,
    0x0F, 0x80, 0x02, 0x07, 0x80, 0xA1, 0x09, 0x06,
    0x07, 0x04, 0x56 /* bad */, 0x00, 0x01, 0x00, 0x01, 0x03,
    0x6C, 0x2E, 0xA1, 0x2C, 0x02, 0x01, 0x01, 0x02,
    0x01, 0x02, 0x30, 0x24, 0x04, 0x08, 0x52, 0x10,
    0x03, 0x21, 0x34, 0x32, 0x01, 0xF0, 0x81, 0x08,
    0x91, 0x91, 0x97, 0x31, 0x89, 0x06, 0x00, 0xF1,
    0x04, 0x08, 0x91, 0x91, 0x97, 0x31, 0x89, 0x06,
    0x00, 0xF1, 0xA6, 0x04, 0x80, 0x02, 0x05, 0x80
  };
  using std::vector;
  using smsc::mtsmsme::comp::UpdateLocationMessage;
  using smsc::mtsmsme::processor::TrId;
  using smsc::mtsmsme::processor::util::dump;
  UpdateLocationMessage msg;
  TrId id; id.size=4; id.buf[0] = 0x29; id.buf[1] = 0x00; id.buf[2] = 0x01; id.buf[3] = 0xB3;
  msg.setOTID(id);
  msg.setComponent("250130124323100","1979139860001","1979139860001");
  vector<unsigned char> ulmsg;
  msg.encode(ulmsg);
  //printf("UpdateLocation[%d]={%s}",ulmsg.size(),dump((uint16_t)ulmsg.size(),&ulmsg[0]).c_str());
  vector<unsigned char> etalon_buf(etalon, etalon + sizeof(etalon) / sizeof(unsigned char) );
  vector<unsigned char> bad_buf(bad, bad + sizeof(bad) / sizeof(unsigned char) );
  CPPUNIT_ASSERT(etalon_buf == ulmsg);
  CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( bad_buf == ulmsg ) );
}
#include "mtsmsme/comp/ReportSmDeliveryStatus.hpp"
void AmericaTestFixture::reportSMDeliveryStatus_arg_decoding(void)
{
  uint8_t ind_encoded[] = {
    0xA1, 0x1D, 0x02, 0x01, 0x01, 0x02, 0x01, 0x2F,
    0x30, 0x15, 0x04, 0x07, 0x91, 0x97, 0x58, 0x81,
    0x55, 0x01, 0xF0, 0x04, 0x07, 0x91, 0x81, 0x67,
    0x83, 0x00, 0x51, 0xF4, 0x0A, 0x01, 0x02
  };
  using smsc::mtsmsme::comp::ReportSmDeliveryStatusInd;
  ReportSmDeliveryStatusInd ind(logger);
  ind.decode(ind_encoded);
  //todo add some validations
}
  /*
   * After this message application crashes
   * ======================================================
   * TCO::NUNITDATA
   * Cd(GTRC SSN=191 GT=(TT=0,NP=1,NA=4,AI=79851855100))
   * Cl(GTRC SSN=8   GT=(TT=0,NP=1,NA=4,AI=187638001540))
   * data[73]={
   * 62 47 48 04 0A 80 37 5A 6B 1E 28 1C 06 07 00 11
   * 86 05 01 01 01 A0 11 60 0F 80 02 07 80 A1 09 06
   * 07 04 00 00 01 00 14 02 6C 1F A1 1D 02 01 01 02
   * 01 2F 30 15 04 07 91 97 58 81 55 01 F0 04 07 91
   * 81 67 83 00 51 F4 0A 01 02
   * }
   * ======================================================
   * Cd GT not equals to VLR or MSC, assuming SSN = 6,
   * new Cd(GTRC SSN=6 GT=(TT=0,NP=1,NA=4,AI=79851855100))
   * ======================================================
   * SRI4SMTSM's modified Cd(GTRC SSN=6 GT=(TT=0,NP=1,NA=4,AI=79168960237))
   * ======================================================
   * SendRoutingInfoForSMInd::decode consumes 2/23 and returns code 2
   * ======================================================
   * D 19-09 19:41:37,424 001 mt.sme.sri: SendRoutingInfoForSMInd
   * RoutingInfoForSM-Arg ::= {
   *    msisdn:
   *    sm-RP-PRI: FALSE
   *    serviceCentreAddress:
   * }
   * msisdn=
   * ASSERT*</home/smsx/smsc/src/sms/sms.h(setValue):205>
   * assertin _len && _value && _value[0] && _len<sizeof(AddressValue) failed
   */
#include "mtsmsme/processor/TCO.hpp"
#include "sms/sms.h"
void AmericaTestFixture::reportSMDeliveryStatus_receiving()
{
  using smsc::mtsmsme::processor::TCO;
  using smsc::sms::Address;

  uint8_t cd[] = { 2, 2, 2, 2, 2 };
  uint8_t cl[] = { 2, 2, 2, 2, 2 };
  uint8_t ud[] = {
    0x62, 0x47, 0x48, 0x04, 0x0A, 0x80, 0x37, 0x5A,
    0x6B, 0x1E, 0x28, 0x1C, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xA0, 0x11, 0x60,
    0x0F, 0x80, 0x02, 0x07, 0x80, 0xA1, 0x09, 0x06,
    0x07, 0x04, 0x00, 0x00, 0x01, 0x00, 0x14, 0x02,
    0x6C, 0x1F, 0xA1, 0x1D, 0x02, 0x01, 0x01, 0x02,
    0x01, 0x2F, 0x30, 0x15, 0x04, 0x07, 0x91, 0x97,
    0x58, 0x81, 0x55, 0x01, 0xF0, 0x04, 0x07, 0x91,
    0x81, 0x67, 0x83, 0x00, 0x51, 0xF4, 0x0A, 0x01,
    0x02
  };
  char msc[] = "791398600223"; // MSC address
  char vlr[] = "79139860004"; //VLR address
  char hlr[] = "79139860004"; //HLR address

  TCO mtsms(10);
  mtsms.setAdresses(Address((uint8_t)strlen(msc), 1, 1, msc),
                    Address((uint8_t)strlen(vlr), 1, 1, vlr),
                    Address((uint8_t)strlen(hlr), 1, 1, hlr));

  //SccpSender* sccpsender = new SccpSenderMImpl();
  mtsms.setSccpSender(sender);

  mtsms.NUNITDATA((uint8_t) (sizeof(cd)/sizeof(uint8_t)), cd,
                  (uint8_t) (sizeof(cl)/sizeof(uint8_t)), cl,
                  (uint8_t) (sizeof(ud)/sizeof(uint8_t)), ud);
  CPPUNIT_ASSERT(true);
}
#include "mtsmsme/processor/ACRepo.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
void AmericaTestFixture::sendRoutingInfoForSM_sending()
{
  /*
   * D 12-10 14:44:59,580 001 mt.sme.sri: tsm otid=BABE0001 create SendRoutingInfoForSM
   * D 12-10 14:44:59,582 001 mt.sme.sri: CALLED[5]={03 03 03 03 03 }
   * D 12-10 14:44:59,582 001 mt.sme.sri: CALLING[5]={02 02 02 02 02 }
   * D 12-10 14:44:59,582 001 mt.sme.sri: SRI4SM[73]={62 47 48 04 BA BE 00 01 6B 1E 28 1C 06 07 00 11 86 05 01 01 01 A0 11 60 0F 80 02 07 80 A1 09 06 07 04 00 00 01 00 14 02 6C 1F A1 1D 02 01 01 02 01 2D 30 15 80 07 91 97 31 89 95 84 F9 81 01 FF 82 07 91 97 31 89 96 99 F9 }
   * D 12-10 14:44:59,582 001 mt.sme.tco: void TCO::SCCPsend: cd=GTRC SSN=3 don't know how to decode, cl=GTRC SSN=2 don't know how to decode
   */
  using smsc::mtsmsme::processor::TCO;
  using smsc::mtsmsme::processor::TSM;
  using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;
  using smsc::logger::Logger;

  using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;

  uint8_t cl[] = { 2, 2, 2, 2, 2 };
  uint8_t cd[] = { 3, 3, 3, 3, 3 };
  TCO mtsms(10);
  //SccpSender* sccpsender = new SccpSenderImpl();
  mtsms.setSccpSender(sender);
  TSM* tsm = 0;
  tsm = mtsms.TC_BEGIN(shortMsgGatewayContext_v2);
  if (tsm)
  {
    SendRoutingInfoForSMReq* inv = new SendRoutingInfoForSMReq("79139859489", true, "79139869999");
    tsm->TInvokeReq(1, 45, *inv);
    tsm->TBeginReq((uint8_t) (sizeof(cd) / sizeof(uint8_t)), cd,
        (uint8_t) (sizeof(cl) / sizeof(uint8_t)), cl);
  }
}
