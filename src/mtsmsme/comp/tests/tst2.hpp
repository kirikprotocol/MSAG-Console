#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "logger/Logger.h"
namespace smsc{namespace mtsmsme{namespace comp{namespace tests{
std::string tsname() { return "smsc::mtsmsme::comp::tests";}
}}}}
using smsc::logger::Logger;
class AmericaTestFixture : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE (AmericaTestFixture);
  CPPUNIT_TEST (AmericaTest);
  CPPUNIT_TEST (ulprint);
  CPPUNIT_TEST_SUITE_END ();
public:
  void setUp();
  void tearDown();
protected:
  void AmericaTest (void);
  void ulprint(void);
  void sri4smprint(void);
private:
  Logger* logger;
};
