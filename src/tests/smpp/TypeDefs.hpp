#ifndef ___SMPP_TYPE_DEFINITIONS_FOR_TESTS___
#define ___SMPP_TYPE_DEFINITIONS_FOR_TESTS___

#include <smpp/smpp_structures.h>
#include <tests/util/Handler.hpp>

namespace smsc {
  namespace test {
    namespace smpp {

      // handler for pdu, освобождает PDU вызывая метод disposePdu
      template <class T> class PduDisposePolicy {
      public:
          void dispose(T *pdu) {
              smsc::smpp::disposePdu(pdu);
          }
      };
      typedef smsc::test::util::Handler<smsc::smpp::SmppHeader, smsc::test::util::Exceptioner, PduDisposePolicy> PduHandler;

      class GenericSme;
      class SmppSession;
      class SmppListener;
      typedef smsc::test::util::Handler<GenericSme> GenericSmeHandler;
      typedef smsc::test::util::Handler<SmppSession> SmppSessionHandler;
      typedef smsc::test::util::Handler<SmppListener> PduListenerHandler;

      class QueuedSme;
      typedef smsc::test::util::Handler<QueuedSme> QueuedSmeHandler;

    }//namespace smpp
  }//namespace test
}//namespace smsc

#endif
