#include <iostream>

// CPPUnit includes
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include "Configurator.hpp"

namespace smsc {
	namespace test {
		namespace util {
			using std::map;
			using std::string;

			// test class for Configurator
			class ConfiguratorTest : public CppUnit::TestFixture {
				CPPUNIT_TEST_SUITE( ConfiguratorTest );
				CPPUNIT_TEST( testConfigurator );
				//CPPUNIT_TEST( testAddition );
				//CPPUNIT_TEST_EXCEPTION( testException,  TestException );
				CPPUNIT_TEST_SUITE_END();

			protected:
				void testConfigurator() {
					ContextConfiguratorHandler config = ContextConfigurator::instance();
					try {
						config->loadContext();
						//config.loadContext("test.xml");
					} catch (ConfigurationException ex) {
						std::cout <<  std::endl << ex.what();
						CPPUNIT_FAIL("ConfigurationException has occured");
					}
					try {
						ContextHandler root = config->getContext("default_context");
						std::cout << root->toString();
					} catch (ObjectNotFoundException ex) {
						std::cout <<  std::endl << ex.what();
						CPPUNIT_FAIL("ObjectNotFoundException has occured");
					}
				}
			};
		}
	}
}

int main( int argc, char **argv) {
	try {
		// CPP Unit test
		CppUnit::TextUi::TestRunner runner;
		runner.addTest( smsc::test::util::ConfiguratorTest::suite() );
		bool wasSucessful = runner.run();
		return wasSucessful;
	} catch (...) {
		printf("Unexpected error !!!");
	}
	smsc::util::xml::TerminateXerces();
}




