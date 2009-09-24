#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class SampleTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE (SampleTest);
  CPPUNIT_TEST (SampleTest1);
  CPPUNIT_TEST (SampleTest2);
  CPPUNIT_TEST_SUITE_END ();
public:
  void setUp(){}
  void tearDown(){}
protected:
  void SampleTest1 (void);
  void SampleTest2 (void);
private:
};
