#include <iostream>
#include <fstream>

// CPPUnit includes
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <tests/util/Configurator.hpp>
#include "Sme.hpp"
#include "CsvOutputter.hpp"
#include "TestSpecification.hpp"
#include "ResponseChecker.hpp"

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
			using smsc::test::util::ContextHandlerMultiMap;

			class BindTest : public CppUnit::TestFixture {
				// logger
				log4cpp::Category& log;
				// SMEs for testing
				QueuedSmeHandler sme;
				QueuedSmeHandler sme1;
				QueuedSmeHandler transmitter;
				QueuedSmeHandler receiver;
				uint32_t timeout;
				static bool configInited;

				typedef void (BindTest::*TestMethod)();
				typedef CppUnit::TestCaller<BindTest> TestCaller;

				static TestCaller* createTestCaller(int testId, TestMethod method) {
					std::ostringstream sout;
					sout << testId;
					return new TestCaller(sout.str(), method);
				}
			public:
				// adding test cases
				static CppUnit::Test * suite() {
					CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "BindTest" );
					suiteOfTests->addTest(createTestCaller(TestSpecification::BIND_TEST_test_bind, &BindTest::testBind));
					suiteOfTests->addTest(createTestCaller(TestSpecification::BIND_TEST_test_transmitter_receiver_enquire_link, &BindTest::testTransmitterReceiverEnquireLink));
					return suiteOfTests;
				}

				BindTest() : log(smsc::test::util::logger.getLog("smsc.test.smpp.BindTest"))
				, timeout(10000) {
					//std::cerr<<"BindTest::BindTest"<<std::endl;

				}
				virtual ~BindTest()
				{
					//std::cerr<<"destructor"<<std::endl;
				}
				void setUp() {
					//std::cerr<<"setup"<<std::endl;
					log.info("enter: setUp Test Suit");
					ContextConfiguratorHandler config = ContextConfigurator::instance();
					try {
						//std::cerr<<configInited<<std::endl;
						if (!configInited) {
							config->loadContext("bind_test.xml");
							configInited = true;
							//std::cerr<<configInited<<std::endl;
						}
					} catch (ConfigurationException &ex) {
						std::cout <<  std::endl << ex.what();
						CPPUNIT_FAIL("ConfigurationException has occured");
					}
					try {
						ContextHandler root = config->getContext("smsc");
						log.debug("Getting subcontext sme");
						smsc::test::util::ContextHandlerList lst = root->findSubcontext("sme");
						typedef smsc::test::util::ContextHandlerList::iterator CHI;
						for (CHI itr = lst.begin(); itr != lst.end(); ++itr) {
							ContextHandler ctx = *itr;
							log.debug("Performing sme context %s", ctx->toString().c_str());
							string name = ctx->getAttribute("name");
							if (name == "sme") {
								log.debug("Creating \"sme\" by using SmeFactory");
								GenericSmeHandler gh = SmeFactory::createSme(ctx);
								sme = gh;
								log.debug(sme->toString());
								log.debug("Creating \"sme1\" by using SmeFactory");
								gh = SmeFactory::createSme(ctx);
								sme1 = gh;
								log.debug(sme1->toString());
							} else if (name == "transmitter") {
								log.debug("Creating \"transmitter\" by using SmeFactory");
								GenericSmeHandler gh = SmeFactory::createSme(ctx);
								transmitter = gh;
								log.debug(transmitter->toString());
							} else if (name == "receiver") {
								log.debug("Creating \"receiver\" by using SmeFactory");
								GenericSmeHandler gh = SmeFactory::createSme(ctx);
								receiver = gh;
								log.debug(receiver->toString());
							}
						}
					} catch (std::runtime_error &ex) {
						std::cout <<  std::endl << ex.what();
						CPPUNIT_FAIL("Exception has occured");
					}
					log.info("exit: Test suit has been set up");
				}

				void tearDown() {
					log.info("method tearDown");
					sme->close();
					sme1->close();
					transmitter->close();
					receiver->close();
				}
			protected:
				// проверка различных вариантов соединения с SMSC
				void testBind() {
					try {
						sme->connect();
						sme1->connect();
						log.debug("Testing simple binding & unbinding ...");

						// transceiver
						log.debug("\n\ntestBind: Connecting as tranceiver");
						uint32_t sequence = sme->bind(smsc::sme::BindType::Transceiver);
						if (ResponseChecker::checkBind(sequence, smsc::sme::BindType::Transceiver, sme, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for transceiver bind pdu");
						}
						sequence = sme->unbind();
						if (ResponseChecker::checkUnbind(sequence, sme, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for transceiver unbind pdu");
						}

						// receiver
						log.debug("\n\ntestBind: Connecting as receiver");
						sequence = sme->bind(smsc::sme::BindType::Receiver);
						if (ResponseChecker::checkBind(sequence, smsc::sme::BindType::Receiver, sme, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for receiver bind pdu");
						}
						sequence = sme->unbind();
						if (ResponseChecker::checkUnbind(sequence, sme, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for receiver unbind pdu");
						}

						// transmitter & receiver
						log.debug("\n\ntestBind: Connecting as transmitter");
						sequence = sme->bind(smsc::sme::BindType::Transmitter);
						if (ResponseChecker::checkBind(sequence, smsc::sme::BindType::Transmitter, sme, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for transmitter bind pdu");
						}
						log.debug("\n\ntestBind: Connecting as receiver");
						sequence = sme1->bind(smsc::sme::BindType::Receiver);
						if (ResponseChecker::checkBind(sequence, smsc::sme::BindType::Receiver, sme1, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for receiver bind pdu");
						}

						sequence = sme->unbind();
						if (ResponseChecker::checkUnbind(sequence, sme, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for transmitter unbind pdu");
						}
						sequence = sme1->unbind();
						if (ResponseChecker::checkUnbind(sequence, sme1, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for receiver unbind pdu");
						}


						// проверка что не может быть 2-х transceivers
						if (checkDuplicateBinding(smsc::sme::BindType::Transceiver) == false) {
							CPPUNIT_FAIL("Test Error: error when checking duplicate transceivers");
						}
						// проверка что не может быть 2-х transmitter
						if (checkDuplicateBinding(smsc::sme::BindType::Transmitter) == false) {
							CPPUNIT_FAIL("Test Error: error when checking duplicate transmitters");
						}
						// проверка что не может быть 2-х receivers
						if (checkDuplicateBinding(smsc::sme::BindType::Receiver) == false) {
							CPPUNIT_FAIL("Test Error: error when checking duplicate receivers");
						}

					} catch (PduListenerException &ex) {
						log.error("PduListenerException when simple binding: %s", ex.what());
						CPPUNIT_FAIL("Test Error: PduListenerException when simple binding");
					}
				}

				// проверка работы transmitter & receiver с одним sistem id
				void testTransmitterReceiverEnquireLink() {
					try {
						transmitter->connect();
						receiver->connect();

						uint32_t sequence = transmitter->bind(smsc::sme::BindType::Transmitter);
						if (ResponseChecker::checkBind(sequence, smsc::sme::BindType::Transmitter, transmitter, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for transmitter bind pdu");
						}

						sequence = receiver->bind(smsc::sme::BindType::Receiver);
						if (ResponseChecker::checkBind(sequence, smsc::sme::BindType::Receiver, receiver, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for receiver bind pdu");
						}

						// проверка, что transmitter получает ответ на enquire_link
						log.debug("Transmitter sends EnquireLink");
						sequence = transmitter->sendEquireLink();
						if (ResponseChecker::checkEnquireLink(sequence, transmitter, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for transmitter enquire_link pdu");
						}
						// проверка, что receiver получает ответ на enquire_link
						log.debug("Receiver sends EnquireLink");
						sequence = receiver->sendEquireLink();
						if (ResponseChecker::checkEnquireLink(sequence, receiver, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for receiver enquire_link pdu");
						}

						//unbinding
						sequence = receiver->unbind();
						if (ResponseChecker::checkUnbind(sequence, receiver, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for receiver unbind pdu");
						}
						sequence = transmitter->unbind();
						if (ResponseChecker::checkUnbind(sequence, transmitter, timeout)== false) {
							CPPUNIT_FAIL("Test Error: error in response for transmitter unbind pdu");
						}

					} catch (PduListenerException &ex) {
						log.error("testTransmitterReceiverEnquireLink: PduListenerException : %s", ex.what());
						CPPUNIT_FAIL("Test Error: PduListenerException in testTransmitterReceiverEnquireLink()");
					}
				}

			private:
				// проверка того, что нельзя забайндить два (transceiver | receiver | transmitter) как 
				// в одном сокете, так и в разных
				bool checkDuplicateBinding(const int bindType) throw(PduListenerException) {
					bool res = true;
					try {
						std::string mode;
						switch (bindType) {
						case smsc::sme::BindType::Receiver:
							mode = "receiver";
							break;
						case smsc::sme::BindType::Transceiver:
							mode = "transceiver";
							break;
						case smsc::sme::BindType::Transmitter:
							mode = "transmitter";
							break;
						default:
							std::ostringstream sout;
							sout << "checkDuplicateBinding: wrong bind type " << bindType;
							throw PduListenerException(sout.str(), bindType);
						}
						// два в разных сокетах
						log.debug("\n\ntestBind: two " + mode + "s");
						log.debug("testBind: binding \"sme\" as a " + mode);
						uint32_t sequence = sme->bind(bindType);
						if (ResponseChecker::checkBind(sequence, bindType, sme, timeout)== false) {
							log.error("Test Error: error in response for %s bind pdu", mode.c_str());
							//res = false;
							return false;
						}
						log.debug("testBind: binding \"sme1\" as a " + mode + " in other socket");
						sequence = sme1->bind(bindType);
						if (ResponseChecker::checkBind(sequence, bindType, sme1, timeout)== false) {
							// все нормально, нельзя законнектиться вторым XXX
							sme1->reconnect();
						} else {
							log.error("checkDuplicateBinding error: the second \"sme1\" has been successfully bound as " + mode);
							log.debug("testBind: unbinding \"sme1\"");
							sequence = sme1->unbind();
							ResponseChecker::checkUnbind(sequence, sme1, timeout);
							//res = false;
							return false;
						}

						// два в одном сокете
						log.debug("testBind: binding \"sme\" as a " + mode + " in the same socket");
						try {
							sequence = sme->bind(bindType);
							if (ResponseChecker::checkBind(sequence, bindType, sme, timeout)== false) {
								// все нормально, нельзя законнектиться вторым XXX
							} else {
								log.error("checkDuplicateBinding error: the second " + mode + " has been successfully bound in the same socket");
								log.debug("testBind: unbinding \"sme\"");
								sequence = sme->unbind();
								ResponseChecker::checkUnbind(sequence, sme, timeout);
								//res = false;
								return false;
							}
						} catch (PduListenerException &ex) {
							// все нормально, байнд должен был быть неудачным
						}
						log.debug("testBind: unbinding \"sme\"");
						sequence = sme->unbind();
						ResponseChecker::checkUnbind(sequence, sme, timeout);
					} catch (std::exception &ex) {
                      log.error("checkDuplicateBinding: std exception has occured, typeid=%s, msg=%s", typeid(ex).name(), ex.what());
                      sme->reconnect();
                      sme1->reconnect();
                    } catch (...) {
						log.error("BindTest#checkDuplicateBinding: unknown exception has occured");
					}

					return res;
				}
			};
			bool BindTest::configInited = false;

		}	// namespace smpp
	}	// namespace test
}	// namespace smsc

int main( int argc, char **argv) {
	try {
		CppUnit::TextUi::TestRunner runner;
		runner.addTest(smsc::test::smpp::BindTest::suite());
		std::ofstream fout("BindTest.res");
		smsc::test::smpp::CsvOutputter csvOutputter(&runner.result(), fout);
		//smsc::test::smpp::CsvOutputter csvOutputter(&runner.result(), std::cout);
		runner.setOutputter(&csvOutputter);
		bool wasSucessful = runner.run();
		smsc::util::xml::TerminateXerces();   
		return wasSucessful;
	} catch (std::exception &ex) {
		std::cout << "Test Wrong: Unexpected error !!!\n" << ex.what();
	} catch (...) {
		std::cout << "Test Wrong: Unexpected error !!!\n";
	}
}




