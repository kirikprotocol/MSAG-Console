#include "ResponseQueue.hpp"

namespace smsc {
  namespace test {
    namespace smpp {

      void ResponseQueue::clear() {
        responseMutex.Lock();
        responseMap.Empty();
        responseMutex.Unlock();
      }

      void ResponseQueue::registerPdu(uint32_t sequence) throw(ResponseQueueException) {
        responseMutex.Lock();
        if(responseMap.Exist(sequence)) {
          responseMutex.Unlock();
          std::ostringstream sout;
          sout << "ResponseQueueException: error in registerPdu, sequence " << sequence << " has been already registered";
          throw ResponseQueueException(sout.str().c_str());
        }
        ResponseHandler resp = new Response();
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

    }//namespace test
  }//namespace test
}//namespace smsc

