#include "mtsmsme/comp/tests/tst1.hpp"


CPPUNIT_TEST_SUITE_REGISTRATION (SampleTest);

void SampleTest::SampleTest1 (void)
{
  CPPUNIT_ASSERT( true );
}
void SampleTest::SampleTest2 (void)
{
  CPPUNIT_ASSERT_MESSAGE( "just a message assertion test", false);
}
