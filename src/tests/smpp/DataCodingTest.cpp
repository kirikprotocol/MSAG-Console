// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

#include <util/smstext.h>
#include "DataCodingTest.hpp"

namespace smsc {
  namespace test {
    namespace smpp {

      uint8_t DataCodingTest::dataCodingConstants[4] = {
        smsc::smpp::DataCoding::BINARY,
        smsc::smpp::DataCoding::LATIN1,
        smsc::smpp::DataCoding::SMSC7BIT,
        smsc::smpp::DataCoding::UCS2
      };

      uint8_t DataCodingTest::profileDataCodingConstants[4]= {
        SmppProfileManager::CP_DEFAULT,
        SmppProfileManager::CP_LATIN1,
        SmppProfileManager::CP_LATIN1_UCS2,
        SmppProfileManager::CP_UCS2
      };

      const char *DataCodingTest::symbols = "`1234567890-=\\qwertyuiop[]asdfghjkl;\'zxcvbnm,./~!@#$%^&*()_+|QWERTYUIOP{}ASDFGHJKL:\"ZXCVBNM<>?¸¹/éöóêåíãøùçõúôûâàïðîëäæýÿ÷ñìèòüáþÉÖÓÊÅÍÃØÙÇÕÚÔÛÂÀÏÐÎËÄÆÝß×ÑÌÈÒÜÁÞ";
      //const char *DataCodingTest::symbols = "{}";
      //const char *DataCodingTest::symbols = "ÀÁÂ";

      void DataCodingTest::testDataCoding() {
        log.debug("testDataCoding: --- enter");
        // binding as transmitter
        uint32_t sequence = sender->bind(smsc::sme::BindType::Transmitter);
        if(sender->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testDataCoding: error in response for transmitter bind pdu");
        }
        // binding as receiver
        sequence = receiver->bind(smsc::sme::BindType::Receiver);
        if(receiver->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testDataCoding: error in response for receiver bind pdu");
        }

        // test with transmitter and receiver
        log.debug("\n\n ********* testDataCoding: test with transmitter and receiver");
        test();

        // unbinding transmitter
        sequence = sender->unbind();
        if(sender->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testDataCoding: error when sender unbinding");
        }
        // unbinding receiver
        sequence = receiver->unbind();
        if(receiver->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testDataCoding: error when receiver unbinding");
        }

        // binding as transceiver
        sequence = sender->bind(smsc::sme::BindType::Transceiver);
        if(sender->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testDataCoding: error in response for transceiver bind pdu");
        }
        // binding as transceiver
        sequence = receiver->bind(smsc::sme::BindType::Transceiver);
        if(receiver->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testDataCoding: error in response for transceiver bind pdu");
        }

        // test with two transceivers
        log.debug("\n\n ********* testDataCoding: test with two transceivers");
        test();

        // unbinding sender
        sequence = sender->unbind();
        if(sender->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testDataCoding: error when sender unbinding");
        }
        // unbinding receiver
        sequence = receiver->unbind();
        if(receiver->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("testDataCoding: error when receiver unbinding");
        }
        log.debug("testDataCoding: --- exit");
      }

