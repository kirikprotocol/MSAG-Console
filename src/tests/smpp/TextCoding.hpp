#ifndef ___SMSC_TEXT_CODING___
#define ___SMSC_TEXT_CODING___

#include <util/smstext.h>
#include <tests/util/ProtectedCopy.hpp>

namespace smsc {
  namespace test {
    namespace smpp {

      class Ucs2Text;
      class Smsc7BitText;

      class Latin1Text : smsc::test::util::ProtectedCopy {
        char *latin1;
      public:
        Latin1Text(const char *text) {
          setLatin1(text);
        }

        inline Latin1Text(const Ucs2Text &ucs2);

        inline Latin1Text(const Smsc7BitText &smsc4bit);

        int getLength() const {
          return strlen(latin1);
        }

        const char * const getLatin1() const {
          return latin1;
        }

        ~Latin1Text() {
          delete[] latin1;
        }

      private:
        void setLatin1(const char * const text) {
          uint32_t length = strlen(text);
          char buf[length*3];
          uint32_t readLength = Transliterate(text, length, CONV_ENCODING_CP1251, buf, length*3);
          latin1 = new char[readLength +1];
          memcpy(latin1, buf, readLength);
          latin1[readLength] = 0;
        }
      };

      class Ucs2Text : smsc::test::util::ProtectedCopy {
        short *ucs2;
        int length;
      public:

        Ucs2Text(const char *text) {
          setUcs2(text);
        }

        Ucs2Text(const Latin1Text &latin1) {
          setUcs2(latin1.getLatin1());
        }

        int getLength() const {
          return length;
        }

        const short * const getUcs2() const {
          return ucs2;
        }

        ~Ucs2Text() {
          delete[] ucs2;
        }

      private:
        void setUcs2(const char * const text) {
          short buf[strlen(text)];
          uint32_t readLength = ConvertMultibyteToUCS2(text, strlen(text), buf, 2*strlen(text), CONV_ENCODING_CP1251);
          this->ucs2 = new short[readLength];
          memcpy(this->ucs2, buf, 2*readLength);
          this->length = readLength/2;
        }
      };

      class Smsc7BitText : smsc::test::util::ProtectedCopy {
        char *smsc7bit;
        int length;
      public:

        Smsc7BitText(const char *text) {
          Latin1Text latin1(text);
          setSmsc7Bit(latin1);
        }

        Smsc7BitText(const Latin1Text &latin1) {
          setSmsc7Bit(latin1);
        }

        Smsc7BitText(const Ucs2Text &ucs2) {
          Latin1Text latin1(ucs2);
          setSmsc7Bit(latin1);
        }


        int getLength() const {
          return length;
        }

        const char * const getSmsc7Bit() const {
          return smsc7bit;
        }

        ~Smsc7BitText() {
          delete[] smsc7bit;
        }

      private:
        void setSmsc7Bit(const Latin1Text &latin1) {
          char buf[3*latin1.getLength()];
          uint32_t readLength = ConvertLatin1ToSMSC7Bit(latin1.getLatin1(), latin1.getLength(), buf);
          smsc7bit = new char[readLength];
          memcpy(smsc7bit, buf, readLength);
          length = readLength;
        }
      };

      inline Latin1Text::Latin1Text(const Ucs2Text &ucs2) {
        char buf[3*2*ucs2.getLength()];
        uint32_t readLength = ConvertUCS2ToMultibyte(ucs2.getUcs2(), 2*ucs2.getLength(), buf, 3*2*ucs2.getLength(), CONV_ENCODING_CP1251);
        buf[readLength] = 0;
        setLatin1(buf);
      }

      inline Latin1Text::Latin1Text(const Smsc7BitText &smsc7bit) {
        char buf[smsc7bit.getLength()];
        uint32_t readLength = ConvertSMSC7BitToLatin1(smsc7bit.getSmsc7Bit(), smsc7bit.getLength(), buf);
        latin1 = new char[readLength +1];
        memcpy(latin1, buf, readLength);
        latin1[readLength] = 0;
      }

    }//namespace smpp
  }//namespace test
}//namespace smsc

#endif //___SMSC_TEXT_CODING___
