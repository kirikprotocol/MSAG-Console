#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <stdio.h>
int main(int argc, char* argv[])
{
  // Get the top level suite from the registry
  //CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  // Adds the test to the list of test to run
  //CppUnit::TextTestRunner runner;
  //runner.addTest(suite);

  // Run the tests.
  //bool wasSucessful = runner.run();

  // Return error code 1 if the one of test failed.
  //return wasSucessful ? 0 : 1;

  CppUnit::TextTestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );
  //runner.setOutputter(new CppUnit::CompilerOutputter( &runner.result(),std::cerr ));
  bool wasSuccessful = runner.run( "", false );
  return wasSuccessful;
}
