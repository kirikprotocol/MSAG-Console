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
        // binding map proxy as receiver
        sequence = mapProxy->bind(smsc::sme::BindType::Receiver);
        if(mapProxy->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testLongSms: error in response for receiver bind pdu");
        }

        // test with transmitter and receiver
        test();

        // unbinding transmitter
        sequence = sender->unbind();
        if(sender->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testLongSms: error when sender unbinding");
        }
        // unbinding map proxy
        sequence = mapProxy->unbind();
        if(mapProxy->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testLongSms: error when receiver unbinding");
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
          smsc::sms::Address destAddr(mapProxy->getConfig().origAddr.c_str());
          sms.setDestinationAddress(destAddr);
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

        char *msg = "|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\|^{}[]\\";
        char *stopCmd = "stop";
        log.debug("test: original message length = %d", strlen(msg));

        // coding binary message 
        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::BINARY);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, msg, strlen(msg));
        sender->sendSubmitSms(sms);//send the message
        // coding smsc7bit message 
        Smsc7BitText smsc7bit(msg);
        log.debug("test: smsc7bit message length = %d", smsc7bit.getLength());
        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::SMSC7BIT);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, smsc7bit.getSmsc7Bit(), smsc7bit.getLength());
        sender->sendSubmitSms(sms);//send the message
        // coding latin1 message 
        Latin1Text latin1(msg);
        log.debug("test: latin1 message length = %d", latin1.getLength());
        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::LATIN1);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, latin1.getLatin1(), latin1.getLength());
        sender->sendSubmitSms(sms);//send the message
        // coding ucs2 message 
        Ucs2Text ucs2(msg);
        log.debug("test: ucs2 message length = %d", ucs2.getLength());
        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::UCS2);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, (char*)ucs2.getUcs2(), 2*ucs2.getLength());
        sender->sendSubmitSms(sms);//send the message

        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, smsc::smpp::DataCoding::BINARY);
        sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, stopCmd, strlen(stopCmd));
        sender->sendSubmitSms(sms);//send stop command

        // receive SMS
        log.debug("test: receiving SMS from sender");
        bool proceed = true;
        while(proceed) {
          smsc::sms::SMS receivedSms;
          if(!mapProxy->receiveSms(timeout, receivedSms)) {
            log.debug("test: receiver did not receive SMS");
            CPPUNIT_FAIL("test: receiver did not receive SMS");
          } else {
            //extracting SMS message
            unsigned length = receivedSms.getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
            const char *data;
            if(length != 0) {//сообщение в body
              data = receivedSms.getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length);
            } else {//payload
              data = receivedSms.getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length);
            }
            uint8_t esmClass = receivedSms.getIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS);
            if(esmClass & 0x40) {//есть UDH
              UDH udh(data);
              log.debug("test: SMS contains UDH, UDH.check()=%d", udh.check());
              std::string s = udh.toString();
              log.debug("test:\n%s", s.c_str());
            } else {
              log.debug("test: SMS does not contain UDH");
            }
            log.debug("test: received SMS, length = %d", length);
            switch(receivedSms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING)) {
            case smsc::smpp::DataCoding::SMSC7BIT: {
                Smsc7BitText smsc7bit(data, length);
                Latin1Text latin1(smsc7bit);
                log.debug("test: received SMS, msg = %s", latin1.getLatin1());
                break;
              }
            case smsc::smpp::DataCoding::BINARY: {
                if(length == strlen(stopCmd)) {
                  bool compare = true;
                  for(int i=0; i<strlen(stopCmd); i++) {
                    if(data[i] != stopCmd[i]) {
                      compare = false;
                      break;
                    }
                  }
                  if(compare) {
                    proceed = false;
                  }
                }
                break;
              }
            }
          }
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

