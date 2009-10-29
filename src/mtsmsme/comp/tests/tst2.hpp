#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "logger/Logger.h"
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
  CPPUNIT_TEST (updateLocation_dialogue_cleanup);
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
  void updateLocation_dialogue_cleanup(void);
  void dialogue_limit_check(void);
private:
  Logger* logger;
};
