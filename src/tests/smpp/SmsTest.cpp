#include <iostream>
#include <fstream>
#include <vector>

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
#include "SmppProfileManager.hpp"
#include "DataCodingTest.hpp"
#include "LongSmsTest.hpp"

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
        smsc::logger::Logger log;
        std::string message;
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
          suiteOfTests->addTest(createTestCaller(TestSpecification::SMS_TEST_test_sms_for_transmitter_receiver, &SmsTest::testSmsForTransmitterReceiver));
          suiteOfTests->addTest(createTestCaller(TestSpecification::SMS_TEST_test_sms_for_transceiver_itself, &SmsTest::testSmsForTransceiverItself));
          suiteOfTests->addTest(createTestCaller(TestSpecification::SMS_TEST_test_smpp_profiler, &SmsTest::testSmppProfiler));
          suiteOfTests->addTest(createTestCaller(TestSpecification::SMS_TEST_test_data_coding, &SmsTest::testDataCoding));
          suiteOfTests->addTest(createTestCaller(TestSpecification::SMS_TEST_test_long_sms, &SmsTest::testLongSms));
          return suiteOfTests;
        }

        SmsTest() : log(smsc::test::util::logger.getLog("smsc.test.smpp.SmsTest"))
        , timeout(30000) {
        }

        void setUp() {
          log.info("enter: setUp Test Suit");
          ContextConfiguratorHandler config = ContextConfigurator::instance();
          try {
            if (!configInited) {
              config->loadContext("sms_test.xml");
              configInited = true;
            }
            timeout = getContextIntValue("smsc.timeout");
            log.debug("timeout=%d", timeout);
            message = getContextStringValue("smsc.sms-message");
            log.debug("message=%s", message.c_str());
          } catch (ConfigurationException &ex) {
            std::cout <<  std::endl << ex.what();
            CPPUNIT_FAIL("ConfigurationException has occured");
          }
          log.info("exit: Test suit has been set up");
        }

        void tearDown() {
          log.info("method tearDown");
        }

      protected:
        void testSmppProfiler() {
          log.error("testSmppProfiler: ---> enter");
          try {
            QueuedSmeHandler sme = createSme("smsc.profile-manager.sme");
            SmppProfileManager profileManager(sme);
            std::string profilerAddr = getContextStringValue("smsc.profile-manager.profiler-address");
            profileManager.setProfilerAddress(profilerAddr.c_str());
            for (int i=SmppProfileManager::REPORT_NONE; i<=SmppProfileManager::UNHIDE_SENDER; i++) {
              if (i != SmppProfileManager::LOCALE_RU) {
                if(!profileManager.setProfile((SmppProfileManager::ProfileCommand)i, smsc::smpp::DataCoding::BINARY)) {
                  log.error("testSmppProfiler: error when setting profile");
                  CPPUNIT_FAIL("testSmppProfiler: error when setting profile");
                }
                if(!profileManager.setProfile((SmppProfileManager::ProfileCommand)i, smsc::smpp::DataCoding::LATIN1)) {
                  log.error("testSmppProfiler: error when setting profile");
                  CPPUNIT_FAIL("testSmppProfiler: error when setting profile");
                }
                if (!profileManager.setProfile((SmppProfileManager::ProfileCommand)i, smsc::smpp::DataCoding::SMSC7BIT)) {
                  log.error("testSmppProfiler: error when setting profile");
                  CPPUNIT_FAIL("testSmppProfiler: error when setting profile");
                }
                if(!profileManager.setProfile((SmppProfileManager::ProfileCommand)i, smsc::smpp::DataCoding::UCS2)) {
                  log.error("testSmppProfiler: error when setting profile");
                  CPPUNIT_FAIL("testSmppProfiler: error when setting profile");
                }
              }
            }
          } catch (CppUnit::Exception &ex) {
            throw;
          } catch (SmppException &ex) {
            log.error("testSmppProfiler: SmppException : %s", ex.what());
            CPPUNIT_FAIL("Test Error: SmppException in testSmppProfiler()");
          } catch (std::exception &ex) {
            log.error("testSmppProfiler: std::exception : %s, type=%s", ex.what(), typeid(ex).name());
            CPPUNIT_FAIL("Test Error: std::exception in testSmppProfiler()");
          }
          log.error("testSmppProfiler: ---> exit");
        }

        void testDataCoding() {
          log.error("testDataCoding: ---> enter");
          try {
            // creating profile manager
            QueuedSmeHandler sme = createSme("smsc.profile-manager.sme");
            SmppProfileManagerHandler profileManager = new SmppProfileManager(sme);
            std::string profilerAddr = getContextStringValue("smsc.profile-manager.profiler-address");
            profileManager->setProfilerAddress(profilerAddr.c_str());
            // creating sender and receiver
            QueuedSmeHandler sender = createSmeByName("smsc.sme.sender");
            QueuedSmeHandler receiver = createSmeByName("smsc.sme.receiver");

            DataCodingTest test(sender, receiver, profileManager, timeout);
            test.testDataCoding();

          } catch (CppUnit::Exception &ex) {
            throw;
          } catch (SmppException &ex) {
            log.error("testDataCoding: SmppException : %s", ex.what());
            CPPUNIT_FAIL("Test Error: SmppException in testDataCoding()");
          } catch (std::exception &ex) {
            log.error("testDataCoding: std::exception : %s, type=%s", ex.what(), typeid(ex).name());
            CPPUNIT_FAIL("Test Error: std::exception in testDataCoding()");
          }
          log.error("testDataCoding: ---> exit");
        }

        void testLongSms() {
          log.error("testLongSms: ---> enter");
          try {
            // creating map proxy sme
            QueuedSmeHandler mapProxySme = createSmeByName("smsc.sme.map-proxy");
            // creating profile manager
            QueuedSmeHandler sme = createSme("smsc.profile-manager.sme");
            SmppProfileManagerHandler profileManager = new SmppProfileManager(sme);
            std::string profilerAddr = getContextStringValue("smsc.profile-manager.profiler-address");
            profileManager->setProfilerAddress(profilerAddr.c_str());
            profileManager->setOriginatingAddress(mapProxySme->getConfig().origAddr.c_str());
            // creating sender
            QueuedSmeHandler sender = createSmeByName("smsc.sme.sender");
            // getting long sms message to send
            std::string msg = getContextStringValue("smsc.long-sms-message");
            log.debug("long message=%s", msg.c_str());

            //set profile for map proxy
            if(profileManager->setProfile(SmppProfileManager::CP_LATIN1_UCS2)) {
              LongSmsTest test(sender, mapProxySme, msg, timeout);
              test.testLongSms();
            } else {
              log.error("testLongSms: Error when setting profile for map proxy");
              CPPUNIT_FAIL("Test Error: when setting profile for map proxy in testLongSms");
            }

          } catch (CppUnit::Exception &ex) {
            throw;
          } catch (SmppException &ex) {
            log.error("testLongSms: SmppException : %s", ex.what());
            CPPUNIT_FAIL("Test Error: SmppException in testLongSms()");
          } catch (std::exception &ex) {
            log.error("testLongSms: std::exception : %s, type=%s", ex.what(), typeid(ex).name());
            CPPUNIT_FAIL("Test Error: std::exception in testLongSms()");
          }
          log.error("testLongSms: ---> exit");
        }

        void testSmsForTransmitterReceiver() {
          log.error("testSmsForTransmitterReceiver: ---> enter, sms=%s", message.c_str());
          try {
            QueuedSmeHandler sender = createSmeByName("smsc.sme.sender");
            QueuedSmeHandler receiver = createSmeByName("smsc.sme.receiver");
            sender->connect();
            receiver->connect();
            // binding as transmitter
            uint32_t sequence = sender->bind(smsc::sme::BindType::Transmitter);
            if (sender->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransmitterReceiver: error in response for transmitter bind pdu");
            }
            // binding as receiver
            sequence = receiver->bind(smsc::sme::BindType::Receiver);
            if (receiver->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransmitterReceiver: error in response for receiver bind pdu");
            }

            // test
            testSms(sender, receiver, message.c_str());

            // unbinding transmitter
            sequence = sender->unbind();
            if (sender->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransmitterReceiver: error when sender unbinding");
            }
            // unbinding receiver
            sequence = receiver->unbind();
            if (receiver->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransmitterReceiver: error when receiver unbinding");
            }

          } catch (CppUnit::Exception &ex) {
            throw;
          } catch (SmppException &ex) {
            log.error("testSmsForTransmitterReceiver: SmppException : %s", ex.what());
            CPPUNIT_FAIL("Test Error: SmppException in testSmsForTransmitterReceiver()");
          } catch (std::exception &ex) {
            log.error("testSmsForTransmitterReceiver: std::exception : %s, type=%s", ex.what(), typeid(ex).name());
            CPPUNIT_FAIL("Test Error: std::exception in testSmsForTransmitterReceiver()");
          }
          log.error("testSmsForTransmitterReceiver: ---> exit");
        }

        void testSmsForTransmitterReceiverItself() {
          log.error("testSmsForTransmitterReceiverItself: ---> enter, sms=%s", message.c_str());
          try {
            QueuedSmeHandler sender = createSmeByName("smsc.sme.sender");
            QueuedSmeHandler receiver = createSmeByName("smsc.sme.sender");//the same
            sender->connect();
            receiver->connect();
            // binding as transmitter
            uint32_t sequence = sender->bind(smsc::sme::BindType::Transmitter);
            if (sender->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransmitterReceiver: error in response for transmitter bind pdu");
            }
            // binding as receiver
            sequence = receiver->bind(smsc::sme::BindType::Receiver);
            if (receiver->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransmitterReceiver: error in response for receiver bind pdu");
            }

            //test
            testSms(sender, receiver, message.c_str());

            // unbinding transmitter
            sequence = sender->unbind();
            if (sender->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransmitterReceiver: error when sender unbinding");
            }
            // unbinding receiver
            sequence = receiver->unbind();
            if (receiver->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransmitterReceiver: error when receiver unbinding");
            }

          } catch (CppUnit::Exception &ex) {
            throw;
          } catch (SmppException &ex) {
            log.error("testSmsForTransmitterReceiver: SmppException : %s", ex.what());
            CPPUNIT_FAIL("Test Error: SmppException in testSmsForTransmitterReceiver()");
          } catch (std::exception &ex) {
            log.error("testSmsForTransmitterReceiver: std::exception : %s, type=%s", ex.what(), typeid(ex).name());
            CPPUNIT_FAIL("Test Error: std::exception in testSmsForTransmitterReceiver()");
          }
          log.error("testSmsForTransmitterReceiverItself: ---> exit");
        }

        void testSms(QueuedSmeHandler sender, QueuedSmeHandler receiver, const char *message) {
          try {
            for (int mode = 1; mode < 4; mode++) {
              switch (mode) {
                case 0:
                  log.debug("\n\n******** DEFAULT MESSAGE MODE ********\n");
                  break;
                case 1:
                  log.debug("\n\n******** DATAGRAM MESSAGE MODE ********\n");
                  break;
                case 2:
                  log.debug("\n\n******** TRANSACTION MESSAGE MODE ********\n");
                  break;
                case 3:
                  log.debug("\n\n******** STORE AND FORWARD MESSAGE MODE ********\n");
                  break;
              }
              // sender sends SMS to receiver
              log.debug("testSmsForTransmitterReceiver: sending SMS to receiver");
              smsc::smpp::PduSubmitSm submit;
              submit.get_header().set_commandId(smsc::smpp::SmppCommandSet::SUBMIT_SM);
              smsc::sms::SMS sms;
              try {
                smsc::sms::Address origAddr(sender->getConfig().origAddr.c_str());
                sms.setOriginatingAddress(origAddr);
                smsc::sms::Address destAddr(receiver->getConfig().origAddr.c_str());
                sms.setDestinationAddress(destAddr);
              } catch (...) {
                CPPUNIT_FAIL("testSmsForTransmitterReceiver: Invalid source address");
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
              uint32_t length = strlen(message);
              sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING,smsc::smpp::DataCoding::BINARY);
              sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, message, length);
              smsc::smpp::fillSmppPduFromSms(&submit, &sms);
              uint32_t sequence = sender->sendPdu((smsc::smpp::SmppHeader*)&submit);

              // receive SMS
              log.debug("testSmsForTransmitterReceiver: receiving SMS from sender");
              PduHandler pdu = receiver->receive(timeout);
              if (pdu != 0) {
                if (pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM) {
                  // send deliver_sm_resp
                  PduDeliverySmResp resp;
                  resp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DELIVERY_SM_RESP);
                  resp.set_messageId("");
                  resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
                  resp.get_header().set_commandStatus(smsc::smpp::SmppStatusSet::ESME_ROK);
                  receiver->sendDeliverySmResp(resp);

                  // check PDU received
                  smsc::sms::SMS receivedSms;
                  smsc::smpp::PduXSm *pduXSM = (smsc::smpp::PduXSm*) pdu.getObjectPtr();
                  smsc::smpp::fetchSmsFromSmppPdu(pduXSM, &receivedSms);
                  if (log.isDebugEnabled()) {
                    log.debug("testSmsForTransmitterReceiver: SMS originating address = %s", receivedSms.getOriginatingAddress().toString().c_str());
                    log.debug("testSmsForTransmitterReceiver: SMS destination address = %s", receivedSms.getDestinationAddress().toString().c_str());
                    log.debug("testSmsForTransmitterReceiver: SMS data coding = %d", receivedSms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING));
                    log.debug("testSmsForTransmitterReceiver: SMS user message reference = %d", receivedSms.getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE));
                  }
                  bool isEqual = false;
                  char buf[256];
                  if (smsc::util::getPduText(pduXSM, buf, sizeof(buf))==-1) {
                    int sz=(pduXSM)->optional.size_messagePayload();
                    char *data = new char[sz*2];
                    if (smsc::util::getPduText(pduXSM, data, sz*2)!=-1) {
                      log.debug("testSmsForTransmitterReceiver: SMS message(payload) = %s", data);
                      isEqual = (strcmp(message, data) == 0);
                    } else {
                      log.debug("testSmsForTransmitterReceiver: faield to retrieve SMS message");
                    }
                    delete [] data;
                  } else {
                    log.debug("testSmsForTransmitterReceiver: SMS message = %s", buf);
                    isEqual = (strcmp(message, buf) == 0);
                  }
                  if (!isEqual) {
                    log.error("testSmsForTransmitterReceiver: sender received SMS that is not equal an original SMS sent");
                    CPPUNIT_FAIL("testSmsForTransmitterReceiver: sender received SMS that is not equal an original SMS sent");
                  }
                } else {
                  log.debug("testSmsForTransmitterReceiver: receiver received unexpected PDU with commandId=%x", pdu->get_commandId());
                  CPPUNIT_FAIL("testSmsForTransmitterReceiver: receiver received unexpected PDU");
                }
              } else {
                CPPUNIT_FAIL("testSmsForTransmitterReceiver: receiver did not receive delivery_sm");
              }

              // sender checks response from SMSC
              if (sender->checkResponse(sequence, timeout) == false) {
                CPPUNIT_FAIL("testSmsForTransmitterReceiver: error in response for submit_sm sent");
              }
            }

          } catch (CppUnit::Exception &ex) {
            throw;
          } catch (SmppException &ex) {
            log.error("testSmsForTransmitterReceiver: SmppException : %s", ex.what());
            CPPUNIT_FAIL("Test Error: SmppException in testSmsForItself()");
          } catch (std::exception &ex) {
            log.error("testSmsForTransmitterReceiver: std::exception : %s, type=%s", ex.what(), typeid(ex).name());
            CPPUNIT_FAIL("Test Error: std::exception in testSmsForItself()");
          }
        }

        void testSmsForTransceiverItself() {
          log.error("testSmsForTransceiverItself: ---> enter, sms=%s", message.c_str());
          try {
            QueuedSmeHandler trx = createSmeByName("smsc.sme.sender");
            trx->connect();
            // binding as transceiver
            uint32_t sequence = trx->bind(smsc::sme::BindType::Transceiver);
            if (trx->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransceiverItself: error in response for transceiver bind pdu");
            }

            //test
            testSms(trx, trx, message.c_str());

            // unbinding transceiver
            sequence = trx->unbind();
            if (trx->checkResponse(sequence, timeout)== false) {
              CPPUNIT_FAIL("testSmsForTransceiverItself: error when transceiver unbinding");
            }

          } catch (CppUnit::Exception &ex) {
            throw;
          } catch (SmppException &ex) {
            log.error("testSmsForTransmitterReceiver: SmppException : %s", ex.what());
            CPPUNIT_FAIL("Test Error: SmppException in testSmsForTransmitterReceiver()");
          } catch (std::exception &ex) {
            log.error("testSmsForTransmitterReceiver: std::exception : %s, type=%s", ex.what(), typeid(ex).name());
            CPPUNIT_FAIL("Test Error: std::exception in testSmsForTransmitterReceiver()");
          }
          log.error("testSmsForTransmitterReceiverItself: ---> exit");
        }

      private:
        int getContextIntValue(const char *contextPath) {
          int res;
          std::vector<std::string> contexts = parseString(contextPath);
          unsigned int size = contexts.size();
          ContextConfiguratorHandler config = ContextConfigurator::instance();
          ContextHandler ctx = config->getContext(contexts[0]);
          for (unsigned int i=1; i<size; i++) {
            ctx = ctx->firstSubcontext(contexts[i]);
          }
          res = atoi(ctx->getValue().c_str());
          return res;
        }

        std::string getContextStringValue(const char *contextPath) {
          std::string res;
          std::vector<std::string> contexts = parseString(contextPath);
          unsigned int size = contexts.size();
          ContextConfiguratorHandler config = ContextConfigurator::instance();
          ContextHandler ctx = config->getContext(contexts[0]);
          for (unsigned int i=1; i<size; i++) {
            ctx = ctx->firstSubcontext(contexts[i]);
          }
          res = ctx->getValue();
          return res;
        }

        GenericSmeHandler createSme(const char *contextPath) {
          std::vector<std::string> contexts = parseString(contextPath);
          unsigned int size = contexts.size();
          ContextConfiguratorHandler config = ContextConfigurator::instance();
          ContextHandler ctx = config->getContext(contexts[0]);
          for (unsigned int i=1; i<size; i++) {
            ctx = ctx->firstSubcontext(contexts[i]);
          }
          
          return SmeFactory::createSme(ctx);
        }

        GenericSmeHandler createSmeByName(const char *contextPath) {
          GenericSmeHandler sme;
          std::vector<std::string> contexts = parseString(contextPath);
          unsigned int size = contexts.size();
          std::string name = contexts[size-1];
          ContextConfiguratorHandler config = ContextConfigurator::instance();
          ContextHandler ctx = config->getContext(contexts[0]);
          for (unsigned int i=1; i<size-2; i++) {
            ctx = ctx->firstSubcontext(contexts[i]);
          }
          smsc::test::util::ContextHandlerList lst = ctx->findSubcontext(contexts[size-2]);
          typedef smsc::test::util::ContextHandlerList::iterator CHI;
          for (CHI itr = lst.begin(); itr != lst.end(); ++itr) {
            ctx  = *itr;
            //log.debug("Performing sme context,  name=%s", ctx->getName().c_str());
            string smeName = ctx->getAttribute("name");
            if (smeName == name) {
              log.debug("Creating \"%s\" by using SmeFactory", name.c_str());
              sme = SmeFactory::createSme(ctx);
              break;
            }
          }

          return sme;
        }

        std::vector<std::string> parseString(const char *str) {
          std::vector<std::string> res;
          std::string string(str);
          unsigned int pos=0;
          for (unsigned int i=0; i<strlen(str); i++) {
            if (str[i] == '.') {
              res.push_back(string.substr(pos, i-pos));
              pos = i+1;
            }
          }
          if (pos < strlen(str)) {
            res.push_back(string.substr(pos, strlen(str)-pos));
          }

          return res;
        }
      };

      bool SmsTest::configInited = false;
    } // namespace smpp
  } // namespace test
} // namespace smsc

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

