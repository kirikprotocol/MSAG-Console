#include <stdio.h>
#include "debug.h"
#include "smstext.h"
#include "sms/sms.h"
#include "smpp/smpp.h"
#include "util/templates/Formatters.h"
#include "util/templates/Adapters.h"
#include <string>

using namespace smsc::sms;
using namespace smsc::util;
using namespace smsc::smpp;
using namespace smsc::util::templates;

class MyGetAdapter:public GetAdapter{
public:
        virtual bool isNull(const char* key)
            throw(AdapterException)
        {
          return false;
        }

        virtual const char* getString(const char* key)
            throw(AdapterException)
        {
          if(!strcmp(key,"hello"))
          {
            return "hello";
          }
          return "";
        }

        virtual int8_t getInt8(const char* key)
            throw(AdapterException)
        {
          return 0;
        }

        virtual int16_t getInt16(const char* key)
            throw(AdapterException)
        {
          return 0;
        }

        virtual int32_t getInt32(const char* key)
            throw(AdapterException)
        {
          return 0;
        }

        virtual int64_t getInt64(const char* key)
            throw(AdapterException)
        {
          return 0;
        }


        virtual uint8_t getUint8(const char* key)
            throw(AdapterException)
        {
          return 0;
        }

        virtual uint16_t getUint16(const char* key)
            throw(AdapterException)
        {
          return 0;
        }

        virtual uint32_t getUint32(const char* key)
            throw(AdapterException)
        {
          return 0;
        }

        virtual uint64_t getUint64(const char* key)
            throw(AdapterException)
        {
          return 0;
        }


        virtual float getFloat(const char* key)
            throw(AdapterException)
        {
          return 0;
        }

        virtual double getDouble(const char* key)
            throw(AdapterException)
        {
          return 0;
        }

        virtual long double getLongDouble(const char* key)
            throw(AdapterException)
        {
          return 0;
        }


        virtual time_t getDateTime(const char* key)
            throw(AdapterException)
        {
          return 0;
        }

};

int main(int argc, char** argv)
{
  OutputFormatter of("this is test:$$string arg=hello$$");
  MyGetAdapter ga;
  string out;
  ContextEnvironment ce;
  of.format(out,ga,ce);
  printf("<<%s>>\n",out.c_str());
}
