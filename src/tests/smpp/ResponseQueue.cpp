#include <system/status.h>
#include "ResponseQueue.hpp"

namespace smsc {
  namespace test {
    namespace smpp {

      void ResponseQueue::clear() {
        responseMutex.Lock();
        responseMap.Empty();
        responseMutex.Unlock();
      }

      void ResponseQueue::registerPdu(uint32_t sequence, uint32_t cmdId) throw(ResponseQueueException) {
        log.debug("registerPdu(%d, %x): -- enter", sequence, cmdId);
        uint32_t expectedCmd;
        switch(cmdId) {
        case smsc::smpp::SmppCommandSet::SUBMIT_SM:
          expectedCmd = smsc::smpp::SmppCommandSet::SUBMIT_SM_RESP;
          break;
        case smsc::smpp::SmppCommandSet::SUBMIT_MULTI:
          expectedCmd = smsc::smpp::SmppCommandSet::SUBMIT_MULTI_RESP;
          break;
        case smsc::smpp::SmppCommandSet::DATA_SM:
          expectedCmd = smsc::smpp::SmppCommandSet::DATA_SM_RESP;
          break;
        case smsc::smpp::SmppCommandSet::QUERY_SM:
          expectedCmd = smsc::smpp::SmppCommandSet::QUERY_SM_RESP;
          break;
        case smsc::smpp::SmppCommandSet::CANCEL_SM:
          expectedCmd = smsc::smpp::SmppCommandSet::CANCEL_SM_RESP;
          break;
        case smsc::smpp::SmppCommandSet::REPLACE_SM:
          expectedCmd = smsc::smpp::SmppCommandSet::REPLACE_SM_RESP;
          break;
        case smsc::smpp::SmppCommandSet::BIND_TRANCIEVER:
          expectedCmd = smsc::smpp::SmppCommandSet::BIND_TRANCIEVER_RESP;
          break;
        case smsc::smpp::SmppCommandSet::BIND_TRANSMITTER:
          expectedCmd = smsc::smpp::SmppCommandSet::BIND_TRANSMITTER_RESP;
          break;
        case smsc::smpp::SmppCommandSet::BIND_RECIEVER:
          expectedCmd = smsc::smpp::SmppCommandSet::BIND_RECIEVER_RESP;
          break;
        case smsc::smpp::SmppCommandSet::UNBIND:
          expectedCmd = smsc::smpp::SmppCommandSet::UNBIND_RESP;
          break;
        case smsc::smpp::SmppCommandSet::ENQUIRE_LINK:
          expectedCmd = smsc::smpp::SmppCommandSet::ENQUIRE_LINK_RESP;
          break;
        default:
          std::ostringstream sout;
          sout << "ResponseQueueException: Illegal command id " << cmdId << " in registerPdu()";
          throw ResponseQueueException(sout.str().c_str());
        }
        log.debug("registerPdu: expectedCmd = %x", expectedCmd);
        responseMutex.Lock();
        if(responseMap.Exist(sequence)) {
          responseMutex.Unlock();
          std::ostringstream sout;
          sout << "ResponseQueueException: error in registerPdu, sequence " << sequence << " has been already registered";
          throw ResponseQueueException(sout.str().c_str());
        }
        ResponseHandler resp = new Response();
        resp->expectedCmd = expectedCmd;
        responseMap.Insert(sequence, resp);
        responseMutex.Unlock();
      }

      void ResponseQueue::processResponse(PduHandler pdu) {
        responseMutex.Lock();
        uint32_t sequence = pdu->get_sequenceNumber();
        if(responseMap.Exist(sequence)) {//зарегистрированный ответ
          ResponseHandler resp = responseMap.Get(sequence);
          resp->pdu = pdu;
          resp->event.Signal();
        } else {//левый ответ
        }
        responseMutex.Unlock();
      }

