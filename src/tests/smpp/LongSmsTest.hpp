#ifndef ___LONG_SMS_TEST___
#define ___LONG_SMS_TEST___

#include "Sme.hpp"
#include "MapProxy.hpp"

namespace smsc {
  namespace test {
    namespace smpp {


      class LongSmsTest {
        QueuedSmeHandler sender;
        MapProxy mapProxy;
        log4cpp::Category& log;
        uint32_t timeout;
        smsc::sms::Address mapProxyAddr;
        std::string message;
      public:
        LongSmsTest(QueuedSmeHandler _sender, QueuedSmeHandler _mapProxy, std::string msg, uint32_t _timeout) 
        : sender(_sender)
        , mapProxy(_mapProxy, _timeout)
        , message(msg)
        , log(smsc::test::util::logger.getLog("smsc.test.smpp.LongSmsTest")) 
        , timeout(_timeout) 
        , mapProxyAddr(_mapProxy->getConfig().origAddr.c_str()) {
          //connecting SMEs
          sender->connect();
        }

        void testLongSms();

        ~LongSmsTest() {
          sender->close();
        }
      private:
        void test();
      };

    }//namespace smpp
  }//namespace test
}//namespace smsc

#endif //___LONG_SMS_TEST___

