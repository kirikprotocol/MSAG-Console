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
			using smsc::test::util::ContextHandlerMultiMap;

			// For SMPP testing
			class SMPPTest : public CppUnit::TestFixture {
				// logger
				log4cpp::Category& log;
				// adding test cases
				CPPUNIT_TEST_SUITE( SMPPTest );
				CPPUNIT_TEST( testBind );
				//CPPUNIT_TEST( testTransmitterReceiverEnquireLink );
				//CPPUNIT_TEST_EXCEPTION( testException,  TestException );
				CPPUNIT_TEST_SUITE_END();
				// SME for testing
				GenericSmeHandler sme;
				GenericSmeHandler sme1;
				QueuedSmeHandler transmitter;
				QueuedSmeHandler receiver;
				QueuedSmeHandler sender;
			public:
				SMPPTest() : log(smsc::test::util::logger.getLog("smsc.test.smpp.SMPPTest")) {}
				void setUp() {
					log.info("enter: setUp Test Suit");
					ContextConfiguratorHandler config = ContextConfigurator::instance();
					try {
						config->loadContext();
					} catch (ConfigurationException ex) {
						std::cout <<  std::endl << ex.what();
						CPPUNIT_FAIL("ConfigurationException has occured");
					}
					try {
						ContextHandler root = config->getContext("smsc");
						log.debug("Getting subcontext sme");
						smsc::test::util::ContextHandlerList lst = root->findSubcontext("sme");
						PduListenerHandler listener = new SmppListener();
						typedef smsc::test::util::ContextHandlerList::iterator CHI;
						for (CHI itr = lst.begin(); itr != lst.end(); ++itr) {
							ContextHandler ctx = *itr;
							log.debug("Performing sme context %s", ctx->toString().c_str());
							string name = ctx->getAttribute("name");
							if (name == "sme") {
								log.debug("Creating \"sme\" by using SmeFactory");
								sme = SmeFactory::createSme(ctx);
								sme->setListener(listener);
								log.debug(sme->toString());
								log.debug("Creating \"sme1\" by using SmeFactory");
								sme1 = SmeFactory::createSme(ctx);
								sme1->setListener(listener);
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
							}/* else if (name == "sender") {
								log.debug("Creating \"sender\" by using SmeFactory");
								sender = SmeFactory::createSme(ctx);
								sender->setListener(listener);
								log.debug(sender->toString());
							}*/
						}
					} catch (std::runtime_error ex) {
						std::cout <<  std::endl << ex.what();
						CPPUNIT_FAIL("Exception has occured");
					}
					log.info("exit: Test suit has been set up");
				}

				void tearDown() {
					log.info("method tearDown");
					smsc::util::xml::TerminateXerces();
				}
			protected:
				// проверка различных вариантов соединения с SMSC
				void testBind() {
					try {
						log.debug("Testing simple binding & unbinding ...");
						// transceiver
						log.debug("testBind: Connecting as tranceiver");
						sme->bind(smsc::sme::BindType::Transceiver);
						sleep(1);	//???
						log.debug("testBind: unbinding");
						sme->unbind();
						// receiver
						log.debug("testBind: Connecting as receiver");
						sme->bind(smsc::sme::BindType::Receiver);
						sleep(1);	//???
						log.debug("testBind: unbinding");
						sme->unbind();
						// transmitter
						log.debug("testBind: Connecting as transmitter");
						sme->bind(smsc::sme::BindType::Transmitter);
						sleep(1);	//???
						log.debug("testBind: unbinding");
						sme->unbind();
						// transmitter & receiver
						log.debug("testBind: Connecting as transmitter & receiver");
						log.debug("testBind: Binding \"sme\" as transmitter");
						sme->bind(smsc::sme::BindType::Transmitter);
						log.debug("testBind: Binding \"sme1\" as receiver");
						sme1->bind(smsc::sme::BindType::Receiver);
						sleep(1);	//???
						log.debug("testBind: unbinding \"sme\"");
						sme->unbind();
						log.debug("testBind: unbinding \"sme1\"");
						sme1->unbind();
						// проверка что не может быть 2-х transceivers
						checkDuplicateBinding(smsc::sme::BindType::Transceiver);
						// проверка что не может быть 2-х transmitter
						checkDuplicateBinding(smsc::sme::BindType::Transmitter);
						// проверка что не может быть 2-х receivers
						checkDuplicateBinding(smsc::sme::BindType::Receiver);
					} catch (PduListenerException &ex) {
						log.error("PduListenerException when simple binding: %s", ex.what());
						CPPUNIT_FAIL("Test Error: PduListenerException when simple binding");
					}
				}

				// проверка работы transmitter & receiver с одним sistem id
				void testTransmitterReceiverEnquireLink() {
				  try {
					transmitter->bind(smsc::sme::BindType::Transmitter);
					receiver->bind(smsc::sme::BindType::Receiver);

					// проверка, что transmitter получает ответ на enquire_link
					log.debug("Sending EnquireLink");
					uint32_t sequence = transmitter->sendEquireLink();
					log.debug("SequenceNumber = %u", sequence);
					PduHandler pdu = transmitter->receiveWithSequence(sequence, 10000);
					if(pdu != 0 && pdu->get_commandId() == smsc::smpp::SmppCommandSet::ENQUIRE_LINK_RESP) {
					  log.debug("Transmitter received EnquireLinkResponse");
					} else {
					  log.error("testTransmitterReceiverEnquireLink: Transmitter did not receive EnquireLinkResponse");
					  //CPPUNIT_FAIL("Test Error: testTransmitterReceiverEnquireLink: Transmitter did not receive EnquireLinkResponse");
					}
					// проверка, что receiver получает ответ на enquire_link
					sequence = receiver->sendEquireLink();
					pdu = receiver->receiveWithSequence(sequence, 10000);
					if(pdu != 0 && pdu->get_commandId() == smsc::smpp::SmppCommandSet::ENQUIRE_LINK_RESP) {
					  log.debug("Receiver received EnquireLinkResponse");
					} else {
					  log.error("testTransmitterReceiverEnquireLink: Receiver did not receive EnquireLinkResponse");
					  //CPPUNIT_FAIL("Test Error: testTransmitterReceiverEnquireLink: Receiver did not receive EnquireLinkResponse");
					}

					//unbinding
					receiver->unbind();
					transmitter->unbind();
				  } catch (PduListenerException &ex) {
					  log.error("testTransmitterReceiverEnquireLink: PduListenerException : %s", ex.what());
					  CPPUNIT_FAIL("Test Error: PduListenerException in testTransmitterReceiverEnquireLink()");
				  }
				}

			private:

				// проверка того, что нельзя забайндить два (transmitter | receiver | transmitter) как 
				// в одном сокете, так и в разных
				void checkDuplicateBinding(const int bindType) throw(PduListenerException) {
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
					log.debug("testBind: two " + mode + "s");
					log.debug("testBind: binding \"sme\" as a " + mode);
					sme->bind(bindType);
					sleep(1);
					try {
						log.debug("testBind: binding \"sme1\" as a " + mode + " in other socket");
						sme1->bind(bindType);
						log.debug("checkDuplicateBinding error: the second \"sme1\" has been successfully bound as " + mode);
						CPPUNIT_FAIL("checkDuplicateBinding error: the second \"sme1\" has been successfully bound as " + mode);
					} catch (PduListenerException &ex) {
						// все нормально, нельзя законнектиться вторым XXX
					}
					log.debug("testBind: unbinding \"sme1\"");
					sme1->unbind();
					// два в одном сокете
					try {
						log.debug("testBind: binding \"sme1\" as a " + mode + " in the same socket");
						sme->bind(bindType);
						log.debug("checkDuplicateBinding error: the second " + mode + " has been successfully bound in the same socket");
						CPPUNIT_FAIL("checkDuplicateBinding error: the second " + mode + " has been successfully bound in the same socket");
					} catch (PduListenerException &ex) {
						// все нормально, нельзя законнектиться вторым XXX
						log.error("Normal PduListenerException for second %s\n%s", mode.c_str(), ex.what());
					}
					log.debug("testBind: unbinding \"sme\"");
					sme->unbind();
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
		std::cout << "Test Wrong: Unexpected error !!!\n";
	}
}