      PduHandler ResponseQueue::receiveResponse(uint32_t sequence) throw(ResponseQueueException) {
        responseMutex.Lock();
        if(responseMap.Exist(sequence)) {
            ResponseHandler resp = responseMap.Get(sequence);
            responseMutex.Unlock();
            int status = resp->event.Wait();
            responseMutex.Lock();
            responseMap.Delete(sequence);
            responseMutex.Unlock();
            return resp->pdu;

        } else {
          responseMutex.Unlock();
          std::ostringstream sout;
          sout << "ResponseQueueException: error in receiveResponse, sequence " << sequence << " has not been registered";
          throw ResponseQueueException(sout.str().c_str());
        }
      }

      PduHandler ResponseQueue::receiveResponse(uint32_t sequence, uint32_t timeout) throw(ResponseQueueException) {
        responseMutex.Lock();
        if(responseMap.Exist(sequence)) {
            ResponseHandler resp = responseMap.Get(sequence);
            responseMutex.Unlock();
            int status = resp->event.Wait(timeout);
            responseMutex.Lock();
            responseMap.Delete(sequence);
            responseMutex.Unlock();
            return resp->pdu;

        } else {
          responseMutex.Unlock();
          std::ostringstream sout;
          sout << "ResponseQueueException: error in receiveResponse, sequence " << sequence << " has not been registered";
          throw ResponseQueueException(sout.str().c_str());
        }
      }

      bool ResponseQueue::checkResponse(uint32_t sequence, uint32_t timeout) throw(ResponseQueueException) {
        bool res = false;

        ResponseHandler resp;
        if(responseMap.Exist(sequence)) {
          resp = responseMap.Get(sequence);
          PduHandler pdu = receiveResponse(sequence, timeout);
          if (pdu != 0) {
              if (log.isDebugEnabled()) {
                  log.debug("checkResponse: Received response");
                  log.debug("checkResponse: command_id=%x", pdu->get_commandId());
                  log.debug("checkResponse: command_status=%x", pdu->get_commandStatus());
                  log.debug("checkResponse: sequence_number=%u", pdu->get_sequenceNumber());
              }
              if (pdu->get_commandId() == resp->expectedCmd) {
                  if (pdu->get_commandStatus() == smsc::system::Status::OK) {
                      res = true;
                  } else {
                      log.error("checkResponse: The response with sequence=%u has wrong status=%u", sequence, pdu->get_commandStatus());
                  }
              } else {
                  log.error("checkResponse: The response with sequence=%u has wrong commandId=%x, expectedCmd=%x", sequence, pdu->get_commandId(), resp->expectedCmd);
              }
          } else {
              log.error("checkResponse: There is no response with sequence=%u", sequence);
          }
        }

        return res;
      }

      bool ResponseQueue::checkAllResponses() throw(ResponseQueueException) {
        bool res = true;
        responseMutex.Lock();
        smsc::core::buffers::IntHash<ResponseHandler>::Iterator itr = responseMap.First();
        int sequence;
        ResponseHandler resp;
        while(itr.Next(sequence, resp)) {
          if (resp->pdu != 0) {
              if (log.isDebugEnabled()) {
                  log.debug("checkResponse: Received response");
                  log.debug("checkResponse: command_id=%x", resp->pdu->get_commandId());
                  log.debug("checkResponse: command_status=%x", resp->pdu->get_commandStatus());
                  log.debug("checkResponse: sequence_number=%u", resp->pdu->get_sequenceNumber());
              }
              if (resp->pdu->get_commandId() == resp->expectedCmd) {
                  if (resp->pdu->get_commandStatus() != smsc::system::Status::OK) {
                      log.error("checkResponse: The response with sequence=%u has wrong status=%u", sequence, resp->pdu->get_commandStatus());
                      res = false;
                  }
              } else {
                  log.error("checkResponse: The response with sequence=%u has wrong commandId=%x", sequence, resp->pdu->get_commandId());
                  res = false;
              }
          } else {
              log.error("checkResponse: There is no response with sequence=%u", sequence);
              res = false;
          }
        }
        clear();
        responseMutex.Unlock();

        return res;
      }

    }//namespace test
  }//namespace test
}//namespace smsc

