// CPPUnit includes
#include <cppunit/extensions/HelperMacros.h>

#include "UDH.hpp"
#include "MapProxy.hpp"
#include "TextCoding.hpp"

namespace smsc {
  namespace test {
    namespace smpp {

      MapProxy::MapProxy(QueuedSmeHandler mapProxy_, uint32_t timeout_) 
      : mapProxy(mapProxy_), timeout(timeout_) 
      , log(smsc::test::util::logger.getLog("smsc.test.smpp.MapProxy")) {
        mapProxy->connect();
        // binding map proxy as receiver
        uint32_t sequence = mapProxy->bind(smsc::sme::BindType::Receiver);
        if(mapProxy->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("MapProxy: error in response for map proxy bind pdu");
        }
      }

      bool MapProxy::receiveAndCheck(const char * const message) {
        log.debug("receiveAndCheck: --- enter");
        receive();
        log.debug("receiveAndCheck: --- exit");
        return check(message);
      }

      void MapProxy::receive() {
        for(;;) {
          SmsHandler sms = new smsc::sms::SMS();
          if(!mapProxy->receiveSms(timeout, *sms)) {
            log.debug("receiveAndCheck: receiver did not receive SMS");
            break;
          } else {
            uint8_t esmClass = sms->getIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS);
            if(esmClass & 0x40) {//есть UDH
              //extracting SMS message
              unsigned length = sms->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
              const char *data;
              if(length != 0) {//сообщение в body
                data = sms->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length);
              } else {//payload
                data = sms->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length);
              }
              UDH udh(data);
              log.debug("receiveAndCheck: SMS contains UDH, UDH.check()=%d", udh.check());
              std::string s = udh.toString();
              log.debug("receiveAndCheck:\n%s", s.c_str());
              uint16_t messageReference = 0;
              uint8_t messageSequence = 0;
              uint8_t messageNumber = 0;
              for(int i=0; i<udh.getElementsNumber(); i++) {
                IE ie = udh.getElements()[i];
                if(ie.getInformationElementIdentifier() == 0) {
                  log.debug("receiveAndCheck: 8-bit reference number");
                  messageReference = ie.getInformationElementData()[0];
                  messageSequence = ie.getInformationElementData()[2];
                  messageNumber = ie.getInformationElementData()[1];
                } else if(ie.getInformationElementIdentifier() == 8) {
                  log.debug("receiveAndCheck: 16-bit reference number, %d, %d",ie.getInformationElementData()[0], ie.getInformationElementData()[1]);
                  messageReference = (ie.getInformationElementData()[0]<<8) + ie.getInformationElementData()[1];
                  messageSequence = ie.getInformationElementData()[3];
                  messageNumber = ie.getInformationElementData()[2];
                }
              }
              log.debug("receiveAndCheck: messageReference=%d, messageSequence=%d, messageNumber=%d", messageReference, messageSequence, messageNumber);
              if(messageReference != 0) {
                SmsArrayHandler sah;
                if(smsArrayMap.Exist(messageReference)) {
                  sah = smsArrayMap.Get(messageReference);
                } else {
                  sah = new SmsArray(messageNumber);
                  smsArrayMap.Insert(messageReference, sah);
                }
                (*sah)[messageSequence - 1] = sms;
              }
            } else {//skip this sms because it is not truncated
              //extracting SMS message
              unsigned length = sms->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
              const char *data;
              if(length != 0) {//сообщение в body
                data = sms->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length);
              } else {//payload
                data = sms->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length);
              }
              log.debug("receiveAndCheck: SMS does not contain UDH, length=%d", length);
            }
          }
        }
      }

      MapProxy::~MapProxy() {
        // unbinding map proxy
        uint32_t sequence = mapProxy->unbind();
        if(mapProxy->checkResponse(sequence, timeout)== false) {
          CPPUNIT_FAIL("~MapProxy: error when unbinding map proxy");
        }
      }

      bool MapProxy::check(const char * const message) {
        log.debug("check: --- enter()");
        bool res = true;
        IntHash<SmsArrayHandler>::Iterator itr = smsArrayMap.First();
        int msgRef;
        SmsArrayHandler sah;
        while(itr.Next(msgRef, sah)) {
          log.debug("check: processing messages set");
          if(sah->getSize() > 0) {
            uint8_t dataCoding = (*sah)[0]->getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING);
            switch(dataCoding) {
            case smsc::smpp::DataCoding::BINARY: {
                log.debug("check: DataCoding::BINARY");
                if(!checkBinary(message, sah)) {
                  res = false;
                }
                break;
              }
            case smsc::smpp::DataCoding::LATIN1: {
                log.debug("check: DataCoding::LATIN1");
                if(!checkLatin1(message, sah)) {
                  res = false;
                }
                break;
              }
            case smsc::smpp::DataCoding::SMSC7BIT: {
                log.debug("check: DataCoding::SMSC7BIT");
                if(!checkSmsc7bit(message, sah)) {
                  res = false;
                }
                break;
              }
            case smsc::smpp::DataCoding::UCS2: {
                log.debug("check: DataCoding::UCS2");
                if(!checkUcs2(message, sah)) {
                  res = false;
                }
                break;
              }
            }
          }
        }

        log.debug("check: --- exit(), res = %d", res);
        return res;
      }

      bool MapProxy::checkBinary(const char * const message, SmsArrayHandler sah) {
        log.debug("checkBinary: --- enter");
        bool res = true;
        uint32_t lengthSum = 0;
        int size = strlen(message);
        char messageSum[size + 1];
        messageSum[size] = 0;
        for(int i=0; i<sah->getSize(); i++) {
          //extracting SMS message
          unsigned length = (*sah)[i]->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
          const char *data;
          if(length != 0) {//сообщение в body
            data = (*sah)[i]->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length);
          } else {//payload
            data = (*sah)[i]->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length);
          }
          UDH udh(data);
          length -= udh.getLength() + 1;
          data += udh.getLength() + 1;

          lengthSum += length;
          if(lengthSum < size + 1) {
            memcpy(messageSum + (lengthSum - length), data, length);
          } else {
            log.error("checkBinary: error, lengthSum >= size + 1, lengthSum = %d, size = %d", lengthSum, size);
          }
        }
        res = compare(message, strlen(message), messageSum, lengthSum);
        /*if(res) {
          for(int i=0; i<sah->getSize() -1; i++) {
            if(!analyzeBinaryCutting((*sah)[i], (*sah)[i+1])) {
              res = false;
              break;
            }
          }
        }*/
        log.debug("checkBinary: --- exit, res = %d", res);
        return res;
      }

      bool MapProxy::checkLatin1(const char * const message, SmsArrayHandler sah) {
        log.debug("checkLatin1: --- enter");
        Latin1Text latin1(message);
        bool res = true;
        uint32_t lengthSum = 0;
        char messageSum[latin1.getLength() + 1];
        messageSum[latin1.getLength()] = 0;
        for(int i=0; i<sah->getSize(); i++) {
          //extracting SMS message
          unsigned length = (*sah)[i]->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
          const char *data;
          if(length != 0) {//сообщение в body
            data = (*sah)[i]->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length);
          } else {//payload
            data = (*sah)[i]->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length);
          }
          UDH udh(data);
          length -= udh.getLength() + 1;
          data += udh.getLength() + 1;

          lengthSum += length;
          if(lengthSum < latin1.getLength() + 1) {
            memcpy(messageSum + (lengthSum - length), data, length);
          } else {
            log.error("checkLatin1: error, lengthSum >= latin1.getLength() + 1, lengthSum = %d, size = %d", lengthSum, latin1.getLength());
          }
        }
        log.debug("checkLatin1: lengthSum = %d", lengthSum);
        res = compare(latin1.getLatin1(), latin1.getLength(), messageSum, lengthSum);
        if(res) {
          log.debug("checkLatin1: checking message pairs, total messages=%d",sah->getSize());
          for(int i=0; i<sah->getSize() -1; i++) {
            log.debug("checkLatin1: analyzing cutting for %d and %d messages", i, i+1);
            if(!analyzeLatin1Cutting((*sah)[i], (*sah)[i+1])) {
              log.debug("checkLatin1: analyze latin1 cutting is FALSE, i=%d", i);
              res = false;
              break;
            }
          }
        }
        log.debug("checkLatin1: --- exit, res = %d", res);
        return res;
      }
    
      bool MapProxy::analyzeLatin1Cutting(SmsHandler sms1, SmsHandler sms2) {
        log.debug("analyzeLatin1Cutting: --- enter");
        bool res = true;
        //extracting SMS message1
        unsigned length1 = sms1->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
        const char *data1;
        if(length1 != 0) {//сообщение в body
          data1 = sms1->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length1);
        } else {//payload
          data1 = sms1->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length1);
        }
        log.debug("analyzeLatin1Cutting: length1 = %d", length1);
        unsigned length2;
        const char *data2;
        //extracting SMS message2
        length2 = sms2->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
        if(length2 != 0) {//сообщение в body
          data2 = sms2->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length2);
        } else {//payload
          data2 = sms2->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length2);
        }
        UDH udh(data2);
        length2 -= udh.getLength() + 1;
        data2 += udh.getLength() + 1;
        log.debug("analyzeLatin1Cutting: length2 = %d", length2);
        if(length1 < 159) {
          if(data2[0] == ' ' || data2[0] == '\n' || data2[0] == '\r') {
            log.debug("analyzeLatin1Cutting: sms2 contains symbols (space) that should be added to sms1");
            res = false;
          } else if(data1[length1 -1] != ' ' && data1[length1 -1] != '\n' && data1[length1 -1] != '\r') {
            log.debug("analyzeLatin1Cutting: sms2 contains symbols (not space) that should be added to sms1");
            res = false;
          } else {
            int wordLength = length2;
            for(int i=0; i<length2; i++) {
              if(data2[0] == ' ' || data2[0] == '\n' || data2[0] == '\r') {
                wordLength = i;
                break;
              }
            }
            log.debug("analyzeLatin1Cutting: wordLength = %d", wordLength);
            if(length1 + wordLength < 159) {
              log.debug("analyzeLatin1Cutting: sms2 contains word that should be added to sms1");
              res = false;
            }
          }
        }

        if(log.isDebugEnabled()) {
          UDH udh(data1);
          length1 -= udh.getLength() + 1;
          data1 += udh.getLength() + 1;
          std::string s1(data1, length1);
          std::string s2(data2, length2);
          log.debug("analyzeLatin1Cutting: string1 = %s",  s1.c_str());
          log.debug("analyzeLatin1Cutting: string2 = %s",  s2.c_str());
          log.debug("analyzeLatin1Cutting: --- exit, res = %d",  res);
        }
        return res;
      }

      bool MapProxy::checkSmsc7bit(const char * const message, SmsArrayHandler sah) {
        log.debug("checkSmsc7bit: --- enter");
        Smsc7BitText smsc7bit(message);
        log.debug("checkSmsc7bit: smsc7bit length = %d", smsc7bit.getLength());
        bool res = true;
        uint32_t lengthSum = 0;
        char messageSum[smsc7bit.getLength() + 1];
        messageSum[smsc7bit.getLength()] = 0;
        for(int i=0; i<sah->getSize(); i++) {
          //extracting SMS message
          unsigned length = (*sah)[i]->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
          const char *data;
          if(length != 0) {//сообщение в body
            data = (*sah)[i]->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length);
          } else {//payload
            data = (*sah)[i]->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length);
          }
          UDH udh(data);
          length -= udh.getLength() + 1;
          data += udh.getLength() + 1;

          lengthSum += length;
          if(lengthSum < smsc7bit.getLength() + 1) {
            memcpy(messageSum + (lengthSum - length), data, length);
          } else {
            log.error("checkSmsc7bit: error, lengthSum >= smsc7bit.getLength() + 1, lengthSum = %d, size = %d", lengthSum, smsc7bit.getLength());
          }
        }
        log.debug("checkSmsc7bit: lengthSum = %d", lengthSum);
        res = compare(smsc7bit.getSmsc7Bit(), smsc7bit.getLength(), messageSum, lengthSum);
        if(res) {
          log.debug("checkSmsc7bit: checking message pairs, total messages=%d",sah->getSize());
          for(int i=0; i<sah->getSize() -1; i++) {
            log.debug("checkSmsc7bit: analyzing cutting for %d and %d messages", i, i+1);
            if(!analyzeSmsc7bitCutting((*sah)[i], (*sah)[i+1])) {
              log.debug("checkSmsc7bit: analyze smsc7bit cutting is FALSE, i=%d", i);
              res = false;
              break;
            }
          }
        }
        log.debug("checkSmsc7bit: --- exit, res = %d", res);
        return res;
      }
    
      bool MapProxy::analyzeSmsc7bitCutting(SmsHandler sms1, SmsHandler sms2) {
        log.debug("analyzeSmsc7bitCutting: --- enter");
        bool res = true;
        //extracting SMS message1
        unsigned length1 = sms1->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
        const char *data1;
        if(length1 != 0) {//сообщение в body
          data1 = sms1->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length1);
        } else {//payload
          data1 = sms1->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length1);
        }
        log.debug("analyzeSmsc7bitCutting: length1 = %d", length1);
        unsigned length2;
        const char *data2;
        //extracting SMS message2
        length2 = sms2->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
        if(length2 != 0) {//сообщение в body
          data2 = sms2->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length2);
        } else {//payload
          data2 = sms2->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length2);
        }
        UDH udh(data2);
        length2 -= udh.getLength() + 1;
        data2 += udh.getLength() + 1;
        log.debug("analyzeSmsc7bitCutting: length2 = %d", length2);
        if(length1 < 140) {
          if(data2[0] == ' ' || data2[0] == '\n' || data2[0] == '\r') {
            log.debug("analyzeSmsc7bitCutting: sms2 contains symbols (space) that should be added to sms1");
            res = false;
          } else if(data1[length1 -1] != ' ' && data1[length1 -1] != '\n' && data1[length1 -1] != '\r') {
            log.debug("analyzeSmsc7bitCutting: sms2 contains symbols (not space) that should be added to sms1");
            res = false;
          } else {
            int wordLength = length2;
            for(int i=0; i<length2; i++) {
              if(data2[0] == ' ' || data2[0] == '\n' || data2[0] == '\r') {
                wordLength = i;
                break;
              }
            }
            log.debug("analyzeSmsc7bitCutting: wordLength = %d", wordLength);
            if(length1 + wordLength < 140) {
              log.debug("analyzeSmsc7bitCutting: sms2 contains word that should be added to sms1");
              res = false;
            }
          }
        }

        if(log.isDebugEnabled()) {
          /*UDH udh(data1);
          length1 -= udh.getLength() + 1;
          data1 += udh.getLength() + 1;
          std::string s1(data1, length1);
          std::string s2(data2, length2);
          log.debug("analyzeLatin1Cutting: string1 = %s",  s1.c_str());
          log.debug("analyzeLatin1Cutting: string2 = %s",  s2.c_str());*/
          log.debug("analyzeLatin1Cutting: --- exit, res = %d",  res);
        }
        return res;
      }

      bool MapProxy::checkUcs2(const char * const message, SmsArrayHandler sah) {
        bool res = true;
        return res;
      }

      bool MapProxy::analyzeUcs2Cutting(SmsHandler sms1, SmsHandler sms2) {
        log.debug("analyzeBinaryCutting: --- enter");
        bool res = true;
        //extracting SMS message1
        unsigned length1 = sms1->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
        const char *data1;
        if(length1 != 0) {//сообщение в body
          data1 = sms1->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length1);
        } else {//payload
          data1 = sms1->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length1);
        }
        log.debug("analyzeBinaryCutting: length1 = %d", length1);
        if(length1 < 159) {
          //extracting SMS message2
          unsigned length2 = sms2->getIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH);
          const char *data2;
          if(length2 != 0) {//сообщение в body
            data2 = sms2->getBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, &length2);
          } else {//payload
            data2 = sms2->getBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, &length2);
          }
          UDH udh(data2);
          length2 -= udh.getLength() + 1;
          data2 += udh.getLength() + 1;
          log.debug("analyzeBinaryCutting: length2 = %d", length2);
          if(data2[0] == 32 || data2[0] == 10 || data2[0] == 13) {
            log.debug("analyzeBinaryCutting: sms2 contains symbols (space) that should be added to sms1");
            res = false;
          } else if(data1[length1 -1] != 32 && data1[length1 -1] != 10 && data1[length1 -1] != 13) {
            log.debug("analyzeBinaryCutting: sms2 contains symbols (not space) that should be added to sms1");
            res = false;
          } else {
            int wordLength = 0;
            for(int i=0; i<length2; i++) {
              if(data2[0] == 32 || data2[0] == 10 || data2[0] == 13) {
                wordLength = i;
              }
            }
            log.debug("analyzeBinaryCutting: wordLength = %d", wordLength);
            if(length1 + wordLength <= 140) {
              log.debug("analyzeBinaryCutting: sms2 contains word that should be added to sms1");
              res = false;
            }
          }
        }

        log.debug("analyzeBinaryCutting: --- exit, res = %d",  res);
        return res;
      }

      bool MapProxy::compare(const char * const buf1, int size1, const char * const buf2, int size2) {
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
