#ifndef ___QUEUE_FOR_RESPONSES___
#define ___QUEUE_FOR_RESPONSES___

#include <inttypes.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <tests/util/Configurator.hpp>
#include "TypeDefs.hpp"

namespace smsc {
  namespace test {
    namespace smpp {

      class ResponseQueueException : public std::runtime_error {
      public:
          ResponseQueueException(const char *const message) : std::runtime_error(message) {}
      };

      struct Response {
        smsc::core::synchronization::Event event;
        PduHandler pdu;
        uint32_t expectedCmd;
      };

      using smsc::core::buffers::IntHash;

      class ResponseQueue : smsc::test::util::ProtectedCopy {
        typedef smsc::test::util::Handler<Response> ResponseHandler;

        smsc::logger::Logger log;
        smsc::core::synchronization::Mutex responseMutex;
        IntHash<ResponseHandler> responseMap;
      public:
        ResponseQueue() : log(smsc::test::util::logger.getLog("smsc.test.smpp.ResponseQueue")) {}
        void registerPdu(uint32_t sequence, uint32_t cmdId) throw(ResponseQueueException);
        void processResponse(PduHandler pdu);
        PduHandler receiveResponse(uint32_t sequence) throw(ResponseQueueException);
        PduHandler receiveResponse(uint32_t sequence, uint32_t timeout) throw(ResponseQueueException);
        bool checkResponse(uint32_t sequence, uint32_t timeout) throw(ResponseQueueException);
        bool checkAllResponses() throw(ResponseQueueException);
        void clear();
      };
    }//namespace test
  }//namespace test
}//namespace smsc

#endif
