#include <cppunit/extensions/HelperMacros.h>
#include <util/smstext.h>
#include <util/recoder/recode_dll.h>

#include "SmppProfileManager.hpp"
#include "ResponseChecker.hpp"

namespace smsc {
  namespace test {
    namespace smpp {

      const char * SmppProfileManager::profileCommands[] = {
        "report none",
        "report full",
        "report final",
        "locale ru_ru",
        "locale en_en",
        "default",
        "latin1",
        "ucs2andlat",
        "ucs2",
        "hide",
        "unhide"
      };

      const char * SmppProfileManager::profileResponses[] = {
        "Now you will receive only requested delivery reports",
        "Now you will receive auxiliary delivery reports",
        "Now you will receive final delivery reports",
        "Your locale settings changed",
        "Your locale settings changed",
        "Now you are a default language user",
        "Now you will be able to receive latin1-encoded messages",
        "Now you will be able to receive latin1 and ucs2  encoded messages",
        "Now you will be able to receive ucs2-encoded messages",
        "Hidden",
        "Not Hidden"
      };

      SmppProfileManager::SmppProfileManager(QueuedSmeHandler esme) 
      : sme(esme), timeout(10000), profilerAddr(new smsc::sms::Address("0"))
      , log(smsc::test::util::logger.getLog("smsc.test.smpp.SmppProfileManager")) {
        
        origAddr = new smsc::sms::Address(sme->getConfig().origAddr.c_str());
        sme->connect();
        // binding as transceiver
        uint32_t sequence = sme->bind(smsc::sme::BindType::Transceiver);
        if(ResponseChecker::checkBind(sequence, smsc::sme::BindType::Transceiver, sme, timeout)== false) {
          CPPUNIT_FAIL("SmppProfileManager: error in response for transceiver bind pdu");
        }
      }

      SmppProfileManager::~SmppProfileManager() {
        // unbinding transceiver
        uint32_t sequence = sme->unbind();
        if(ResponseChecker::checkUnbind(sequence, sme, timeout)== false) {
          CPPUNIT_FAIL("SmppProfileManager: error when sme unbinding");
        }
      }

