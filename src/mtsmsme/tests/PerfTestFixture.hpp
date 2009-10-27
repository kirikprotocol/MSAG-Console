#ifndef PERFTEST_HPP_
#define PERFTEST_HPP_
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "logger/Logger.h"
using smsc::logger::Logger;
class PerfTestFixture : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE (PerfTestFixture);
  CPPUNIT_TEST (perftest);
  CPPUNIT_TEST (szlist);
  CPPUNIT_TEST_SUITE_END ();
public:
  void setUp();
  void tearDown();
protected:
  void perftest(void);
  void szlist(void);
private:
  Logger* logger;
};


#endif /* PERFTEST_HPP_ */
