// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

#include <util/smstext.h>
#include "LongSmsTest.hpp"
#include "TextCoding.hpp"
#include "UDH.hpp"

namespace smsc {
  namespace test {
    namespace smpp {

      void LongSmsTest::testLongSms() {
        log.debug("testLongSms: --- enter");
        // binding as transmitter
        uint32_t sequence = sender->bind(smsc::sme::BindType::Transmitter);
        if(sender->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testLongSms: error in response for transmitter bind pdu");
        }

        // test
        test();

        // unbinding transmitter
        sequence = sender->unbind();
        if(sender->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testLongSms: error when sender unbinding");
        }

        log.debug("testLongSms: --- exit");
      }

      void LongSmsTest::test() {
        log.debug("test: --- enter");
        // sender sends SMS to receiver
        log.debug("test: sending SMS to receiver");
        smsc::sms::SMS sms;
        try {
          smsc::sms::Address origAddr(sender->getConfig().origAddr.c_str());
          sms.setOriginatingAddress(origAddr);
          sms.setDestinationAddress(mapProxyAddr);
        } catch(...) {
          CPPUNIT_FAIL("test: Invalid source address");
        }
        char msc[]="";
        char imsi[]="";
        sms.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
        sms.setValidTime(0);
        sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS, 3); //store and forward
        sms.setDeliveryReport(0);
        sms.setArchivationRequested(false);
        sms.setIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE, 0);
        sms.setEServiceType("XXX");

        //char *msg = "|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\";
        //char *msg = "                                                                                                                                                                                                                                                                                                                                                                                                      ";
        //char *msg = " 1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111 11111111111111111111";
        //char *msg = "     111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
        //char *msg = "1111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111";
        char *stopCmd = "stop";
        log.debug("test: original message length = %d", strlen(message.c_str()));

        /*
        // coding binary message 
        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::BINARY);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, message.c_str(), strlen(message.c_str()));
        uint32_t sequence = sender->sendSubmitSms(sms);//send the message
        if(!sender->checkResponse(sequence, timeout)) {
          CPPUNIT_FAIL("test: sender sent binary SMS but did not receive any response");
        }
        // coding smsc7bit message 
        Smsc7BitText smsc7bit(message.c_str());
        log.debug("test: smsc7bit message length = %d", smsc7bit.getLength());
        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::SMSC7BIT);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, smsc7bit.getSmsc7Bit(), smsc7bit.getLength());
        sequence = sender->sendSubmitSms(sms);//send the message
        if(!sender->checkResponse(sequence, timeout)) {
          CPPUNIT_FAIL("test: sender sent smsc7bit SMS but did not receive any response");
        }
        // coding latin1 message 
        Latin1Text latin1(message.c_str());
        log.debug("test: latin1 message length = %d", latin1.getLength());
        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::LATIN1);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, latin1.getLatin1(), latin1.getLength());
        sequence = sender->sendSubmitSms(sms);//send the message
        if(!sender->checkResponse(sequence, timeout)) {
          CPPUNIT_FAIL("test: sender sent latin1 SMS but did not receive any response");
        }
        */
        
        // coding ucs2 message 
        Ucs2Text ucs2(message.c_str());
        log.debug("test: ucs2 message length = %d", ucs2.getLength());
        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::UCS2);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, (char*)ucs2.getUcs2(), 2*ucs2.getLength());
        uint32_t sequence = sender->sendSubmitSms(sms);//send the message
        if(!sender->checkResponse(sequence, timeout)) {
          CPPUNIT_FAIL("test: sender sent ucs2 SMS but did not receive any response");
        }

        /*sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::BINARY);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, stopCmd, strlen(stopCmd));
        sequence = sender->sendSubmitSms(sms);//send stop command
        if(!sender->checkResponse(sequence, timeout)) {
          CPPUNIT_FAIL("test: sender sent stop SMS but did not receive any response");
        }*/

        // receive SMS
        log.debug("test: receiving SMS from sender");
        bool res = mapProxy.receiveAndCheck(message.c_str());
        if(res == false) {
          CPPUNIT_FAIL("test: error when checking sms received by map proxy");
        }

        // sender checks responses from SMSC
        log.debug("test: sender checks responses from SMSC");
        if(sender->checkAllResponses() == false) {
          CPPUNIT_FAIL("test: error in responses for submit_sm sent");
        }
        log.debug("test: --- exit");
      }

    }//namespace smpp
  }//namespace test
}//namespace smsc


