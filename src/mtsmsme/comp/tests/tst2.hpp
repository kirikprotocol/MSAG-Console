#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "logger/Logger.h"
#include "mtsmsme/processor/SccpSender.hpp"
namespace smsc{namespace mtsmsme{namespace comp{namespace tests{
std::string tsname() { return "smsc::mtsmsme::comp::tests";}
}}}}
using smsc::logger::Logger;
class AmericaTestFixture : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE (AmericaTestFixture);
  CPPUNIT_TEST (reportSMDeliveryStatus_receiving);
  CPPUNIT_TEST (updateLocation_arg_encoding);
  CPPUNIT_TEST (reportSMDeliveryStatus_arg_decoding);
  CPPUNIT_TEST (sendRoutingInfoForSM_sending);
  CPPUNIT_TEST_SUITE_END ();
public:
  class SccpSenderImpl;
  void setUp();
  void tearDown();
protected:
  void reportSMDeliveryStatus_receiving(void);
  void updateLocation_arg_encoding(void);
  void reportSMDeliveryStatus_arg_decoding(void);
  void sendRoutingInfoForSM_sending(void);
private:
  Logger* logger;
  SccpSenderImpl* sender;
public:
  using smsc::mtsmsme::processor::SccpSender;
  class SccpSenderImpl: public SccpSender {
    public:
      virtual void send(uint8_t cdlen,uint8_t *cd,uint8_t cllen,uint8_t *cl,uint16_t ulen,uint8_t *udp)
      {
        smsc_log_debug(logger, "fake sccp sender has pushed message to network");
      };
  };

};
