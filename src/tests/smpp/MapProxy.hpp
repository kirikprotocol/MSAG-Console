#ifndef ___MAP_PROXY_ESME___
#define ___MAP_PROXY_ESME___

#include <core/buffers/IntHash.hpp>
#include "Sme.hpp"

namespace smsc {
  namespace test {
    namespace smpp {
      
      using smsc::test::util::Handler;

      typedef Handler<smsc::sms::SMS> SmsHandler;
      
      class SmsArray : smsc::test::util::ProtectedCopy {
        SmsHandler *smsHandlers;
        int size;
      public:
        SmsArray(int size) {
          smsHandlers = new SmsHandler[size];
          this->size = size;
        }
        SmsHandler& operator [] (int index) {
          return smsHandlers[index];
        }
        const SmsHandler& operator [] (int index) const {
          return smsHandlers[index];
        }
        int getSize() {
          return size;
        }
        ~SmsArray() {
          delete[] smsHandlers;
        }
      };

      typedef Handler<SmsArray> SmsArrayHandler;

      class MapProxy : smsc::test::util::ProtectedCopy {
        QueuedSmeHandler mapProxy;
        IntHash<SmsArrayHandler> smsArrayMap;
        uint32_t timeout;
        log4cpp::Category& log;
      public:
        MapProxy(QueuedSmeHandler mapProxy, uint32_t timeout);
        bool receiveAndCheck(const char * const message);
        ~MapProxy();
      private:
        void receive();
        bool check(const char * const message);
        bool checkBinary(const char * const message, SmsArrayHandler sah);
        bool checkLatin1(const char * const message, SmsArrayHandler sah);
        bool analyzeLatin1Cutting(SmsHandler sms1, SmsHandler sms2);
        bool checkSmsc7bit(const char * const message, SmsArrayHandler sah);
        bool analyzeSmsc7bitCutting(SmsHandler sms1, SmsHandler sms2);
        bool checkUcs2(const char * const message, SmsArrayHandler sah);
        bool analyzeUcs2Cutting(SmsHandler sms1, SmsHandler sms2);
        bool compare(const char * const buf1, int size1, const char * const buf2, int size2);
      };

    }//namespace smpp
  }//namespace test
}//namespace smsc

#endif //___MAP_PROXY_ESME___