      bool SmppProfileManager::setProfile(ProfileCommand cmd, uint8_t dataCoding) {
        bool res = false;
        log.debug("setProfile(%s): --- enter", profileCommands[cmd]);
        smsc::smpp::PduSubmitSm submit;
        submit.get_header().set_commandId(smsc::smpp::SmppCommandSet::SUBMIT_SM);
        smsc::sms::SMS sms;
        sms.setOriginatingAddress(*origAddr);
        sms.setDestinationAddress(*profilerAddr);
        char msc[]="";
        char imsi[]="";
        sms.setOriginatingDescriptor(strlen(msc),msc,strlen(imsi),imsi,1);
        sms.setValidTime(0);
        sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS, 2);//transaction mode
        sms.setDeliveryReport(0);
        sms.setArchivationRequested(false);
        sms.setIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE, 0);
        sms.setEServiceType("XXX");
        // sending message 
        uint32_t length = strlen(profileCommands[cmd]);
        sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, dataCoding);
        switch(dataCoding) {
        case smsc::smpp::DataCoding::BINARY: {
          log.debug("setProfile: DataCoding::BINARY");
          sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, profileCommands[cmd], length);
        }
        break;
        case smsc::smpp::DataCoding::LATIN1: {
          log.debug("setProfile: DataCoding::LATIN1");
          sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, profileCommands[cmd], length);
        }
        break;
        case smsc::smpp::DataCoding::SMSC7BIT: {
          char buff[length + 1];
          uint32_t readLength = ConvertTextTo7Bit(profileCommands[cmd], length, buff, length, CONV_ENCODING_CP1251);
          log.debug("setProfile: DataCoding::SMSC7BIT, buff=%s", buff);
          sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, buff, readLength);
        }
        break;
        case smsc::smpp::DataCoding::UCS2: {
          short ucs2[length + 1];
          ConvertMultibyteToUCS2(profileCommands[cmd], length, ucs2, 2*length, CONV_ENCODING_CP1251);
          log.debug("setProfile: DataCoding::UCS2, ucs2=%s", (char*)ucs2);
          sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, (char*)ucs2, 2*length);
        }
        break;
        default: {
          log.error("setProfile: Unknown Data Coding");
          return false;
          }
        }
        smsc::smpp::fillSmppPduFromSms(&submit, &sms);
        uint32_t sequence = sme->sendPdu((smsc::smpp::SmppHeader*)&submit);
        // checking response from SMSC
        res = ResponseChecker::checkResponse(sequence, smsc::smpp::SmppCommandSet::SUBMIT_SM, sme, timeout);
        // receive SMS from profiler
        log.debug("setProfile: receiving SMS from profile");
        PduHandler pdu = sme->receive(timeout);
        if(pdu != 0) {
          if(pdu->get_commandId() == smsc::smpp::SmppCommandSet::DELIVERY_SM) {
            // send deliver_sm_resp
            log.debug("setProfile: sending DELIVER_SM_RESP");
            PduDeliverySmResp resp;
            resp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DELIVERY_SM_RESP);
            resp.set_messageId("");
            resp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
            resp.get_header().set_commandStatus(smsc::smpp::SmppStatusSet::ESME_ROK);
            sme->sendDeliverySmResp(resp);
            // check PDU received
            log.debug("setProfile: extracting SMS from PDU received");
            smsc::sms::SMS receivedSms;
            smsc::smpp::PduXSm *pduXSM = (smsc::smpp::PduXSm*) pdu.getObjectPtr();
            smsc::smpp::fetchSmsFromSmppPdu(pduXSM, &receivedSms);
            if(log.isDebugEnabled()) {
              log.debug("setProfile: SMS originating address = %s", receivedSms.getOriginatingAddress().toString().c_str());
              log.debug("setProfile: SMS destination address = %s", receivedSms.getDestinationAddress().toString().c_str());
              log.debug("setProfile: SMS data coding = %d", receivedSms.getIntProperty(smsc::sms::Tag::SMPP_DATA_CODING));
              log.debug("setProfile: SMS user message reference = %d", receivedSms.getIntProperty(smsc::sms::Tag::SMPP_USER_MESSAGE_REFERENCE));
            }
            char buff[256];
            if(smsc::util::getPduText(pduXSM, buff, sizeof(buff))==-1) {
              int sz=(pduXSM)->optional.size_messagePayload();
              char *data = new char[sz*2];
              if(smsc::util::getPduText(pduXSM, data, sz*2)!=-1) {
                log.debug("setProfile: SMS message(payload) = %s", data);
                res = (strcmp(data, profileResponses[cmd])==0);
              } else {
                log.debug("setProfile: faield to retrieve SMS message");
                res = false;
              }
              delete [] data;
            } else {
              log.debug("setProfile: SMS message = %s, res=%d", buff, strcmp(buff, profileResponses[cmd]));
              res = (strcmp(buff, profileResponses[cmd])==0);
            }
          } else {
            log.debug("setProfile: received unexpected PDU with commandId=%x", pdu->get_commandId());
            res = false;
          }
        } else {
          log.warn("setProfile: there is no response sms from profiler");
          res = false;
        }

        log.debug("setProfile(%s)=%d: --- exit", profileCommands[cmd], res);
        return res;
      }

      void SmppProfileManager::setOriginatingAddress(const char *addr) {
        try {
          origAddr = new smsc::sms::Address(addr);
        } catch(...) {
          CPPUNIT_FAIL("SmppProfileManager#setOriginatingAddress: Invalid orig address");
        }
      }

      void SmppProfileManager::setProfilerAddress(const char *addr) {
        try {
          profilerAddr = new smsc::sms::Address(addr);
        } catch(...) {
          CPPUNIT_FAIL("SmppProfileManager#setProfilerAddress: Invalid profiler address");
        }
      }

    }//namespace smpp
  }//namespace test
}//namespace smsc
