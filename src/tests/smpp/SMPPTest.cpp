#include <iostream>

// CPPUnit includes
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tests/util/Configurator.hpp>
#include "Sme.hpp"

namespace smsc {
	namespace test {
		namespace smpp {

			using std::map;
			using std::string;
			using smsc::test::util::ContextConfigurator;
			using smsc::test::util::ContextConfiguratorHandler;
			using smsc::test::util::ConfigurationException;
			using smsc::test::util::ObjectNotFoundException;
			using smsc::test::util::ContextHandler;

			// For SMPP testing
			class SMPPTest : public CppUnit::TestFixture {
				log4cpp::Category& log;
				CPPUNIT_TEST_SUITE( SMPPTest );
				CPPUNIT_TEST( simpleSmppScenario );
				//CPPUNIT_TEST( testAddition );
				//CPPUNIT_TEST_EXCEPTION( testException,  TestException );
				CPPUNIT_TEST_SUITE_END();
				GenericSmeHandler sme;
			public:
				SMPPTest() : log(smsc::test::util::logger.getLog("smsc.test.smpp.SMPPTest")) {
				}
				void setUp() {
					log.info("method setUp");
					ContextConfiguratorHandler config = ContextConfigurator::instance();
					try {
						config->loadContext();
					} catch (ConfigurationException ex) {
						std::cout <<  std::endl << ex.what();
						CPPUNIT_FAIL("ConfigurationException has occured");
					}
					try {
						ContextHandler root = config->getContext("smsc");
						root->print();
						log.debug("Getting subcontext sme");
						ContextHandler ctx = root->firstSubcontext("sme");
						log.debug("Creating SME by using SmeFactory");
						sme = SmeFactory::createSme(ctx);
						std::cout << "host = " << sme->getConfig().host << std::endl;
						std::cout << "port = " << sme->getConfig().port << std::endl;
						std::cout << "sid = " << sme->getConfig().sid << std::endl;
						std::cout << "password = " << sme->getConfig().password << std::endl;
						sme->bind();
					} catch (std::runtime_error ex) {
						std::cout <<  std::endl << ex.what();
						CPPUNIT_FAIL("Exception has occured");
					}
				}

				void tearDown() {
					log.info("method tearDown");
					smsc::util::xml::TerminateXerces();
				}
			protected:
				void simpleSmppScenario() {
				}
			};

		}
	}
}

int main( int argc, char **argv) {
	try {
		CppUnit::TextUi::TestRunner runner;
		runner.addTest( smsc::test::smpp::SMPPTest::suite() );
		bool wasSucessful = runner.run();
		return wasSucessful;
	} catch (...) {
		std::cout << "Unexpected error !!!";
	}
}




