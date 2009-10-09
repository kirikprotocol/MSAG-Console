static char const ident[] = "$Id$";
#include "mtsmsme/comp/tests/tst2.hpp"
CPPUNIT_TEST_SUITE_REGISTRATION(AmericaTestFixture);
void psevdomain();
void AmericaTestFixture::AmericaTest()
{
  psevdomain();
  CPPUNIT_ASSERT(true);
}
#include "mtsmsme/comp/UpdateLocation.hpp"
void AmericaTestFixture::ulprint()
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
    0x07, 0x04, 0x56, 0x00, 0x01, 0x00, 0x01, 0x03,
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
  UpdateLocationMessage msg;
  TrId id; id.size=4; id.buf[0] = 0x29; id.buf[1] = 0x00; id.buf[2] = 0x01; id.buf[3] = 0xB3;
  msg.setOTID(id);
  msg.setComponent("250130124323100","1979139860001","1979139860001");
  vector<unsigned char> ulmsg;
  msg.encode(ulmsg);
  vector<unsigned char> etalon_buf(etalon, etalon + sizeof(etalon) / sizeof(unsigned char) );
  vector<unsigned char> bad_buf(bad, bad + sizeof(bad) / sizeof(unsigned char) );
  CPPUNIT_ASSERT(etalon_buf == ulmsg);
  CPPUNIT_ASSERT(bad_buf == ulmsg);
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
#include "logger/Logger.h"
#include "sms/sms.h"
using smsc::mtsmsme::processor::SccpSender;
class SccpSenderMImpl: public SccpSender {
  public:
    SccpSenderMImpl(){}
    virtual void send(uint8_t cdlen, uint8_t *cd,
                      uint8_t cllen, uint8_t *cl,
                      uint16_t ulen, uint8_t *udp){}
};
void psevdomain()
{
  using smsc::mtsmsme::processor::TCO;
  using smsc::logger::Logger;
  using smsc::sms::Address;

  Logger::Init(); Logger* logger = Logger::getInstance("all");
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

  SccpSender* sccpsender = new SccpSenderMImpl();
  mtsms.setSccpSender(sccpsender);

  mtsms.NUNITDATA((uint8_t) (sizeof(cd)/sizeof(uint8_t)), cd,
                  (uint8_t) (sizeof(cl)/sizeof(uint8_t)), cl,
                  (uint8_t) (sizeof(ud)/sizeof(uint8_t)), ud);
}
