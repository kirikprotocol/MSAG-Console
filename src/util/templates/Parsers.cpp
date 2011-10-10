
#include <stdlib.h>
#include <util/debug.h>

#include "Parsers.h"

namespace smsc { namespace util { namespace templates
{

ParserRegistry::FakeHash ParserRegistry::parsers;

InputParser::InputParser(const char* format)
    throw(FormatRenderingException)
        : FormatEntityRenderer(format, false)
{
    static Int8Parser       _Int8Parser;
    static Int16Parser      _Int16Parser;
    static Int32Parser      _Int32Parser;
    static Int64Parser      _Int64Parser;
    static Uint8Parser      _Uint8Parser;
    static Uint16Parser     _Uint16Parser;
    static Uint32Parser     _Uint32Parser;
    static Uint64Parser     _Uint64Parser;
    static StringParser     _StringParser;
    static FloatParser      _FloatParser;
    static DoubleParser     _DoubleParser;
    static LongDoubleParser _LongDoubleParser;
    static DateTimeParser   _DateTimeParser;
}
void InputParser::parse(std::string& input,
    SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    int bytes = 0;
    const char* str = input.c_str();
    while (str && isspace(str[bytes])) bytes++;
    if (bytes) input.erase(0, bytes);

    for (int i=0; i<entities.Count(); i++)
    {
        FormatEntity* entity = entities[i];
        if (entity && entity->type < ET_TEXT)
        {
            __trace2__("Parsing arg of type: %s", ioEntityTypeStrings[entity->type]);
            Parser* parser =
                ParserRegistry::getParser(ioEntityTypeStrings[entity->type]);
            if (parser)
            {
                parser->parse(input, *entity, adapter, ctx);
                bytes = 0; str = input.c_str();
                while (str && isspace(str[bytes])) bytes++;
                if (bytes) input.erase(0, bytes);
            }
            else throw ParsingException("Parser for type <%s> not defined !",
                                         ioEntityTypeStrings[entity->type]);
        }
        else throw ParsingException("Type <%s> is invalid !",
                                    ioEntityTypeStrings[entity->type]);
    }

    if (!ctx.isNeedMsgText() && (input.length() > 0))
        throw ParsingWarning("Unexpected paramenters left in input : '%s'", input.c_str());
}

void Int8Parser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    int32_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (int32_t)impVal;
    }
    else
    {
        int bytes = 0; int result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if ((result = sscanf(str, "%d%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0 || value != (int8_t)value)
            throw ParsingException("Error scanning int8 type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, (int64_t)value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setInt8(arg, (int8_t)value);

    __trace2__("Arg-Pos: %s, Value: %d, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void Int16Parser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    int32_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (int32_t)impVal;
    }
    else
    {
        int bytes = 0; int result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if ((result = sscanf(str, "%d%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0 || value != (int16_t)value)
            throw ParsingException("Error scanning int16 type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, (int64_t)value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setInt16(arg, (int16_t)value);

    __trace2__("Arg-Pos: %s, Value: %d, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void Int32Parser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    int32_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (int32_t)impVal;
    }
    else
    {
        int bytes = 0; int result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if ((result = sscanf(str, "%d%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning int32 type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, (int64_t)value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setInt32(arg, (int32_t)value);

    __trace2__("Arg-Pos: %s, Value: %d, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void Int64Parser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    int64_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = impVal;
    }
    else
    {
        int bytes = 0; int result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if ((result = sscanf(str, "%lld%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning int64 type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, (int64_t)value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setInt64(arg, (int64_t)value);

    __trace2__("Arg-Pos: %s, Value: %lld, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void Uint8Parser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    uint32_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (uint32_t)impVal;
    }
    else
    {
        int bytes = 0; int result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if (*str == '-')
            throw ParsingException("Error scanning uint8 type. Unexpected '-' "
                                   "Processing string: '%s'", str);
        if ((result = sscanf(str, "%u%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0 || value != (uint8_t)value)
            throw ParsingException("Error scanning uint8 type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, (int64_t)value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setUint8(arg, (uint8_t)value);

    __trace2__("Arg-Pos: %s, Value: %u, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void Uint16Parser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    uint32_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (uint32_t)impVal;
    }
    else
    {
        int bytes = 0; int result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if (*str == '-')
            throw ParsingException("Error scanning uint16 type. Unexpected '-' "
                                   "Processing string: '%s'", str);
        if ((result = sscanf(str, "%u%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0 || value != (uint16_t)value)
            throw ParsingException("Error scanning uint16 type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, (int64_t)value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setUint16(arg, (uint16_t)value);

    __trace2__("Arg-Pos: %s, Value: %u, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void Uint32Parser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    uint32_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (uint32_t)impVal;
    }
    else
    {
        int bytes = 0; int result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if (*str == '-')
            throw ParsingException("Error scanning uint32 type. Unexpected '-' "
                                   "Processing string: '%s'", str);
        if ((result = sscanf(str, "%lu%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning uint32 type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, (int64_t)value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setUint32(arg, (uint32_t)value);

    __trace2__("Arg-Pos: %s, Value: %lu, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void Uint64Parser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    uint64_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (uint32_t)impVal;
    }
    else
    {
        int bytes = 0; int result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if (*str == '-')
            throw ParsingException("Error scanning uint64 type. Unexpected '-' "
                                   "Processing string: '%s'", str);
        if ((result = sscanf(str, "%llu%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning uint64 type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, (int64_t)value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setUint64(arg, (uint64_t)value);

    __trace2__("Arg-Pos: %s, Value: %llu, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void StringParser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    std::string line = "";

    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);


    if (imp && (*SMSC_DBSME_MSG_TEXT == *imp)) ctx.setNeedMsgText(true);

    char* impVal = 0;
    if (imp && ctx.importStr(imp, impVal) && impVal && impVal[0])
    {
        line += impVal;
    }
    else
    {
        const char* str = input.c_str();

        if (str && *str)
        {
            int curPos = 0;
            if (str[curPos] == '\"')
            {
                curPos++;
                while (str[curPos] && !(str[curPos] == '\"'))
                    line += str[curPos++];
                if (str[curPos]) curPos++;
            }
            else
            {
                while (str[curPos] && !isspace(str[curPos]))
                    line += str[curPos++];
            }

             input.erase(0, curPos);
        }

        if (!line.length())
        {
            const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
            if (def) line += def;
        }
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportStr(exp, line.c_str());

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setString(arg, line.c_str());

    __trace2__("Arg-Pos: %s, Value: '%s', Less: <%s>",
               (arg) ? arg:"-", line.c_str(), input.c_str());
}
void FloatParser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    double value = 0.0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    double impVal = 0.0;
    if (imp && ctx.importDbl(imp, impVal))
    {
        value = impVal;
    }
    else
    {
        int bytes, result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        //double dblval = 0.0;
        if ((result = sscanf(str, "%le%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning float type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportDbl(exp, value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setDouble(arg, value);

    __trace2__("Arg-Pos: %s, Value: %lf, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void DoubleParser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    double value = 0.0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    double impVal = 0.0;
    if (imp && ctx.importDbl(imp, impVal))
    {
        value = impVal;
    }
    else
    {
        int bytes, result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if ((result = sscanf(str, "%le%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning double type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportDbl(exp, value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setDouble(arg, value);

    __trace2__("Arg-Pos: %s, Value: %le, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}
void LongDoubleParser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    long double value = 0.0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    long double impVal = 0.0;
    if (imp && ctx.importLdl(imp, impVal))
    {
        value = impVal;
    }
    else
    {
        int bytes, result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        if ((result = sscanf(str, "%Le%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning long-double type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportLdl(exp, value);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setLongDouble(arg, value);

    __trace2__("Arg-Pos: %s, Value: %Le, Less: <%s>",
               (arg) ? arg:"-", value, input.c_str());
}

char* stringToUpperCase(const char* str)
{
    if (!str) return 0;
    char* up = new char[strlen(str)+1];
    int cur = 0;
    while (*str) up[cur++] = (char)toupper(*str++);
    up[cur] = '\0';
    return up;
}
bool compareStringsIgnoreCase(const char* str1, const char* str2)
{
    char* up1 = stringToUpperCase(str1);
    char* up2 = stringToUpperCase(str2);
    bool result = ((!up1 && !up2) || strcmp(up1, up2) == 0);
    //__trace2__("compare '%s' with '%s' result = %d", up1, up2, result);
    if (up1) delete up1; if (up2) delete up2;
    return result;
}
void DateTimeParser::parse(std::string& input,
    FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
        throw(ParsingException, AdapterException)
{
    const char* error = "Error scanning Date/Time type: %s. "
                        "Processing string: '%s' "
                        "Pattern is: '%s'";
    tm      tmdt;
    time_t  date;

    memset(&tmdt, 0, sizeof(tmdt));

    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    if (!imp || !ctx.importDat(imp, date))
    {
        int     strPos = 0;
        int     bytes, result = 0;

        const char* pattern = entity.getOption(SMSC_DBSME_IO_FORMAT_PATTERN_OPTION);

        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;

        if (&str[strPos] == strstr(&str[strPos], ioNowString))
        {
            date = time(NULL);
        }
        else if (&str[strPos] == strstr(&str[strPos], ioTodayString))
        {
            date = time(NULL);
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            tmdt.tm_isdst = 0;
            date = mktime(&tmdt);
            date -= (tmdt.tm_isdst==1) ? 3600:0;
        }
        else if (&str[strPos] == strstr(&str[strPos], ioTomorrowString))
        {
            date = time(NULL) + 3600*24;
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            tmdt.tm_isdst = 0;
            date = mktime(&tmdt);
            date -= (tmdt.tm_isdst==1) ? 3600:0;
        }
        else if (&str[strPos] == strstr(&str[strPos], ioYesterdayString))
        {
            date = time(NULL) - 3600*24;
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            tmdt.tm_isdst = 0;
            date = mktime(&tmdt);
            date -= (tmdt.tm_isdst==1) ? 3600:0;
        }
        else if (pattern) // scan date by user-desired pattern
        {
            int     curPos = 0;

            bool needAMPM = false;  bool isAMPM  = false;   bool AMPM   = false;
            bool isDay    = false;  bool isMonth = false;   bool isYear = false;
            bool isHour   = false;  bool isMins  = false;   bool isSecs = false;

            while (pattern[curPos] && str[strPos])
            {
                while (isspace(str[strPos])) strPos++;
                while (isspace(pattern[curPos])) curPos++;

                switch(pattern[curPos])
                {
                case 'd':
                {
                    if (isDay) throw ParsingException(
                        "Invalid date/time pattern '%s'. "
                        "Day number already scanned !", pattern);

                    if ((result = sscanf(&str[strPos],
                        (pattern[++curPos] == 'd') ? "%2d%n":"%d%n",
                        &tmdt.tm_mday, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 ||
                        tmdt.tm_mday<=0 || tmdt.tm_mday>31)
                        throw ParsingException(error, "Month day expected",
                                               str, pattern);

                    __trace2__("Day %d <%s>",
                               tmdt.tm_mday, &str[strPos]);
                    if (pattern[curPos] == 'd') curPos++;
                    strPos += bytes; isDay = true;
                    continue;
                }
                case 'M':
                {
                    if (isMonth) throw ParsingException(
                        "Invalid date/time pattern '%s'. "
                        "Month already scanned !", pattern);

                    if (pattern[++curPos] == 'M')
                    {
                        if (pattern[++curPos] == 'M')
                        {
                            std::string buff = ""; bytes = 0;
                            while (isalpha(str[strPos])) buff += str[strPos++];

                            curPos++; int i;
                            for (i=0; i<12; i++)
                                if (compareStringsIgnoreCase(
                                    buff.c_str(), (pattern[curPos] == 'M') ?
                                    ioFullMonthesNames[i] :
                                    ioShortMonthesNames[i]))
                                {
                                    tmdt.tm_mon = i+1;
                                    break;
                                };

                            if (i >= 12)
                                throw ParsingException(error, "Month name expected",
                                                       str, pattern);

                            if (pattern[curPos] == 'M') curPos++;
                        }
                        else if ((result = sscanf(&str[strPos], "%2d%n",
                                &tmdt.tm_mon, &bytes)) == EOF ||
                                !result || !bytes || bytes<0 ||
                                 tmdt.tm_mon<=0 || tmdt.tm_mon>12)
                                throw ParsingException(error, "Month number expected",
                                                       str, pattern);
                    }
                    else if ((result = sscanf(&str[strPos], "%d%n",
                            &tmdt.tm_mon, &bytes)) == EOF ||
                            !result || !bytes || bytes<0 ||
                             tmdt.tm_mon<=0 || tmdt.tm_mon>12)
                            throw ParsingException(error, "Month number expected",
                                                   str, pattern);

                    __trace2__("Month %d <%s>",
                               tmdt.tm_mon, &str[strPos]);
                    tmdt.tm_mon--;
                    strPos += bytes; isMonth = true;
                    continue;
                }
                case 'H':
                {
                    if (isHour) throw ParsingException(
                        "Invalid date/time pattern '%s'. "
                        "Hour number already scanned !", pattern);

                    if ((result = sscanf(&str[strPos],
                        (pattern[++curPos] == 'H') ? "%2d%n":"%d%n",
                        &tmdt.tm_hour, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 ||
                        tmdt.tm_hour<0 || tmdt.tm_hour>23)
                        throw ParsingException(error,
                                               "[0,23] hour number expected",
                                               str, pattern);

                    __trace2__("Hour (H) %d <%s>",
                               tmdt.tm_hour, &str[strPos]);

                    if (pattern[curPos] == 'H') curPos++;
                    strPos += bytes;

                    needAMPM = false; isHour = true;
                    continue;
                }
                case 'h':
                {
                    if (isHour) throw ParsingException(
                        "Invalid date/time pattern '%s'. "
                        "Hour number already scanned !", pattern);

                    if ((result = sscanf(&str[strPos],
                        (pattern[++curPos] == 'h') ? "%2d%n":"%d%n",
                        &tmdt.tm_hour, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 ||
                        tmdt.tm_hour<=0 || tmdt.tm_hour>12)
                        throw ParsingException(error,
                                               "[1,12] hour number expected",
                                               str, pattern);

                    __trace2__("Hour (h) %d <%s>",
                               tmdt.tm_hour, &str[strPos]);

                    if (pattern[curPos] == 'h') curPos++;
                    strPos += bytes;

                    needAMPM = true; isHour = true;
                    continue;
                }
                case 't':
                {
                    if (isAMPM) throw ParsingException(
                        "Invalid date/time pattern '%s'. "
                        "AM/PM qualifer already scanned !", pattern);

                    curPos++;
                    std::string buff = "";

                    while (str[strPos] && isalpha(str[strPos]))
                        buff += str[strPos++];

                    //__trace2__("AM/PM = %s", buff.c_str());
                    if (compareStringsIgnoreCase(buff.c_str(),
                                                 ioDayTimeParts[0])) {
                        AMPM = false;
                    }
                    else if (compareStringsIgnoreCase(buff.c_str(),
                                                      ioDayTimeParts[1])) {
                        AMPM = true;
                    }
                    else throw ParsingException(error, "AM/PM qualifer expected",
                                                str, pattern);
                    isAMPM = true;
                    continue;
                }
                case 'm':
                {
                    if (isMins) throw ParsingException(
                        "Invalid date/time pattern '%s'. "
                        "Minutes number already scanned !", pattern);

                    if ((result = sscanf(&str[strPos],
                        (pattern[++curPos] == 'm') ? "%2d%n":"%d%n",
                        &tmdt.tm_min, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 ||
                        tmdt.tm_min<0 || tmdt.tm_min>59)
                        throw ParsingException(error, "Minutes number expected",
                                               str, pattern);

                    __trace2__("Minute %d <%s>",
                               tmdt.tm_min, &str[strPos]);

                    if (pattern[curPos] == 'm') curPos++;
                    strPos += bytes; isMins = true;
                    continue;
                }
                case 's':
                {
                    if (isSecs) throw ParsingException(
                        "Invalid date/time pattern '%s'. "
                        "Seconds number already scanned !", pattern);

                    if ((result = sscanf(&str[strPos],
                        (pattern[++curPos] == 's') ? "%2d%n":"%d%n",
                        &tmdt.tm_sec, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 ||
                        tmdt.tm_sec<0 || tmdt.tm_sec>59)
                        throw ParsingException(error, "Seconds number expected",
                                               str, pattern);

                    if (pattern[curPos] == 's') curPos++;
                    strPos += bytes; isSecs = true;
                    continue;
                }
                case 'y':
                {
                    if (pattern[curPos+1] == 'y')
                    {
                        if (pattern[curPos+2] == 'y')
                        {
                            if (pattern[curPos+3] == 'y')
                            {
                                if (isYear) throw ParsingException(
                                    "Invalid date/time pattern '%s'. "
                                    "Year number already scanned !", pattern);

                                if ((result = sscanf(&str[strPos], "%4d%n",
                                    &tmdt.tm_year, &bytes)) == EOF ||
                                    !result || !bytes || bytes<0 ||
                                    tmdt.tm_year < 1900)
                                    throw ParsingException(error,
                                        "4-digit Year number expected",
                                        str, pattern);

                                __trace2__("Year (yyyy) %d <%s>",
                                           tmdt.tm_year, &str[strPos]);

                                curPos += 4; tmdt.tm_year -= 1900;
                                strPos += bytes; isYear = true;
                                continue;
                            }
                            // else => go to default
                        }
                        else
                        {
                            if (isYear) throw ParsingException(
                                "Invalid date/time pattern '%s'. "
                                "Year number already scanned !", pattern);

                            if ((result = sscanf(&str[strPos], "%2d%n",
                                &tmdt.tm_year, &bytes)) == EOF ||
                                !result || !bytes || bytes<0 ||
                                tmdt.tm_year < 0)
                                throw ParsingException(error,
                                    "2-digit Year number expected",
                                    str, pattern);

                            __trace2__("Year (yy) %d <%s>",
                                       tmdt.tm_year, &str[strPos]);

                            curPos += 2;
                            tmdt.tm_year = (tmdt.tm_year < 70) ?
                                tmdt.tm_year+100 : tmdt.tm_year;
                            strPos += bytes; isYear = true;
                            continue;
                        }
                    }
                    // break missed => go to default !!!
                }
                default:

                    if (isalnum(str[strPos]) ||
                        isalnum(pattern[curPos]) ||
                        str[strPos] != pattern[curPos])
                            throw ParsingException(
                                "Error scanning Date/Time format! "
                                "Delimiter or format option expected. "
                                "Processing string: '%s', position %d "
                                "Pattern is: '%s', position %d",
                                str, strPos, pattern, curPos);

                    strPos++; curPos++;
                    break;
                }
            } // while

            if (needAMPM)
            {
                if (isAMPM)
                {
                    if (AMPM && tmdt.tm_hour>0 && tmdt.tm_hour<12)
                        tmdt.tm_hour += 12;
                    else if (!AMPM && tmdt.tm_hour==12)
                        tmdt.tm_hour = 0;

                    __trace2__("Hour (converted) %d", tmdt.tm_hour);
                }
                else throw ParsingException(error, "AM/PM qualifer missed",
                                            str, pattern);
            }

            time_t  current = time(NULL);
            tm      cur_tmdt;
            localtime_r(&current, &cur_tmdt); // ???

            if (!isYear)  tmdt.tm_year = cur_tmdt.tm_year;
            if (!isMonth) tmdt.tm_mon  = (isYear)  ? 0 : cur_tmdt.tm_mon;
            if (!isDay)   tmdt.tm_mday = (isMonth) ? 1 : cur_tmdt.tm_mday;
            if (!isHour)  tmdt.tm_hour = (isDay)   ? 0 : cur_tmdt.tm_hour;
            if (!isMins)  tmdt.tm_min  = (isHour)  ? 0 : cur_tmdt.tm_min;
            if (!isSecs)  tmdt.tm_sec  = (isMins)  ? 0 : cur_tmdt.tm_min;

            tmdt.tm_isdst = 0;
            date = mktime(&tmdt);
            date -= (tmdt.tm_isdst==1) ? 3600:0;

            char ctbuf[128];
            __trace2__("Scanned user input: '%s' by pattern '%s'. "
                       "Date: %s (%ld) <%d>",
                       str, pattern, ctime_r(&date,ctbuf), date, tmdt.tm_isdst);
        }
        else // scan date by default format
        {
            if ((result = sscanf(str, SMSC_DBSME_IO_DEFAULT_PARSE_PATTERN,
                &tmdt.tm_mday, &tmdt.tm_mon, &tmdt.tm_year,
                &tmdt.tm_hour, &tmdt.tm_min, &tmdt.tm_sec,
                &bytes)) == EOF || !result || !bytes || bytes<0)
                throw ParsingException("Error scanning default Date/Time format."
                                       " Processing string: '%s'", str);

            tmdt.tm_mon -= 1; tmdt.tm_year -= 1900;
            tmdt.tm_isdst = 0;
            date = mktime(&tmdt);
            date -= (tmdt.tm_isdst==1) ? 3600:0;
            strPos = bytes;
        }

        if (str != def && strPos) input.erase(0, strPos);
    }

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportDat(exp, date);

    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setDateTime(arg, date);

    __trace2__("Arg-Pos: %s, Date: %s, Less: <%s>",
               (arg) ? arg:"-", ctime(&date), input.c_str());
}

}}}
