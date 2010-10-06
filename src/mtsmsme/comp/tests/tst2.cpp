static char const ident[] = "$Id$";
#include "mtsmsme/comp/tests/tst2.hpp"
#include "mtsmsme/processor/SccpSender.hpp"
#include "mtsmsme/processor/util.hpp"
#include "mtsmsme/processor/TSM.hpp"
#include "mtsmsme/processor/ACRepo.hpp"
#include "mtsmsme/processor/Processor.h"
#include "mtsmsme/processor/TCO.hpp"
#include "mtsmsme/comp/SendRoutingInfoForSM.hpp"
#include "mtsmsme/comp/ReportSmDeliveryStatus.hpp"
#include "mtsmsme/comp/UpdateLocation.hpp"
#include "mtsmsme/comp/SendRoutingInfo.hpp"
#include "sms/sms.h"
#include <string>
CPPUNIT_TEST_SUITE_REGISTRATION(AmericaTestFixture);

using smsc::mtsmsme::processor::SccpSender;
using smsc::mtsmsme::processor::TsmComletionListener;
using smsc::mtsmsme::processor::RequestSender;
using smsc::mtsmsme::processor::Request;
using std::vector;
using smsc::logger::Logger;
using smsc::mtsmsme::processor::util::dump;
class TsmComletionListenerMock: public TsmComletionListener {
  private:
    Logger* logger;
    bool pending;
  public:
    TsmComletionListenerMock(Logger* _logger):logger(_logger),pending(true){}
    virtual void complete(int status)
    {
      smsc_log_debug(logger,
           "COMPLETED UPDATELOCATION status=%d",status);
      pending = false;
    }
    bool isCompleted() {return !pending;}
};
class SccpSenderMock: public SccpSender {
  private:
    Logger* logger;
    vector<unsigned char>& buffer;
  public:
    SccpSenderMock(Logger* _logger, vector<unsigned char>& _buffer)
      :logger(_logger),buffer(_buffer){}
      virtual void send(uint8_t cdlen, uint8_t *cd,
                        uint8_t cllen, uint8_t *cl,
                        uint16_t ulen, uint8_t *udp)
      {
        //std::vector<unsigned char> *stream = (std::vector<unsigned char> *)app_key;
        //  unsigned char *buf = (unsigned char *)buffer;
        //  stream->insert(stream->end(),buf, buf + size);
        buffer.insert(buffer.end(), udp, udp + ulen);
        smsc_log_debug(logger,
                       "intercepted message to network data[%d]={%s}",
                       buffer.size(),
                       dump((uint16_t)buffer.size(),&buffer[0]).c_str());
      }
  };
