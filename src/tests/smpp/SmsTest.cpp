#include <iostream>
#include <fstream>

// CPPUnit includes
#include <cppunit/TestFixture.h>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <util/smstext.h>
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

			class SmsTest : public CppUnit::TestFixture {
				// logger
				log4cpp::Category& log;
				// SMEs for testing
				QueuedSmeHandler sender;
				QueuedSmeHandler receiver;
				uint32_t timeout;
				static bool configInited;

				typedef void (SmsTest::*TestMethod)();
				typedef CppUnit::TestCaller<SmsTest> TestCaller;

				static TestCaller* createTestCaller(int testId, TestMethod method) {
					std::ostringstream sout;
					sout << testId;
					return new TestCaller(sout.str(), method);
				}
			public:
				// adding test cases
				static CppUnit::Test * suite() {
					CppUnit::TestSuite *suiteOfTests = new CppUnit::TestSuite( "SmsTest" );
					suiteOfTests->addTest(createTestCaller(TestSpecification::SMS_TEST_test_sms_for_transmitter_receiver_itself, &SmsTest::testSmsForTransmitterReceiverItself));
					//suiteOfTests->addTest(createTestCaller(TestSpecification::SMS_TEST_test_sms_for_transceiver_itself, &SmsTest::testSmsForTransceiverItself));
					return suiteOfTests;
				}

				SmsTest() : log(smsc::test::util::logger.getLog("smsc.test.smpp.SmsTest"))
				, timeout(10000) {}

				void setUp() {
					//std::cerr<<"setup"<<std::endl;
					log.info("enter: setUp Test Suit");
					ContextConfiguratorHandler config = ContextConfigurator::instance();
					try {
						//std::cerr<<configInited<<std::endl;
						if (!configInited) {
							config->loadContext("sms_test.xml");
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
							if (name == "sender") {
								log.debug("Creating \"sender\" by using SmeFactory");
								GenericSmeHandler gh = SmeFactory::createSme(ctx);
								sender = gh;
								log.debug(sender->toString());
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
					sender->close();
					receiver->close();
				}
			protected:
				void testSmsForTransmitterReceiverItself() {
					try {
						sender->connect();
						receiver->connect();

						// binding as transmitter
						uint32_t sequence = sender->bind(smsc::sme::BindType::Transmitter);
						if (ResponseChecker::checkBind(sequence, smsc::sme::BindType::Transmitter, sender, timeout)== false) {
							CPPUNIT_FAIL("testSmsForItself: error in response for transmitter bind pdu");
						}

						// binding as receiver
						sequence = receiver->bind(smsc::sme::BindType::Receiver);
						if (ResponseChecker::checkBind(sequence, smsc::sme::BindType::Receiver, receiver, timeout)== false) {
							CPPUNIT_FAIL("testSmsForItself: error in response for receiver bind pdu");
						}

						//for(int mode = 0; mode < 3; mode++) {
						for(int mode = 2; mode < 3; mode++) {
							switch(mode) {
							case 0:
								log.debug("\n******** STORE AND FORWARD MESSAGE MODE ********\n");
								break;
							case 1:
								log.debug("\n******** DATAGRAM MESSAGE MODE ********\n");
								break;
							case 2:
								log.debug("\n******** TRANSACTION MESSAGE MODE ********\n");
								break;
							}
							// sender sends SMS to receiver
							smsc::smpp::PduSubmitSm submit;
							submit.get_header().set_commandId(smsc::smpp::SmppCommandSet::SUBMIT_SM);
							smsc::sms::SMS sms;
							try {
								smsc::sms::Address origAddr(sender->getConfig().origAddr.c_str());
								sms.setOriginatingAddress(origAddr);
								smsc::sms::Address destAddr(receiver->getConfig().origAddr.c_str());
								sms.setDestinationAddress(destAddr);
							} catch(...) {
								CPPUNIT_FAIL("testSmsForItself: Invalid source address");
							}
							char msc[]="";
							char imsi[]="";
							sms.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
							sms.setValidTime(0);
							sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS, mode); 
							//sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,1); //datagram
							//sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,2); //transaction						
							sms.setDeliveryReport(0);
							sms.setArchivationRequested(false);
							sms.setIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE, 1234);
							sms.setEServiceType("XXX");
							// message 
							const char *message = "Simple SMS for testing purposes";
							uint32_t length = strlen(message);
							sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,smsc::smpp::DataCoding::LATIN1);
							sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, message, length);
							smsc::smpp::fillSmppPduFromSms(&submit, &sms);
							sequence = sender->sendPdu((smsc::smpp::SmppHeader*)&submit);

							// receive SMS
							PduHandler pdu = receiver->receive(timeout);
							if(pdu != 0) {
								if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM) {
									// send deliver_sm_resp
									PduDeliverySmResp resp;
									resp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DELIVERY_SM_RESP);
									resp.set_messageId("");
									resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
									resp.get_header().set_commandStatus(smsc::smpp::SmppStatusSet::ESME_ROK);
									sender->sendPduAsIs((smsc::smpp::SmppHeader*) &resp);

									// check PDU received
									smsc::sms::SMS receivedSms;
									smsc::smpp::PduXSm *pduXSM = (smsc::smpp::PduXSm*) pdu.getObjectPtr();
									smsc::smpp::fetchSmsFromSmppPdu(pduXSM, &receivedSms);
									if(log.isDebugEnabled()) {
										log.debug("testSmsForItself: SMS originating address = %s", receivedSms.getOriginatingAddress().toString().c_str());
										log.debug("testSmsForItself: SMS destination address = %s", receivedSms.getDestinationAddress().toString().c_str());
										log.debug("testSmsForItself: SMS data coding = %d", receivedSms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING));
										log.debug("testSmsForItself: SMS user message reference = %d", receivedSms.getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE));
									}
									bool isEqual = false;
									char buf[256];
									if(smsc::util::getPduText(pduXSM, buf, sizeof(buf))==-1) {
										int sz=(pduXSM)->optional.size_messagePayload();
										char *data = new char[sz*2];
										if(smsc::util::getPduText(pduXSM, data, sz*2)!=-1) {
											log.debug("testSmsForItself: SMS message(payload) = %s", data);
											isEqual = (strcmp(message, data) == 0);
										} else {
											log.debug("testSmsForItself:: faield to retrieve SMS message");
										}
										delete [] data;
									} else {
										log.debug("testSmsForItself: SMS message = %s", buf);
										isEqual = (strcmp(message, buf) == 0);
									}
									if(!isEqual) {
										CPPUNIT_FAIL("testSmsForItself: sender received SMS that is not equal an original SMS sent");
									}
								} else {
									log.debug("testSmsForItself: receiver received unexpected PDU with commandId=%x", pdu->get_commandId());
									CPPUNIT_FAIL("testSmsForItself: receiver received unexpected PDU");
								}
							} else {
								CPPUNIT_FAIL("testSmsForItself: receiver did not receive delivery_sm");
							}

							// sender checks response from SMSC
							if(ResponseChecker::checkResponse(sequence, smsc::smpp::SmppCommandSet::SUBMIT_SM, sender, timeout) == false) {						
								CPPUNIT_FAIL("testSmsForItself: error in response for submit_sm sent");
							}
						}

						// unbinding transmitter
						sequence = sender->unbind();
						if (ResponseChecker::checkUnbind(sequence, sender, timeout)== false) {
							CPPUNIT_FAIL("testSmsForItself: error when sender unbinding");
						}

						// unbinding receiver
						sequence = receiver->unbind();
						if (ResponseChecker::checkUnbind(sequence, receiver, timeout)== false) {
							CPPUNIT_FAIL("testSmsForItself: error when receiver unbinding");
						}

					} catch (CppUnit::Exception &ex) {
						throw;
					} catch (PduListenerException &ex) {
						log.error("testSmsForItself: PduListenerException : %s", ex.what());
						CPPUNIT_FAIL("Test Error: PduListenerException in testSmsForItself()");
					} catch (std::exception &ex) {
						log.error("testSmsForItself: std::exception : %s, type=%s", ex.what(), typeid(ex).name());
						CPPUNIT_FAIL("Test Error: std::exception in testSmsForItself()");
					}
				}

				void testSmsForTransceiverItself() {
					try {
						sender->connect();

						// binding as transceiver
						uint32_t sequence = sender->bind(smsc::sme::BindType::Transceiver);
						if (ResponseChecker::checkBind(sequence, smsc::sme::BindType::Transceiver, sender, timeout)== false) {
							CPPUNIT_FAIL("testSmsForItself: error in response for transceiver bind pdu");
						}

						for(int mode = 0; mode < 3; mode++) {
							switch(mode) {
							case 0:
								log.debug("\n******** STORE AND FORWARD MESSAGE MODE ********\n");
								break;
							case 1:
								log.debug("\n******** DATAGRAM MESSAGE MODE ********\n");
								break;
							case 2:
								log.debug("\n******** TRANSACTION MESSAGE MODE ********\n");
								break;
							}
							// sender sends SMS to itself
							smsc::smpp::PduSubmitSm submit;
							submit.get_header().set_commandId(smsc::smpp::SmppCommandSet::SUBMIT_SM);
							smsc::sms::SMS sms;
							try {
								smsc::sms::Address origAddr(sender->getConfig().origAddr.c_str());
								sms.setOriginatingAddress(origAddr);
								sms.setDestinationAddress(origAddr);
							} catch(...) {
								CPPUNIT_FAIL("testSmsForItself: Invalid source address");
							}
							char msc[]="";
							char imsi[]="";
							sms.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
							sms.setValidTime(0);
							sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS, mode); 
							sms.setDeliveryReport(0);
							sms.setArchivationRequested(false);
							sms.setIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE, 1234);
							sms.setEServiceType("XXX");
							// message 
							const char *message = "Simple SMS for testing purposes";
							uint32_t length = strlen(message);
							sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,smsc::smpp::DataCoding::LATIN1);
							sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, message, length);
							smsc::smpp::fillSmppPduFromSms(&submit, &sms);
							sequence = sender->sendPdu((smsc::smpp::SmppHeader*)&submit);

							// receive SMS and response to submit
							for(int i=0; i<2; i++) {
								PduHandler pdu = sender->receive(timeout);
								if(pdu != 0) {
									if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM) {
										log.debug("testSmsForTransceieverItself: Processing delivery_sm");
										// send deliver_sm_resp
										PduDeliverySmResp resp;
										resp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DELIVERY_SM_RESP);
										resp.set_messageId("");
										resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
										sender->sendPduAsIs((smsc::smpp::SmppHeader*) &resp);

										// check PDU received
										smsc::sms::SMS receivedSms;
										smsc::smpp::PduXSm *pduXSM = (smsc::smpp::PduXSm*) pdu.getObjectPtr();
										smsc::smpp::fetchSmsFromSmppPdu(pduXSM, &receivedSms);
										if(log.isDebugEnabled()) {
											log.debug("testSmsForItself: SMS originating address = %s", receivedSms.getOriginatingAddress().toString().c_str());
											log.debug("testSmsForItself: SMS destination address = %s", receivedSms.getDestinationAddress().toString().c_str());
											log.debug("testSmsForItself: SMS data coding = %d", receivedSms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING));
											log.debug("testSmsForItself: SMS user message reference = %d", receivedSms.getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE));
										}
										bool isEqual = false;
										char buf[256];
										if(smsc::util::getPduText(pduXSM, buf, sizeof(buf))==-1) {
											int sz=(pduXSM)->optional.size_messagePayload();
											char *data = new char[sz*2];
											if(smsc::util::getPduText(pduXSM, data, sz*2)!=-1) {
												log.debug("testSmsForItself: SMS message(payload) = %s", data);
												isEqual = (strcmp(message, data) == 0);
											} else {
												log.debug("testSmsForItself:: faield to retrieve SMS message");
											}
											delete [] data;
										} else {
											log.debug("testSmsForItself: SMS message = %s", buf);
											isEqual = (strcmp(message, buf) == 0);
										}
										if(!isEqual) {
											CPPUNIT_FAIL("testSmsForItself: sender received SMS that is not equal an original SMS sent");
										}
									} else if (pdu->get_commandId() == smsc::smpp::SmppCommandSet::SUBMIT_SM_RESP) {
										log.debug("testSmsForTransceieverItself: Processing submit_sm_resp");
										// sender checks response from SMSC
										if (pdu->get_commandStatus() != smsc::smpp::SmppStatusSet::ESME_ROK) {
											log.error("testSmsForItself: The submit_sm_resp has wrong status=%u", pdu->get_commandStatus());
											CPPUNIT_FAIL("testSmsForItself: receiver received wrong submit_sm_resp");
										}
									} else {
										log.debug("testSmsForItself: receiver received unexpected PDU with commandId=%x", pdu->get_commandId());
										CPPUNIT_FAIL("testSmsForItself: receiver received unexpected PDU");
									}
								} else {
									CPPUNIT_FAIL("testSmsForItself: receiver did not receive delivery_sm or submit_sm_resp");
								}
							}
						}

						// unbinding transceiver
						sequence = sender->unbind();
						if (ResponseChecker::checkUnbind(sequence, sender, timeout)== false) {
							CPPUNIT_FAIL("testSmsForItself: error when sender unbinding");
						}
					} catch (CppUnit::Exception &ex) {
						throw;
					} catch (PduListenerException &ex) {
						log.error("testSmsForItself: PduListenerException : %s", ex.what());
						CPPUNIT_FAIL("Test Error: PduListenerException in testSmsForItself()");
					} catch (std::exception &ex) {
						log.error("testSmsForItself: std::exception : %s, type=%s", ex.what(), typeid(ex).name());
						CPPUNIT_FAIL("Test Error: std::exception in testSmsForItself()");
					}
				}
			};
			
			bool SmsTest::configInited = false;
		}	// namespace smpp
	}	// namespace test
}	// namespace smsc

int main( int argc, char **argv) {
	bool wasSucessful = false;
	try {
		CppUnit::TextUi::TestRunner runner;
		runner.addTest(smsc::test::smpp::SmsTest::suite());
		std::ofstream fout("SmsTest.res");
		smsc::test::smpp::CsvOutputter csvOutputter(&runner.result(), fout);
		runner.setOutputter(&csvOutputter);
		wasSucessful = runner.run();
		smsc::util::xml::TerminateXerces();   
	} catch (std::exception &ex) {
		std::cout << "Test Wrong: Unexpected error !!!\n" << ex.what();
	} catch (...) {
		std::cout << "Test Wrong: Unexpected error !!!\n";
	}
	return wasSucessful;
}

