#ifndef ___QUEUE_FOR_RESPONSES___
#define ___QUEUE_FOR_RESPONSES___

#include <inttypes.h>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
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
      };

      using smsc::core::buffers::IntHash;

      class ResponseQueue {
        typedef smsc::test::util::Handler<Response> ResponseHandler;

        smsc::core::synchronization::Mutex responseMutex;
        IntHash<ResponseHandler> responseMap;
      public:
        void registerPdu(uint32_t sequence) throw(ResponseQueueException);
        void processResponse(PduHandler pdu);
        PduHandler receiveResponse(uint32_t sequence) throw(ResponseQueueException);
        PduHandler receiveResponse(uint32_t sequence, uint32_t timeout) throw(ResponseQueueException);
        void clear();
      };
    }//namespace test
  }//namespace test
}//namespace smsc

#endif
