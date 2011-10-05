#ifndef SMSC_UTIL_TEMPLATES_ENTITIES
#define SMSC_UTIL_TEMPLATES_ENTITIES

/**
 *
 * @author Victor V. Makarov
 * @version 1.0
 * @see
 */

#include <time.h>
#include <iostream>

#include <core/buffers/Hash.hpp>
#include <core/buffers/Array.hpp>
#include <util/Exception.hpp>

namespace smsc { namespace util { namespace templates
{
    using smsc::core::buffers::Hash;
    using smsc::core::buffers::Array;

    using smsc::util::Exception;

    class FormatRenderingException : public Exception
    {
    public:

        FormatRenderingException(const char* fmt,...)
            : Exception()
        {
            SMSC_UTIL_EX_FILL(fmt);
        };
        FormatRenderingException()
            : Exception("Exception occurred during format rendering !") {};

        virtual ~FormatRenderingException() throw() {};
    };

    extern const char* SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION;
    extern const char* SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION;
    extern const char* SMSC_DBSME_IO_FORMAT_PATTERN_OPTION;
    extern const char* SMSC_DBSME_IO_FORMAT_PRECISION_OPTION;
    extern const char* SMSC_DBSME_IO_FORMAT_EXPONENT_OPTION;
    extern const char* SMSC_DBSME_IO_FORMAT_DIGITS_OPTION;
    extern const char* SMSC_DBSME_IO_FORMAT_EXPORT_OPTION;
    extern const char* SMSC_DBSME_IO_FORMAT_IMPORT_OPTION;

    extern const char* SMSC_DBSME_MSG_TEXT;

    const char  SMSC_DBSME_IO_FORMAT_ENTITY_DELIMETER = '$';
    const char  SMSC_DBSME_IO_FORMAT_STRING_DELIMETER = '\"';
    const char  SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER   = '\\';
    const char  SMSC_DBSME_IO_FORMAT_OPTION_ASSIGN    = '=';

    extern const char* ioNowString;
    extern const char* ioTodayString;
    extern const char* ioTomorrowString;
    extern const char* ioYesterdayString;
    extern const char* ioInternalPatternString;

    extern const char* SMSC_DBSME_IO_DEFAULT_FORMAT_PATTERN;
    extern const char* SMSC_DBSME_IO_DEFAULT_PARSE_PATTERN;

    extern const char*  ioFullMonthesNames[];
    extern const char*  ioShortMonthesNames[];
    extern const char*  ioFullWeekDays[];
    extern const char*  ioShortWeekDays[];
/*    static const char*  ioFullMonthesNames[12] = {
        "January", "February", "March", "April",
        "May", "June", "July", "August", "September",
        "October", "November", "December"
    };
    static const char*  ioShortMonthesNames[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    static const char*  ioFullWeekDays[7] = {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    };
    static const char*  ioShortWeekDays[7] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };*/

    extern const char*  ioDayTimeParts[2];

    class ContextEnvironment
    {
    protected:

        Hash<std::string>   strs;
        Hash<int64_t>       ints;
        Hash<time_t>        dats;
        Hash<float>         flts;
        Hash<double>        dbls;
        Hash<long double>   ldls;

        static void toUpperCase(const char* str, char* low);

	bool needMsgText;

    public:

        ContextEnvironment() { needMsgText = false; };
        virtual ~ContextEnvironment() {};

        bool exportStr(const char* key, const char* val);
        bool importStr(const char* key, char* &val);
        bool exportInt(const char* key, int64_t val);
        bool importInt(const char* key, int64_t &val);
        bool exportDat(const char* key, time_t val);
        bool importDat(const char* key, time_t &val);
        bool exportFlt(const char* key, float val);
        bool importFlt(const char* key, float &val);
        bool exportDbl(const char* key, double val);
        bool importDbl(const char* key, double &val);
        bool exportLdl(const char* key, long double val);
        bool importLdl(const char* key, long double &val);

        void reset();
	void setNeedMsgText(bool need) { needMsgText = need; };
	bool isNeedMsgText() { return needMsgText; };
    };

    const int ioEntityTypesNumber = 14;
    static const char* ioEntityTypeStrings[ioEntityTypesNumber]  =
    {
        "int8", "int16", "int32", "int64",
        "uint8", "uint16", "uint32", "uint64",
        "float", "double", "long-double", "date",
        "string", "text"
    };

    typedef enum {
        ET_INT8 = 0, ET_INT16 = 1, ET_INT32 = 2, ET_INT64 = 3,
        ET_UINT8 = 4, ET_UINT16 = 5, ET_UINT32 = 6, ET_UINT64 = 7,
        ET_FLOAT = 8, ET_DOUBLE = 9, ET_LDOUBLE = 10, ET_DATE = 11,
        ET_STRING = 12, ET_TEXT = 13
    } EntityType;

    struct FormatEntity
    {

        EntityType          type;
        Hash<std::string>   options;
        std::string         str;

        FormatEntity(std::string line, bool io, bool type=true)
            throw(FormatRenderingException);
        virtual ~FormatEntity();

        const char* getOption(const char* name);

    private:

        void renderOptions(const char* line)
            throw(FormatRenderingException);
    };

    class FormatEntityRenderer
    {
    protected:

        Array<FormatEntity *>   entities;

        void clearEntities();

    public:

        FormatEntityRenderer(const char* format, bool io)
            throw(FormatRenderingException);
        virtual ~FormatEntityRenderer();

        /**
         * ���� �������� �������������� � ���������� � ��� �� �������� ��������� �����.
         * ���� ��������, ����� ��� ���� �� �������, ���������� -1.
         *
         * @param key       �������� ��������� �����
         * @param option    �����, �� ��������� SMSC_DBSME_IO_FORMAT_IMPORT_OPTION
         * @return          EntityType ��� ��������� �����.
         *                  ���� ��������, ����� ��� ���� �� �������, ���������� -1.
         */
        int getEntityType(const char* key,
                          const char* option = SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    };

}}}

#endif
