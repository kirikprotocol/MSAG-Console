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
				CPPUNIT_TEST( testTransmitterReceiverEnquireLink );
				//CPPUNIT_TEST_EXCEPTION( testException,  TestException );
				CPPUNIT_TEST_SUITE_END();
				// SME for testing
				QueuedSmeHandler sme;
				QueuedSmeHandler sme1;
				QueuedSmeHandler transmitter;
				QueuedSmeHandler receiver;
				QueuedSmeHandler sender;
				uint32_t timeout;
				bool configInited;
			public:
				SMPPTest() : log(smsc::test::util::logger.getLog("smsc.test.smpp.SMPPTest"))
							, timeout(1000), configInited(false) {}
				void setUp() {
					log.info("enter: setUp Test Suit");
					ContextConfiguratorHandler config = ContextConfigurator::instance();
					try {
  						if(!configInited) {
						  config->loadContext();
						  configInited = true;
						}
					} catch (ConfigurationException ex) {
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
				// �������� ��������� ��������� ���������� � SMSC
				void testBind() {
					try {
  						sme->connect();
						sme1->connect();
						log.debug("Testing simple binding & unbinding ...");

						// transceiver
						log.debug("\n\ntestBind: Connecting as tranceiver");
						uint32_t sequence = sme->bind(smsc::sme::BindType::Transceiver);
						if(checkBind(sequence, smsc::sme::BindType::Transceiver, sme)== false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error in response for transceiver bind pdu");
						}
						sequence = sme->unbind();
						if(checkUnbind(sequence, sme)== false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error in response for transceiver unbind pdu");
						}

						// receiver
						log.debug("\n\ntestBind: Connecting as receiver");
						sequence = sme->bind(smsc::sme::BindType::Receiver);
						if(checkBind(sequence, smsc::sme::BindType::Receiver, sme)== false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error in response for receiver bind pdu");
						}
						sequence = sme->unbind();
						if(checkUnbind(sequence, sme)== false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error in response for receiver unbind pdu");
						}

						// transmitter & receiver
						log.debug("\n\ntestBind: Connecting as transmitter");
						sequence = sme->bind(smsc::sme::BindType::Transmitter);
						if(checkBind(sequence, smsc::sme::BindType::Transmitter, sme)== false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error in response for transmitter bind pdu");
						}
						log.debug("\n\ntestBind: Connecting as receiver");
						sequence = sme1->bind(smsc::sme::BindType::Receiver);
						if(checkBind(sequence, smsc::sme::BindType::Receiver, sme1)== false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error in response for receiver bind pdu");
						}

						sequence = sme->unbind();
						if(checkUnbind(sequence, sme)== false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error in response for transmitter unbind pdu");
						}
						sequence = sme1->unbind();
						if(checkUnbind(sequence, sme1)== false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error in response for receiver unbind pdu");
						}

						
						// �������� ��� �� ����� ���� 2-� transceivers
						if(checkDuplicateBinding(smsc::sme::BindType::Transceiver) == false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error when checking duplicate transceivers");
						}
						// �������� ��� �� ����� ���� 2-� transmitter
						if(checkDuplicateBinding(smsc::sme::BindType::Transmitter) == false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error when checking duplicate transmitters");
						}
						// �������� ��� �� ����� ���� 2-� receivers
						if(checkDuplicateBinding(smsc::sme::BindType::Receiver) == false) {
						  sme1->close();
						  sme->close();
						  CPPUNIT_FAIL("Test Error: error when checking duplicate receivers");
						}

						sme1->close();
						sme->close();
					} catch (PduListenerException &ex) {
						log.error("PduListenerException when simple binding: %s", ex.what());
						CPPUNIT_FAIL("Test Error: PduListenerException when simple binding");
					}
				}

				// �������� ������ transmitter & receiver � ����� sistem id
				void testTransmitterReceiverEnquireLink() {
				  try {
					transmitter->connect();
					receiver->connect();

					uint32_t sequence = transmitter->bind(smsc::sme::BindType::Transmitter);
					if(checkBind(sequence, smsc::sme::BindType::Transmitter, transmitter)== false) {
					  CPPUNIT_FAIL("Test Error: error in response for transmitter bind pdu");
					}

					sequence = receiver->bind(smsc::sme::BindType::Receiver);
					if(checkBind(sequence, smsc::sme::BindType::Receiver, receiver)== false) {
					  CPPUNIT_FAIL("Test Error: error in response for receiver bind pdu");
					}

					// ��������, ��� transmitter �������� ����� �� enquire_link
					log.debug("Transmitter sends EnquireLink");
					sequence = transmitter->sendEquireLink();
					if(checkEnquireLink(sequence, transmitter)== false) {
					  CPPUNIT_FAIL("Test Error: error in response for transmitter enquire_link pdu");
					}
					// ��������, ��� receiver �������� ����� �� enquire_link
					log.debug("Receiver sends EnquireLink");
					sequence = receiver->sendEquireLink();
					if(checkEnquireLink(sequence, receiver)== false) {
					  CPPUNIT_FAIL("Test Error: error in response for receiver enquire_link pdu");
					}

					//unbinding
					sequence = receiver->unbind();
					if(checkUnbind(sequence, receiver)== false) {
					  CPPUNIT_FAIL("Test Error: error in response for receiver unbind pdu");
					}
					sequence = transmitter->unbind();
					if(checkUnbind(sequence, transmitter)== false) {
					  CPPUNIT_FAIL("Test Error: error in response for transmitter unbind pdu");
					}
				  
					transmitter->close();
					receiver->close();
				  } catch (PduListenerException &ex) {
					  log.error("testTransmitterReceiverEnquireLink: PduListenerException : %s", ex.what());
					  CPPUNIT_FAIL("Test Error: PduListenerException in testTransmitterReceiverEnquireLink()");
				  }
				}

			private:
  				//�������� ������� �� ������� bind
  				bool checkBind(uint32_t sequence, int bindType, QueuedSmeHandler sme) {
				  bool res = false;
				  int expectedCommandId;
				  std::string bindTypeMsg;
				  switch (bindType) {
				  case smsc::sme::BindType::Transceiver:
					expectedCommandId = smsc::smpp::SmppCommandSet::BIND_TRANCIEVER_RESP;
  					bindTypeMsg = "Transceiver";
					break;
				  case smsc::sme::BindType::Transmitter:
					expectedCommandId = smsc::smpp::SmppCommandSet::BIND_TRANSMITTER_RESP;
					bindTypeMsg = "Transmitter";
					break;
				  case smsc::sme::BindType::Receiver:
					expectedCommandId = smsc::smpp::SmppCommandSet::BIND_RECIEVER_RESP;
					bindTypeMsg = "Receiver";
					break;
				  }
				  PduHandler pdu = sme->receiveWithSequence(sequence, timeout);
				  if(pdu != 0) {
					if(log.isDebugEnabled()) {
					  log.debug("checkBind: Received response for binding for %s", bindTypeMsg.c_str());
					  log.debug("checkBind: command_id=%x", pdu->get_commandId());
					  log.debug("checkBind: command_status=%x", pdu->get_commandStatus());
					  log.debug("checkBind: sequence_number=%u", pdu->get_sequenceNumber());
					}
					if(pdu->get_commandId() == expectedCommandId) {
					  smsc::smpp::PduBindTRXResp *resp=(smsc::smpp::PduBindTRXResp*)pdu.getObjectPtr();
					  log.debug("checkBind: sms_system_id=%s", resp->get_systemId());
					  if(pdu->get_commandStatus() == smsc::smpp::SmppStatusSet::ESME_ROK) {
						res = true;
					  } else {
						log.error("checkBind: The bind response with sequence=%u has wrong status=%x", sequence, pdu->get_commandStatus());
					  }
					} else {
					  log.error("checkBind: The response for %s with sequence=%u has wrong commandId", bindTypeMsg.c_str(), sequence);
					}
				  } else {
					log.error("checkBind: There is no response for %s with sequence=%u", bindTypeMsg.c_str(), sequence);
				  }
				  return res;
				}

  				//�������� ������� �� ������� unbind
  				bool checkUnbind(uint32_t sequence, QueuedSmeHandler sme) {
				  bool res = false;
				  PduHandler pdu = sme->receiveWithSequence(sequence, timeout);
				  if(pdu != 0) {
					if(log.isDebugEnabled()) {
					  log.debug("checkUnbind: Received response for unbinding");
					  log.debug("checkUnbind: command_id=%x", pdu->get_commandId());
					  log.debug("checkUnbind: command_status=%x", pdu->get_commandStatus());
					  log.debug("checkUnbind: sequence_number=%u", pdu->get_sequenceNumber());
					}
					if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::UNBIND_RESP) {
					  if(pdu->get_commandStatus() == smsc::smpp::SmppStatusSet::ESME_ROK) {
						res = true;
					  } else {
						log.error("checkUnbind: The unbind response with sequence=%u has wrong status=%x", sequence, pdu->get_commandStatus());
					  }
					} else {
					  log.error("checkUnbind: The unbind response with sequence=%u has wrong commandId", sequence);
					}
				  } else {
					log.error("checkUnbind: There is no response for unbind with sequence=%u", sequence);
				  }
				  return res;
				}

  				//�������� ������� �� ������� enquire_link
  				bool checkEnquireLink(uint32_t sequence, QueuedSmeHandler sme) {
				  bool res = false;
				  PduHandler pdu = sme->receiveWithSequence(sequence, timeout);
				  if(pdu != 0) {
					if(log.isDebugEnabled()) {
					  log.debug("checkEnquireLink: Received response for enquire_link");
					  log.debug("checkEnquireLink: command_id=%x", pdu->get_commandId());
					  log.debug("checkEnquireLink: command_status=%x", pdu->get_commandStatus());
					  log.debug("checkEnquireLink: sequence_number=%u", pdu->get_sequenceNumber());
					}
					if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::ENQUIRE_LINK_RESP) {
					  if(pdu->get_commandStatus() == smsc::smpp::SmppStatusSet::ESME_ROK) {
						res = true;
					  } else {
						log.error("checkEnquireLink: The enquire_link response with sequence=%u has wrong status=%x", sequence, pdu->get_commandStatus());
					  }
					} else {
					  log.error("checkEnquireLink: The enquire_link response with sequence=%u has wrong commandId", sequence);
					}
				  } else {
					log.error("checkUnbind: There is no response for enquire_link with sequence=%u", sequence);
				  }
				  return res;
				}

				// �������� ����, ��� ������ ���������� ��� (transceiver | receiver | transmitter) ��� 
				// � ����� ������, ��� � � ������
				bool checkDuplicateBinding(const int bindType) throw(PduListenerException) {
				  try {
  					bool res = true;
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
					// ��� � ������ �������
					log.debug("\n\ntestBind: two " + mode + "s");
					log.debug("testBind: binding \"sme\" as a " + mode);
					uint32_t sequence = sme->bind(bindType);
					if(checkBind(sequence, bindType, sme)== false) {
					  log.error("Test Error: error in response for %s bind pdu", mode.c_str());
					  //res = false;
					  return false;
					}
					log.debug("testBind: binding \"sme1\" as a " + mode + " in other socket");
					sequence = sme1->bind(bindType);
					if(checkBind(sequence, bindType, sme1)== false) {
					  // ��� ���������, ������ �������������� ������ XXX
					  sme1->reconnect();
					} else {
					  log.error("checkDuplicateBinding error: the second \"sme1\" has been successfully bound as " + mode);
					  log.debug("testBind: unbinding \"sme1\"");
					  sequence = sme1->unbind();
					  checkUnbind(sequence, sme1);
					  //res = false;
					  return false;
					}

					// ��� � ����� ������
					log.debug("testBind: binding \"sme\" as a " + mode + " in the same socket");
					try {
					  sequence = sme->bind(bindType);
					  if(checkBind(sequence, bindType, sme)== false) {
						// ��� ���������, ������ �������������� ������ XXX
					  } else {
						log.error("checkDuplicateBinding error: the second " + mode + " has been successfully bound in the same socket");
						log.debug("testBind: unbinding \"sme\"");
						sequence = sme->unbind();
						checkUnbind(sequence, sme);
						//res = false;
						return false;
					  }
					} catch (PduListenerException ex) {
					  // ��� ���������, ����� ������ ��� ���� ���������
					}
					log.debug("testBind: unbinding \"sme\"");
					sequence = sme->unbind();
					checkUnbind(sequence, sme);

					return res;

				  } catch(...) {
					log.error("SMPPTest#checkDuplicateBinding: unknown exception has occured");
				  }

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