class SlowRequestSender: public RequestSender {
  public:
    Request* req_ptr;
  SlowRequestSender():req_ptr(0){}
  //simulate 1043 to stop TSM and then set status
  void sendTimeoute() { req_ptr->setSendResult(1143); }
  void sendOK() { req_ptr->setSendResult(0); }
  virtual bool send(Request* request)
  {
    //just store Request and let TSM to die
    req_ptr = request;
    return true;
  }
};
void AmericaTestFixture::setUp()
{
  Logger::Init();
  logger = Logger::getInstance("testrunner");
}
void AmericaTestFixture::tearDown()
{
  Logger::Shutdown();
}
void AmericaTestFixture::updateLocation_arg_encoding()
{
  smsc_log_debug(logger, "======== AmericaTestFixture::updateLocation_arg_encoding ========\n");
  unsigned char etalon[] = {
    0x62, 0x54, 0x48, 0x04, 0xba, 0xbe, 0x00, 0x16,
    0x6b, 0x1e, 0x28, 0x1c, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xa0, 0x11, 0x60,
    0x0f, 0x80, 0x02, 0x07, 0x80, 0xa1, 0x09, 0x06,
    0x07, 0x04, 0x00, 0x00, 0x01, 0x00, 0x01, 0x02,
    0x6c, 0x2c, 0xa1, 0x2a, 0x02, 0x01, 0x01, 0x02,
    0x01, 0x02, 0x30, 0x22, 0x04, 0x08, 0x72, 0x02,
    0x71, 0x10, 0x51, 0x12, 0x96, 0xf1, 0x81, 0x07,
    0x91, 0x97, 0x31, 0x89, 0x96, 0x89, 0x21, 0x04,
    0x07, 0x91, 0x97, 0x31, 0x89, 0x96, 0x89, 0x31,
    0xa6, 0x04, 0x80, 0x02, 0x05, 0x80
  };
  unsigned char bad[] = {
    0x62, 0x54, 0x48, 0x04, 0xba, 0xbe, 0x00, 0x16,
    0x6b, 0x1e, 0x28, 0x1c, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xa0, 0x11, 0x60,
    0x0f, 0x80, 0x02, 0x07, 0x80, 0xa1, 0x09, 0x06,
    0x07, 0x04, 0x56 /* bad */, 0x00, 0x01, 0x00, 0x01, 0x02,
    0x6c, 0x2c, 0xa1, 0x2a, 0x02, 0x01, 0x01, 0x02,
    0x01, 0x02, 0x30, 0x22, 0x04, 0x08, 0x72, 0x02,
    0x71, 0x10, 0x51, 0x12, 0x96, 0xf1, 0x81, 0x07,
    0x91, 0x97, 0x31, 0x89, 0x96, 0x89, 0x21, 0x04,
    0x07, 0x91, 0x97, 0x31, 0x89, 0x96, 0x89, 0x31,
    0xa6, 0x04, 0x80, 0x02, 0x05, 0x80
  };
  using smsc::mtsmsme::comp::UpdateLocationMessage;
  using smsc::mtsmsme::processor::TrId;
  using smsc::mtsmsme::processor::util::dump;
  UpdateLocationMessage msg;
  TrId id; id.size=4; id.buf[0] = 0xba; id.buf[1] = 0xbe; id.buf[2] = 0x00; id.buf[3] = 0x16;
  msg.setOTID(id);
  msg.setComponent("272017011521691","791398699812","791398699813");
  vector<unsigned char> ulmsg;
  msg.encode(ulmsg);
  //printf("UpdateLocation[%d]={%s}",ulmsg.size(),dump((uint16_t)ulmsg.size(),&ulmsg[0]).c_str());
  vector<unsigned char> etalon_buf(etalon, etalon + sizeof(etalon) / sizeof(unsigned char) );
  vector<unsigned char> bad_buf(bad, bad + sizeof(bad) / sizeof(unsigned char) );

  using smsc::mtsmsme::comp::UpdateLocationReq;
  using smsc::mtsmsme::processor::net_loc_upd_v2;
  using smsc::mtsmsme::processor::BeginMsg;
  UpdateLocationReq ulreq;
  ulreq.setParameters("272017011521691","791398699812","791398699813");
  vector<unsigned char> temp_arg;
  ulreq.encode(temp_arg);

  TrId otid; otid.size=4; otid.buf[0] = 0xba; otid.buf[1] = 0xbe; otid.buf[2] = 0x00; otid.buf[3] = 0x16;
  BeginMsg begin;
  begin.setOTID(otid);
  begin.setDialog(net_loc_upd_v2);
  begin.setInvokeReq(1 /* invokeId */, 2 /* updateLocation operation */, temp_arg);
  vector<unsigned char> data;
  begin.encode(data);
  smsc_log_debug(logger,"etalon data[%d]={%s}",
                         sizeof(etalon),
                         dump((uint16_t)sizeof(etalon),etalon).c_str());
  smsc_log_debug(logger,"old type API encoded data[%d]={%s}",
                         ulmsg.size(),
                         dump((uint16_t)ulmsg.size(),&ulmsg[0]).c_str());
  smsc_log_debug(logger,"new type API encoded data[%d]={%s}",
                         data.size(),
                         dump((uint16_t)data.size(),&data[0]).c_str());

  CPPUNIT_ASSERT(etalon_buf == ulmsg);
//  CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( bad_buf == ulmsg ) );
  CPPUNIT_ASSERT( bad_buf != ulmsg );
  CPPUNIT_ASSERT(etalon_buf == data);
}
void AmericaTestFixture::reportSMDeliveryStatus_arg_decoding(void)
{
  smsc_log_debug(logger, "======== AmericaTestFixture::reportSMDeliveryStatus_arg_decoding ========\n");
  unsigned char ind_encoded[] = {
    //0xA1, 0x1D, 0x02, 0x01, 0x01, 0x02, 0x01, 0x2F,
    0x30, 0x15, 0x04, 0x07, 0x91, 0x97, 0x58, 0x81,
    0x55, 0x01, 0xF0, 0x04, 0x07, 0x91, 0x81, 0x67,
    0x83, 0x00, 0x51, 0xF4, 0x0A, 0x01, 0x02
  };
  using smsc::mtsmsme::comp::ReportSmDeliveryStatusInd;
  ReportSmDeliveryStatusInd ind(logger);
  ind.decode(vector<unsigned char>(ind_encoded, ind_encoded + sizeof(ind_encoded) / sizeof(unsigned char) ));
  //TODO: add some validations
  CPPUNIT_ASSERT( true );
}
void AmericaTestFixture::reportSMDeliveryStatus_receiving()
{
  smsc_log_debug(logger, "======== AmericaTestFixture::reportSMDeliveryStatus_receiving ========\n");
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
  vector<unsigned char> res ;
  SccpSenderMock sender(logger, res);
  mtsms.setSccpSender((SccpSender*)&sender);

  mtsms.NUNITDATA((uint8_t) (sizeof(cd)/sizeof(uint8_t)), cd,
                  (uint8_t) (sizeof(cl)/sizeof(uint8_t)), cl,
                  (uint8_t) (sizeof(ud)/sizeof(uint8_t)), ud);
  uint8_t expected_data[] = {
    0x64, 0x39, 0x49, 0x04,
    0x0A, 0x80, 0x37, 0x5A, //transaction id
    0x6B, 0x2A, 0x28, 0x28, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xA0, 0x1D, 0x61,
    0x1B, 0x80, 0x02, 0x07, 0x80, 0xA1, 0x09, 0x06,
    0x07, 0x04, 0x00, 0x00, 0x01, 0x00, 0x14, 0x02,
    0xA2, 0x03, 0x02, 0x01, 0x00, 0xA3, 0x05, 0xA1,
    0x03, 0x02, 0x01, 0x00, 0x6C, 0x05, 0xA2, 0x03,
    0x02, 0x01,
    0x01 //invoke id
  };
  vector<unsigned char> expected(expected_data,expected_data + sizeof(expected_data) / sizeof(unsigned char) );
  CPPUNIT_ASSERT( expected == res);
}
void AmericaTestFixture::sendRoutingInfoForSM_sending()
{
  smsc_log_debug(logger, "======== AmericaTestFixture::sendRoutingInfoForSM_sending ========\n");
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
  vector<unsigned char> result ;
  SccpSenderMock sender(logger, result);
  mtsms.setSccpSender((SccpSender*)&sender);
  TSM* tsm = 0;
  tsm = mtsms.TC_BEGIN(shortMsgGatewayContext_v2);
  if (tsm)
  {
    SendRoutingInfoForSMReq* inv = new SendRoutingInfoForSMReq("79139859489", true, "79139869999");
    tsm->TInvokeReq(1, 45, *inv);
    tsm->TBeginReq((uint8_t) (sizeof(cd) / sizeof(uint8_t)), cd,
        (uint8_t) (sizeof(cl) / sizeof(uint8_t)), cl);
  }
  uint8_t expected_data[] = {
    0x62, 0x47, 0x48, 0x04, 0xBA, 0xBE, 0x00, 0x01,
    0x6B, 0x1E, 0x28, 0x1C, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xA0, 0x11, 0x60,
    0x0F, 0x80, 0x02, 0x07, 0x80, 0xA1, 0x09, 0x06,
    0x07, 0x04, 0x00, 0x00, 0x01, 0x00, 0x14, 0x02,
    0x6C, 0x1F, 0xA1, 0x1D, 0x02, 0x01, 0x01, 0x02,
    0x01, 0x2D, 0x30, 0x15, 0x80, 0x07, 0x91, 0x97,
    0x31, 0x89, 0x95, 0x84, 0xF9, 0x81, 0x01, 0xFF,
    0x82, 0x07, 0x91, 0x97, 0x31, 0x89, 0x96, 0x99,
    0xF9
  };
  vector<unsigned char> expected(expected_data,expected_data + sizeof(expected_data) / sizeof(unsigned char) );
  CPPUNIT_ASSERT( expected == result);
}
void AmericaTestFixture::updateLocation_dialogue_cleanup(void)
{
  smsc_log_debug(logger, "======== AmericaTestFixture::updateLocation_dialogue_cleanup ========\n");
  /*
   * <section name="250013903784021">
   *   <param name="reg_type" type="string">external</param>
   *   <param name="address" type="string">.0.1.250013903784021</param>
   *   <param name="alias" type="string">.1.1.79134632021</param>
   *   <param name="mgt" type="string">.1.1.791603903784021</param>
   *   <param name="msisdn" type="string">.1.1.79134632021</param>
   *   <param name="period" type="int">3600</param>
   * </section>
   * <section name="SCCP">
   *   <param name="user_id" type="int">43</param>
   *   <param name="user_ssn" type="int">191</param>
   *   <param name="msc_gt" type="string">.1.1.791398699812</param>
   *   <param name="vlr_gt" type="string">.1.1.791398699813</param>
   *   <param name="hlr_gt" type="string">.1.1.791398699813</param>
   * </section>
   */
  using std::string;
  using smsc::mtsmsme::processor::TCO;
  using smsc::mtsmsme::processor::TSM;
  using smsc::sms::Address;
  using smsc::mtsmsme::processor::util::packSCCPAddress;
  using smsc::mtsmsme::processor::net_loc_upd_v2;
  using smsc::mtsmsme::comp::UpdateLocationReq;
  using smsc::mtsmsme::processor::TSMSTAT;

  string imsi ("250013903784021");
  string msisdn ("79134632021");
  string mgt ("791603903784021");
  int period = 3600;
  string msc_digits ("791398699812");
  string vlr_digits ("791398699813");
  string hlr_digits ("791398699813");

  TCO mtsms(10);
  mtsms.setAdresses(
      Address((uint8_t)strlen(msc_digits.c_str()), 1, 1, msc_digits.c_str()),
      Address((uint8_t)strlen(vlr_digits.c_str()), 1, 1, vlr_digits.c_str()),
      Address((uint8_t)strlen(hlr_digits.c_str()), 1, 1, hlr_digits.c_str()));
  vector<unsigned char> res;
  SccpSenderMock sender(logger, res);
  mtsms.setSccpSender((SccpSender*)&sender);
  TsmComletionListenerMock listener(logger);
  TSMSTAT stat;
  TSM::getCounters(stat);
  CPPUNIT_ASSERT( stat.objcount == 0 );
  smsc_log_debug(logger,
      "FAKE UPDATELOCATION imsi=\'%s\', msisdn=\'%s\', mgt=\'%s\' with period=%d seconds"
      " serving by msc=\'%s\', vlr=\'%s\'",
      imsi.c_str(), msisdn.c_str(), mgt.c_str(), period,
      msc_digits.c_str(), vlr_digits.c_str());

  TSM* tsm = 0;
  tsm = mtsms.TC_BEGIN(net_loc_upd_v2);
  TSM::getCounters(stat);
  CPPUNIT_ASSERT( stat.objcount == 1 );
  if (tsm)
  {
    tsm->setCompletionListener((TsmComletionListener*)&listener);

    UpdateLocationReq msg;
    msg.setParameters(imsi, msc_digits,vlr_digits);
    tsm->TInvokeReq( 1 /* invokeId */, 2 /* updateLocation operation */, msg);

    uint8_t cl[20]; uint8_t cllen; uint8_t cd[20]; uint8_t cdlen;
    cllen = packSCCPAddress(cl, 1 /* E.164 */, vlr_digits.c_str() /* VLR E.164 */, 7 /* VLR SSN */);
    cdlen = packSCCPAddress(cd, 7 /* E.214 */, mgt.c_str() /* MS  E.214 */, 6 /* HLR SSN */);
    tsm->TBeginReq(cdlen, cd, cllen, cl);
  }
  sleep(70);
  mtsms.dlgcleanup();//simulate timer expiration
  TSM::getCounters(stat);
  CPPUNIT_ASSERT( stat.objcount == 0 );
  CPPUNIT_ASSERT( listener.isCompleted() );
}
void AmericaTestFixture::dialogue_limit_check()
{
  smsc_log_debug(logger, "======== AmericaTestFixture::dialogue_limit_check ========\n");
  using smsc::mtsmsme::processor::TCO;
  using smsc::mtsmsme::processor::TSM;
  using smsc::mtsmsme::processor::shortMsgGatewayContext_v2;

  using smsc::mtsmsme::comp::SendRoutingInfoForSMReq;

  uint8_t cl[] = { 2, 2, 2, 2, 2 };
  uint8_t cd[] = { 3, 3, 3, 3, 3 };
  uint8_t limit = 3;
  TCO mtsms(limit);
  vector<unsigned char> result ;
  SccpSenderMock sender(logger, result);
  mtsms.setSccpSender((SccpSender*)&sender);
  for (int i = 1; i <= limit; i++)
  {
    TSM* tsm = 0;
    tsm = mtsms.TC_BEGIN(shortMsgGatewayContext_v2);
    CPPUNIT_ASSERT( tsm != 0 );

    SendRoutingInfoForSMReq* inv = 0;
    inv = new SendRoutingInfoForSMReq("79139859489", true, "79139869999");
    CPPUNIT_ASSERT( inv != 0 );

    tsm->TInvokeReq(1, 45, *inv);
    tsm->TBeginReq((uint8_t) (sizeof(cd) / sizeof(uint8_t)), cd,
        (uint8_t) (sizeof(cl) / sizeof(uint8_t)), cl);
  }
  TSM* nulltsm = 0;
  nulltsm = mtsms.TC_BEGIN(shortMsgGatewayContext_v2);
  CPPUNIT_ASSERT( nulltsm == 0);
}
void AmericaTestFixture::sendRoutingInfo_arg_encoding()
{
  smsc_log_debug(logger, "======== AmericaTestFixture::sendRoutingInfo_arg_encoding ========\n");
  using smsc::mtsmsme::comp::SendRoutingInfoReq;
  using smsc::mtsmsme::processor::util::dump;

  SendRoutingInfoReq req("79131273996","79139869981");
  vector<unsigned char> result ;
  req.encode(result);
  smsc_log_debug(logger,"encoded SendRoutingInfoReq[%d]={%s}",
                        result.size(),
                        dump((uint16_t)result.size(),&result[0]).c_str());
  uint8_t expected_data[] = {
    //0xa1, 0x1d, 0x02, 0x01, 0x01, 0x02, 0x01, 0x16,
      0x30, 0x15, 0x80, 0x07, 0x91, 0x97, 0x31, 0x21,
      0x37, 0x99, 0xf6, 0x83, 0x01, 0x00, 0x86, 0x07,
      0x91, 0x97, 0x31, 0x89, 0x96, 0x89, 0xf1
  };
  vector<unsigned char> expected(expected_data, expected_data + sizeof(expected_data) / sizeof(unsigned char));
  CPPUNIT_ASSERT( expected == result);
}
void AmericaTestFixture::sendRoutingInfo_sending()
{
  smsc_log_debug(logger, "======== AmericaTestFixture::sendRoutingInfo_sending ========\n");
  using smsc::mtsmsme::processor::TCO;
  using smsc::mtsmsme::processor::TSM;
  using smsc::mtsmsme::processor::TSMSTAT;
  using smsc::mtsmsme::processor::locationInfoRetrievalContext_v3;
  using smsc::logger::Logger;

  using smsc::mtsmsme::comp::SendRoutingInfoReq;

  uint8_t cl[] = { 2, 2, 2, 2, 2 };
  uint8_t cd[] = { 3, 3, 3, 3, 3 };
  TCO mtsms(10);
  vector<unsigned char> result ;
  SccpSenderMock sender(logger, result);
  mtsms.setSccpSender((SccpSender*)&sender);
  TSM* tsm = 0;
  tsm = mtsms.TC_BEGIN(locationInfoRetrievalContext_v3);
  //CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( tsm == 0 ) );
  CPPUNIT_ASSERT( tsm != 0 );
  if (tsm)
  {
    SendRoutingInfoReq req("79131273996","79139869981");
    tsm->TInvokeReq( 1 /* invoke_id */, 22 /* SRI */, req);
    tsm->TBeginReq((uint8_t) (sizeof(cd) / sizeof(uint8_t)), cd,
        (uint8_t) (sizeof(cl) / sizeof(uint8_t)), cl);
  }
  uint8_t expected_data[] = {
    0x62, 0x47, 0x48, 0x04, 0xBA, 0xBE, 0x00, 0x01,
    0x6b, 0x1e, 0x28, 0x1c, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xa0, 0x11, 0x60,
    0x0f, 0x80, 0x02, 0x07, 0x80, 0xa1, 0x09, 0x06,
    0x07, 0x04, 0x00, 0x00, 0x01, 0x00, 0x05, 0x03,
    0x6c, 0x1f, 0xa1, 0x1d, 0x02, 0x01, 0x01, 0x02,
    0x01, 0x16, 0x30, 0x15, 0x80, 0x07, 0x91, 0x97,
    0x31, 0x21, 0x37, 0x99, 0xf6, 0x83, 0x01, 0x00,
    0x86, 0x07, 0x91, 0x97, 0x31, 0x89, 0x96, 0x89,
    0xf1
  };
  vector<unsigned char> expected(expected_data,expected_data + sizeof(expected_data) / sizeof(unsigned char) );
  CPPUNIT_ASSERT( expected == result);

  uint8_t expected_income[] = {  // here is expected SRI res
    0x64, 0x64, 0x49, 0x04,
    0xBA, 0xBE, 0x00, 0x01, //transaction id
    0x6b, 0x2a, 0x28, 0x28, 0x06, 0x07, 0x00, 0x11,
    0x86, 0x05, 0x01, 0x01, 0x01, 0xa0, 0x1d, 0x61,
    0x1b, 0x80, 0x02, 0x07, 0x80, 0xa1, 0x09, 0x06,
    0x07, 0x04, 0x00, 0x00, 0x01, 0x00, 0x05, 0x03,
    0xa2, 0x03, 0x02, 0x01, 0x00, 0xa3, 0x05, 0xa1,
    0x03, 0x02, 0x01, 0x00, 0x6c, 0x80, 0xa2, 0x2c,
    0x02, 0x01, 0x01, 0x30, 0x27, 0x02, 0x01, 0x16,
    0xa3, 0x80, 0x89, 0x08, 0x52, 0x00, 0x31, 0x03,
    0x10, 0x90, 0x88, 0xf6, 0x04, 0x07, 0x91, 0x97,
    0x31, 0x04, 0x99, 0x78, 0xf0, 0x82, 0x07, 0x91,
    0x97, 0x31, 0x03, 0x99, 0x99, 0xf2, 0x8f, 0x02,
    0x04, 0xe0, 0x00, 0x00, 0x00, 0x00,
  };
  mtsms.NUNITDATA((uint8_t) (sizeof(cd)/sizeof(uint8_t)), cd,
                  (uint8_t) (sizeof(cl)/sizeof(uint8_t)), cl,
                  (uint8_t) (sizeof(expected_income)/sizeof(uint8_t)), expected_income);
  TSMSTAT stat;
  TSM::getCounters(stat);
  CPPUNIT_ASSERT( stat.objcount == 0 );
}
void AmericaTestFixture::sendRoutingInfo_res_decoding()
{
  smsc_log_debug(logger, "======== AmericaTestFixture::sendRoutingInfo_res_decoding ========\n");
  using smsc::mtsmsme::comp::SendRoutingInfoConf;
  using smsc::mtsmsme::processor::util::dump;

  uint8_t encoded_data[] = {
  //0x64, 0x64, 0x49, 0x04, 0x03, 0x00, 0x00, 0x00,
  //0x6b, 0x2a, 0x28, 0x28, 0x06, 0x07, 0x00, 0x11,
  //0x86, 0x05, 0x01, 0x01, 0x01, 0xa0, 0x1d, 0x61,
  //0x1b, 0x80, 0x02, 0x07, 0x80, 0xa1, 0x09, 0x06,
  //0x07, 0x04, 0x00, 0x00, 0x01, 0x00, 0x05, 0x03,
  //0xa2, 0x03, 0x02, 0x01, 0x00, 0xa3, 0x05, 0xa1,
  //0x03, 0x02, 0x01, 0x00, 0x6c, 0x80, 0xa2, 0x2c,
  //0x02, 0x01, 0x01, 0x30, 0x27, 0x02, 0x01, 0x16,
    0xa3, 0x80, 0x89, 0x08, 0x52, 0x00, 0x31, 0x03,
    0x10, 0x90, 0x88, 0xf6, 0x04, 0x07, 0x91, 0x97,
    0x31, 0x04, 0x99, 0x78, 0xf0, 0x82, 0x07, 0x91,
    0x97, 0x31, 0x03, 0x99, 0x99, 0xf2, 0x8f, 0x02,
    0x04, 0xe0, 0x00, 0x00//, 0x00, 0x00
  };
  vector<unsigned char> encoded(encoded_data, encoded_data + sizeof(encoded_data) / sizeof(unsigned char));
  SendRoutingInfoConf conf(logger);
  conf.decode(encoded);
  //smsc_log_debug(logger,"decoded MSRN=%s",conf.getMSRN());
  char expected_msrn[] = "79134099870";
  CPPUNIT_ASSERT( 0 == strcmp(expected_msrn,conf.getMSRN()));
}
void AmericaTestFixture::slow_smpp_sender(void)
{
  // send MT_SMS  to TCO
  // 2-simulate timeout on SmppRequestSender and free TSM
  // 3-simulate late response on SmppRequestSender and free already freeed TSM
  smsc_log_debug(logger, "======== AmericaTestFixture::slow_smpp_sender ========\n");
  using smsc::mtsmsme::processor::TCO;
  using smsc::mtsmsme::processor::TSM;
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
  vector<unsigned char> res ;
  SccpSenderMock sender(logger, res);
  mtsms.setSccpSender((SccpSender*)&sender);
  SlowRequestSender smppsender;
  mtsms.setRequestSender((RequestSender*)&smppsender);
  CPPUNIT_ASSERT( smppsender.req_ptr == 0 );

  mtsms.NUNITDATA((uint8_t) (sizeof(cd)/sizeof(uint8_t)), cd,
                  (uint8_t) (sizeof(cl)/sizeof(uint8_t)), cl,
                  (uint8_t) (sizeof(ud)/sizeof(uint8_t)), ud);
  //CPPUNIT_ASSERT( (!(smppsender.req_ptr == 0)) );
  //using smsc::mtsmsme::processor::TSMSTAT;
  //TSMSTAT stat;
  //TSM::getCounters(stat);
  //CPPUNIT_ASSERT( stat.objcount == 1 );
  //smppsender.sendTimeoute();
  //TSM::getCounters(stat);
  //CPPUNIT_ASSERT( stat.objcount == 0 );
}
