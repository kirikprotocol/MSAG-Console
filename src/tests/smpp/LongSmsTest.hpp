#ifndef ___LONG_SMS_TEST___
#define ___LONG_SMS_TEST___

#include "Sme.hpp"

namespace smsc {
  namespace test {
    namespace smpp {


      class LongSmsTest {
        QueuedSmeHandler sender;
        QueuedSmeHandler mapProxy;
        log4cpp::Category& log;
        uint32_t timeout;
      public:
        LongSmsTest(QueuedSmeHandler _sender, QueuedSmeHandler _mapProxy, uint32_t _timeout) 
        : sender(_sender)
        , mapProxy(_mapProxy)
        , log(smsc::test::util::logger.getLog("smsc.test.smpp.LongSmsTest")) 
        , timeout(_timeout) {
          //connecting SMEs
          sender->connect();
          mapProxy->connect();
        }

        void testLongSms();

        ~LongSmsTest() {
          sender->close();
          mapProxy->close();
        }
      private:
        void test();
      };

    }//namespace smpp
  }//namespace test
}//namespace smsc

#endif //___LONG_SMS_TEST___

