#ifndef CSV_FILE_ENCODER_DECLARATIONS
#define CSV_FILE_ENCODER_DECLARATIONS

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#include <time.h>
#include <stdio.h>
#include <string.h>

#include <string>

namespace smsc { namespace util { namespace csv 
{
    static const char CSV_STRING_SEPARATOR  = '"';
    static const char CSV_VALUES_SEPARATOR  = ',';
    static const char CSV_VALUES_TERMINATOR = '\n';

    struct CSVFileEncoder
    {
        inline static void addSeparator(std::string& out) {
            out += CSV_VALUES_SEPARATOR;
        }
        inline static void addTerminator(std::string& out) {
            out += CSV_VALUES_TERMINATOR;
        }
        
        inline static void addString(std::string& out, const char* str, bool last=false)
        {
            if (str)
            {
                out += CSV_STRING_SEPARATOR;
                while (*str) {
                    if (*str == CSV_STRING_SEPARATOR) out += CSV_STRING_SEPARATOR;
                    out += *str++;
                }
                out += CSV_STRING_SEPARATOR;
            }
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addChar(std::string& out, const char chr, bool last=false)
        {
            bool spaceOrSep = (isspace(chr) || chr == CSV_VALUES_SEPARATOR);
            if (spaceOrSep || chr == CSV_STRING_SEPARATOR) out += CSV_STRING_SEPARATOR;
            if (chr == CSV_STRING_SEPARATOR) out += CSV_STRING_SEPARATOR;
            out += chr;
            if (spaceOrSep) out += CSV_STRING_SEPARATOR;
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addInt8(std::string& out, int8_t val, bool last=false)
        {
            char buf[32]; sprintf(buf, "%d", val); out += buf;
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addInt16(std::string& out, int16_t val, bool last=false)
        {
            char buf[32]; sprintf(buf, "%d", val); out += buf;
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addInt32(std::string& out, int32_t val, bool last=false)
        {
            char buf[32]; sprintf(buf, "%ld", val); out += buf;
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addInt64(std::string& out, int64_t val, bool last=false)
        {
            char buf[32]; sprintf(buf, "%lld", val); out += buf;
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addUint8(std::string& out, uint8_t val, bool last=false)
        {
            char buf[32]; sprintf(buf, "%u", val); out += buf;
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addUint16(std::string& out, uint16_t val, bool last=false)
        {
            char buf[32]; sprintf(buf, "%u", val); out += buf;
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addUint32(std::string& out, uint32_t val, bool last=false)
        {
            char buf[32]; sprintf(buf, "%lu", val); out += buf;
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addUint64(std::string& out, uint64_t val, bool last=false)
        {
            char buf[32]; sprintf(buf, "%llu", val); out += buf;
            if (last) addTerminator(out);
            else addSeparator(out);
        }
        inline static void addDateTime(std::string& out, time_t date, bool last=false)
        {
            if (date > 0)
            {
                char buf[64];
                tm dt; gmtime_r(&date, &dt);
                sprintf(buf, "%02d.%02d.%04d %02d:%02d:%02d", 
                        dt.tm_mday, dt.tm_mon+1, dt.tm_year+1900,
                        dt.tm_hour, dt.tm_min, dt.tm_sec);
                out += buf;
            } 
            if (last) addTerminator(out);
            else addSeparator(out);
        }
    };

}}}

#endif // #ifndef CSV_FILE_ENCODER_DECLARATIONS