      void DataCodingTest::test(uint8_t dataCoding, uint8_t profileDataCoding) {
        if(log.isDebugEnabled()) {
          char *dc;
          char *pdc;
          switch(dataCoding) {
          case smsc::smpp::DataCoding::BINARY:
            dc = "DataCoding::BINARY";
            break;
          case smsc::smpp::DataCoding::LATIN1:
            dc = "DataCoding::LATIN1";
            break;
          case smsc::smpp::DataCoding::SMSC7BIT:
            dc = "DataCoding::SMSC7BIT";
            break;
          case smsc::smpp::DataCoding::UCS2:
            dc = "DataCoding::UCS2";
            break;
          }
          switch(profileDataCoding) {
          case SmppProfileManager::CP_DEFAULT:
            pdc = "ProfileDataCoding::CP_DEFAULT";
            break;
          case SmppProfileManager::CP_LATIN1:
            pdc = "ProfileDataCoding::CP_LATIN1";
            break;
          case SmppProfileManager::CP_LATIN1_UCS2:
            pdc = "ProfileDataCoding::CP_LATIN1_UCS2";
            break;
          case SmppProfileManager::CP_UCS2:
            pdc = "ProfileDataCoding::CP_UCS2";
            break;
          }
          log.debug("\n\n ********* test(%s, %s): --- enter", dc, pdc);
        }
        if(!profileManager->setProfile((SmppProfileManager::ProfileCommand) profileDataCoding)) {
          CPPUNIT_FAIL("test: Error when setting profile");
        }
        for(int mode = 3; mode > 0; mode--) {
          switch(mode) {
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
          log.debug("test: sending SMS to receiver");
          smsc::sms::SMS sms;
          try {
            smsc::sms::Address origAddr(sender->getConfig().origAddr.c_str());
            sms.setOriginatingAddress(origAddr);
            smsc::sms::Address destAddr(receiver->getConfig().origAddr.c_str());
            sms.setDestinationAddress(destAddr);
          } catch(...) {
            CPPUNIT_FAIL("test: Invalid source address");
          }
          char msc[]="";
          char imsi[]="";
          sms.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
          sms.setValidTime(0);
          sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS, mode); 
          sms.setDeliveryReport(0);
          sms.setArchivationRequested(false);
          sms.setIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE, 0);
          sms.setEServiceType("XXX");
          // coding message 
          sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, dataCoding);
          switch(dataCoding) {
          case smsc::smpp::DataCoding::BINARY: {
              log.debug("test: DataCoding::BINARY");
              sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, symbols, strlen(symbols));
            }
            break;
          case smsc::smpp::DataCoding::LATIN1: {
              log.debug("test: DataCoding::LATIN1, length=%d, dc=%d", latin1Msg->getLength(), dataCoding);
              sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, latin1Msg->getLatin1(), latin1Msg->getLength());
            }
            break;
          case smsc::smpp::DataCoding::SMSC7BIT: {
              sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, smsc7bitMsg->getSmsc7Bit(), smsc7bitMsg->getLength());
            }
            break;
          case smsc::smpp::DataCoding::UCS2: {
            log.debug("test: DataCoding::UCS2, 2*ucs2Msg->getLength() = %d", 2*ucs2Msg->getLength());
              sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, (char*)ucs2Msg->getUcs2(), 2*ucs2Msg->getLength());
            }
            break;
          }
          uint32_t submitSeq = sender->sendSubmitSms(sms);
          uint32_t dataSeq = sender->sendDataSms(sms);

          // receive SMS
          bool isEqual = false;
          log.debug("test: receiving SMS from sender");
          smsc::sms::SMS receivedSms;
          if(!receiver->receiveSms(timeout, receivedSms)) {
            log.debug("test: receiver did not receive any SMS");
            CPPUNIT_FAIL("test: receiver did not receive any SMS");
          } else {
            //extracting SMS message
            unsigned length = receivedSms.getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
            const char *data;
            if(length != 0) {//ñîîáùåíèå â body
              data = receivedSms.getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length);
            } else {//payload
              data = receivedSms.getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length);
            }
            isEqual = compare(dataCoding, profileDataCoding, data, length);
          }

          smsc::sms::SMS receivedSms1;
          if(!receiver->receiveSms(timeout, receivedSms1)) {
            log.debug("test: receiver did not receive the second SMS");
            CPPUNIT_FAIL("test: receiver did not receive the second SMS");
          } else {
            //extracting SMS message
            unsigned length = receivedSms1.getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
            const char *data;
            if(length != 0) {//ñîîáùåíèå â body
              data = receivedSms1.getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length);
            } else {//payload
              data = receivedSms1.getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length);
            }
            isEqual = compare(dataCoding, profileDataCoding, data, length);
          }

          // sender checks responses from SMSC
          if(sender->checkResponse(submitSeq, timeout) == false) {
            CPPUNIT_FAIL("test: error in response for submit_sm sent");
          }

          // sender checks responses from SMSC
          if(sender->checkResponse(dataSeq, timeout) == false) {
            CPPUNIT_FAIL("test: error in response for data_sm sent");
          }

          if(!isEqual) {
            log.error("test: sender received SMS that is not equal an original SMS sent, dc=%d, pdc=%d", dataCoding, profileDataCoding);
            CPPUNIT_FAIL("test: sender received SMS that is not equal an original SMS sent");
          }
          log.debug("test: --- exit");
        }
      }

      bool DataCodingTest::compare(uint8_t dataCoding, uint8_t profileDataCoding, const char * const buf, int size) {
        bool res = false;
        if(dataCoding == smsc::smpp::DataCoding::BINARY) {//äîëæíî ïðèéòè òîæå ñàìîå
          res = compare(buf, size, symbols, strlen(symbols));
        } else {
          switch(profileDataCoding) {
          case SmppProfileManager::CP_DEFAULT:
            if(dataCoding == smsc::smpp::DataCoding::UCS2) { //SMSC7BIT+Translit
              Smsc7BitText smsc7bit(*ucs2Msg);
              res = compare(buf, size, smsc7bit.getSmsc7Bit(), smsc7bit.getLength());
            } else {
              res = compare(buf, size, smsc7bitMsg->getSmsc7Bit(), smsc7bitMsg->getLength());
            }
            break;
          case SmppProfileManager::CP_LATIN1:
            if(dataCoding == smsc::smpp::DataCoding::SMSC7BIT) {
              res = compare(buf, size, smsc7bitMsg->getSmsc7Bit(), smsc7bitMsg->getLength());
            } else {
              res = compare(buf, size, latin1Msg->getLatin1(), latin1Msg->getLength());
            }
            break;
          case SmppProfileManager::CP_UCS2:
            if(dataCoding == smsc::smpp::DataCoding::UCS2) {
              res = compare(buf, size, (char*)ucs2Msg->getUcs2(), 2*ucs2Msg->getLength());
            } else {
              res = compare(buf, size, smsc7bitMsg->getSmsc7Bit(), smsc7bitMsg->getLength());
            }
            break;
          case SmppProfileManager::CP_LATIN1_UCS2:
            if(dataCoding == smsc::smpp::DataCoding::UCS2) {
              res = compare(buf, size, (char*)ucs2Msg->getUcs2(), 2*ucs2Msg->getLength());
            } else if(dataCoding == smsc::smpp::DataCoding::LATIN1) {
              res = compare(buf, size, latin1Msg->getLatin1(), latin1Msg->getLength());
            } else if(dataCoding == smsc::smpp::DataCoding::SMSC7BIT) {
              res = compare(buf, size, smsc7bitMsg->getSmsc7Bit(), smsc7bitMsg->getLength());
            }
            break;
          }
        }

        return res;
      }

      bool DataCodingTest::compare(const char * const buf1, int size1, const char * const buf2, int size2) {
        bool res = true;
        if(size1 == size2) {
          for(int i=0; i<size1; i++) {
            if(buf1[i] != buf2[i]) {
              res = false;
              break;
            }
          }
        } else {
          res = false;
        }

        if(log.isDebugEnabled () && !res) {
          log.debug("Comparing is false, size1=%d, size2=%d", size1, size2);
          for(int i=0; i<size1; i++) {
            log.debug("buf1[%d]=%x ", i, buf1[i]);
          }
          log.debug("\n");
          for(int i=0; i<size2; i++) {
            log.debug("buf2[%d]=%x ", i, buf2[i]);
          }
        }
        return res;
      }

    }//namespace smpp
  }//namespace test
}//namespace smsc
