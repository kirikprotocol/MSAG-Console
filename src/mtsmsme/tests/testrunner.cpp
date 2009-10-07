#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TextTestRunner.h>
#include <cppunit/XmlOutputter.h>
#include <fstream>

int main(int argc, char* argv[])
{
  // Define the file that will store the XML output.
  std::ofstream outputFile("cpptestresults.xml");

  // Create a test runner to execute the test cases you have defined (in other source code).
  CppUnit::TextTestRunner runner;

  // Get the top level suite from the registry
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();

  // Adds the test to the list of test to run
  runner.addTest( registry.makeTest() );

  // Specify XML output and inform the test runner of this format.
  CppUnit::XmlOutputter* outputter =
    new CppUnit::XmlOutputter(&runner.result(), outputFile);
  runner.setOutputter(outputter);

  // Run the tests.
  bool wasSuccessful = runner.run( "", false, true, false );
  outputFile.close();
  // Return error code 1 if the one of test failed.
  return wasSuccessful;
}
