#ifndef ___QUEUE_FOR_PDU___
#define ___QUEUE_FOR_PDU___

#include <inttypes.h>
#include <list>
#include "tests/util/Mutex.hpp"
#include "tests/util/Event.hpp"
#include "TypeDefs.hpp"

namespace smsc {
  namespace test {
    namespace smpp {

      class PduQueue {
        typedef std::list<PduHandler> PduHandlerList;

        std::list<PduHandler> pduQueue;
        smsc::test::util::Mutex pduMutex;
        smsc::test::util::Event pduEvent;
      public:
        void processPdu(PduHandler pdu) throw() {
          pduMutex.Lock();
          pduQueue.push_back(pdu);
          pduEvent.Signal();
          pduMutex.Unlock();
        }
        PduHandler receive() throw() {
          int status = pduEvent.Wait();
          if (status) {	//какая-то ошибка
              return PduHandler();
          }
          pduMutex.Lock();
          PduHandler pdu = pduQueue.front();
          pduQueue.pop_front();
          pduMutex.Unlock();
          return pdu;
        }
        PduHandler receive(uint32_t timeout) throw() {
          int status = pduEvent.Wait(timeout);
          if (status) {	//вылет по таймауту
              return PduHandler();
          }
          pduMutex.Lock();
          PduHandler pdu = pduQueue.front();
          pduQueue.pop_front();
          pduMutex.Unlock();
          return pdu;
        }
        void clear() throw() {
          pduMutex.Lock();
          pduQueue.clear();
          pduMutex.Unlock();
        }
      };
    }//namespace test
  }//namespace test
}//namespace smsc

#endif //___QUEUE_FOR_PDU___
