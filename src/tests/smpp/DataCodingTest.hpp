#ifndef ___DATA_CODING_TEST___
#define ___DATA_CODING_TEST___

#include "Sme.hpp"
#include "SmppProfileManager.hpp"
#include "TextCoding.hpp"

namespace smsc {
  namespace test {
    namespace smpp {


      class DataCodingTest {
        static const char *symbols;
        static uint8_t dataCodingConstants[4];
        static uint8_t profileDataCodingConstants[4];
        QueuedSmeHandler sender;
        QueuedSmeHandler receiver;
        SmppProfileManagerHandler profileManager;
        smsc::logger::Logger log;
        uint32_t timeout;
        Latin1Text *latin1Msg;
        Smsc7BitText *smsc7bitMsg;
        Ucs2Text *ucs2Msg;
      public:
        DataCodingTest(QueuedSmeHandler _sender, QueuedSmeHandler _receiver, SmppProfileManagerHandler _profileManager, uint32_t _timeout) 
        : sender(_sender)
        , receiver(_receiver)
        , profileManager(_profileManager)
        , log(smsc::test::util::logger.getLog("smsc.test.smpp.DataCodingTest")) 
        , timeout(_timeout) {
          latin1Msg = new Latin1Text(symbols);
          smsc7bitMsg = new Smsc7BitText(symbols);
          ucs2Msg = new Ucs2Text(symbols);
          //connecting SMEs
          sender->connect();
          receiver->connect();
        }

        void testDataCoding();

        ~DataCodingTest() {
          delete latin1Msg;
          delete smsc7bitMsg;
          delete ucs2Msg;
          sender->close();
          receiver->close();
        }
      private:
        void test() {
          for(uint8_t i=0; i<4; i++) {
            for(uint8_t j=0; j<4; j++) {
              test(dataCodingConstants[i], profileDataCodingConstants[j]);
            }
          }
        }

        void test(uint8_t dataCoding, uint8_t profileDataCoding);
        bool compare(uint8_t dataCoding, uint8_t profileDataCoding, const char * const buf, int size);
        bool compare(const char * const buf1, int size1, const char * const buf2, int size2);
      };

    }//namespace smpp
  }//namespace test
}//namespace smsc

#endif //___DATA_CODING_TEST___
