
#include <stdlib.h>
#include <util/debug.h>

#include "Parsers.h"

namespace smsc { namespace util { namespace templates
{
    
Hash<Parser *>  ParserRegistry::parsers;

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
    __trace2__("Command arguments count: %d", entities.Count());
    for (int i=0; i<entities.Count(); i++)
    {
        FormatEntity* entity = entities[i];
        if (entity && entity->type < TEXT)
        {
            __trace2__("Parsing arg of type: %s", 
                   ioEntityTypeStrings[entity->type]);
            Parser* parser = 
                ParserRegistry::getParser(ioEntityTypeStrings[entity->type]);
            if (parser)
            {
                int bytes = 0;
                const char* str = input.c_str();
                while (str && isspace(str[bytes])) bytes++; 
                if (bytes) input.erase(0, bytes);
                
                parser->parse(input, *entity, adapter, ctx);
            }
            else throw ParsingException("Parser for type <%s> not defined !",
                                         ioEntityTypeStrings[entity->type]);
        }
        else throw ParsingException("Type <%s> is invalid !",
                                    ioEntityTypeStrings[entity->type]);
    }
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
            || !result || !bytes || bytes<0)
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
            || !result || !bytes || bytes<0)
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
        if ((result = sscanf(str, "%ld%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning int32 type. "
                                   "Processing string: '%s'", str);
        if (str != def) input.erase(0, bytes);
    }
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, (int64_t)value);
    
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setInt32(arg, (int32_t)value);
    
    __trace2__("Arg-Pos: %s, Value: %ld, Less: <%s>", 
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
        if ((result = sscanf(str, "%u%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
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
        if ((result = sscanf(str, "%u%n", &value, &bytes)) == EOF
            || !result || !bytes || bytes<0)
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
    char* impVal = 0;
    if (imp && ctx.importStr(imp, impVal) && impVal)
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
            
            /*if (!line.length())
                throw ParsingException("Parameter of type string missed."
                                       "Processing string: '%s'", 
                                       input.c_str());*/
             input.erase(0, curPos);
        }
        else 
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
    float value = 0.0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    float impVal = 0.0;
    if (imp && ctx.importFlt(imp, impVal))
    {
        value = impVal; 
    }
    else
    {
        int bytes, result;
        const char* str = input.c_str();
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        if (!(str && *str) && def) str = def;
        double dblval = 0.0;
        if ((result = sscanf(str, "%f%n", &dblval, &bytes)) == EOF
            || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning float type. "
                                   "Processing string: '%s'", str);
        value = (float)dblval;
        if (str != def) input.erase(0, bytes);
    }
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportFlt(exp, value);
    
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    if (arg) adapter.setFloat(arg, value);
    
    __trace2__("Arg-Pos: %s, Value: %f, Less: <%s>", 
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
            date = mktime(&tmdt);
        }
        else if (&str[strPos] == strstr(&str[strPos], ioTomorrowString))
        {
            date = time(NULL) + 3600*24;
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            date = mktime(&tmdt);
        }
        else if (&str[strPos] == strstr(&str[strPos], ioYesterdayString))
        {
            date = time(NULL) - 3600*24;
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            date = mktime(&tmdt);
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
                    if ((result = sscanf(&str[strPos], 
                        (pattern[++curPos] == 'd') ? "%2d%n":"%d%n", 
                        &tmdt.tm_mday, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 || 
                        tmdt.tm_mday<=0 || tmdt.tm_mday>31)
                        throw ParsingException(error, "Month day expected",
                                               str, pattern);

                    if (pattern[curPos] == 'd') curPos++;
                    strPos += bytes; isDay = true;
                    continue;
                }
                case 'M':
                {
                    if (pattern[++curPos] == 'M')
                        if (pattern[++curPos] == 'M')
                        {
                            std::string buff = ""; bytes = 0;
                            while (isalpha(str[strPos])) buff += str[strPos++];

                            curPos++; int i;
                            for (i=0; i<12; i++)
                                if (strcmp(buff.c_str(),
                                           (pattern[curPos] == 'M') ?
                                           ioFullMonthesNames[i] :
                                           ioShortMonthesNames[i]) == 0)
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
                                !result || !bytes || bytes<0)
                                throw ParsingException(error, "Month number expected",
                                                       str, pattern);
                    else if ((result = sscanf(&str[strPos], "%d%n", 
                            &tmdt.tm_mon, &bytes)) == EOF ||
                            !result || !bytes || bytes<0)
                            throw ParsingException(error, "Month number expected",
                                                   str, pattern);

                    tmdt.tm_mon--;
                    strPos += bytes; isMonth = true;
                    continue;
                }
                case 'H':
                {
                    if ((result = sscanf(&str[strPos],
                        (pattern[++curPos] == 'H') ? "%2d%n":"%d%n", 
                        &tmdt.tm_hour, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 || 
                        tmdt.tm_hour<0 || tmdt.tm_hour>23)
                        throw ParsingException(error, 
                                               "[0,23] hour number expected",
                                               str, pattern);

                    if (pattern[curPos] == 'H') curPos++;
                    strPos += bytes;

                    needAMPM = false; isHour = true;
                    continue;
                }
                case 'h':
                {
                    if ((result = sscanf(&str[strPos], 
                        (pattern[++curPos] == 'h') ? "%2d%n":"%d%n", 
                        &tmdt.tm_hour, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 || 
                        tmdt.tm_hour<=0 || tmdt.tm_hour>12)
                        throw ParsingException(error, 
                                               "[1,12] hour number expected",
                                               str, pattern);

                    if (pattern[curPos] == 'h') curPos++;
                    strPos += bytes;

                    needAMPM = true; isHour = true;
                    continue;
                }
                case 't':
                {
                    curPos++;
                    std::string buff = "";

                    while (isalpha(str[strPos])) buff += str[strPos++];

                    if (strcmp(buff.c_str(), ioDayTimeParts[0]) == 0) 
                        AMPM = false;
                    else if (strcmp(buff.c_str(), ioDayTimeParts[1]) == 0)
                        AMPM = true;
                    else throw ParsingException(error, "AM/PM qualifer expected",
                                                str, pattern);

                    isAMPM = true;
                    continue;
                }
                case 'm':
                {
                    if ((result = sscanf(&str[strPos], 
                        (pattern[++curPos] == 'm') ? "%2d%n":"%d%n", 
                        &tmdt.tm_min, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 || 
                        tmdt.tm_min<0 || tmdt.tm_min>59)
                        throw ParsingException(error, "Minutes number expected",
                                               str, pattern);

                    if (pattern[curPos] == 'm') curPos++;
                    strPos += bytes; isMins = true;
                    continue;
                }
                case 's':
                {
                    if ((result = sscanf(&str[strPos], 
                        (pattern[++curPos] == 's') ? "%2d%n":"%d%n", 
                        &tmdt.tm_sec, &bytes)) == EOF ||
                        !result || !bytes || bytes<0 || 
                        tmdt.tm_sec<0 || tmdt.tm_sec>61)
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
                        if ((result = sscanf(&str[strPos], "%d%n", 
                            &tmdt.tm_year, &bytes)) == EOF ||
                            !result || !bytes || bytes<0)
                            throw ParsingException(error, 
                                                   "2-digit year number expected",
                                                   str, pattern);
                    
                        if (pattern[curPos+2] == 'y')
                        {
                            if (pattern[curPos+3] == 'y') 
                            {
                                curPos += 4; tmdt.tm_year -= 1900;
                                strPos += bytes; isYear = true;
                                continue;
                            }
                            // else => go to default
                        }
                        else
                        {
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
                                "Delimenter or format option expected. "
                                "Processing string: '%s', position %d "
                                "Pattern is: '%s', position %d",
                                str, strPos, pattern, curPos);
                    
                    strPos++; curPos++;
                    break;
                }
            }

            if (needAMPM)
            {
                if (isAMPM)
                {
                    if (!AMPM && tmdt.tm_hour>0 && tmdt.tm_hour<12) 
                        tmdt.tm_hour += 12; 
                    else if (AMPM && tmdt.tm_hour==12) 
                        tmdt.tm_hour = 0; 
                }
                else throw ParsingException(error, "AM/PM qualifer missed",
                                            str, pattern);
            }

            time_t  current = time(NULL);
            tm      cur_tmdt;
            localtime_r(&current, &cur_tmdt);

            if (!isYear)  tmdt.tm_year = cur_tmdt.tm_year;
            if (!isMonth) tmdt.tm_mon  = (isYear)  ? 0 : cur_tmdt.tm_mon;
            if (!isDay)   tmdt.tm_mday = (isMonth) ? 1 : cur_tmdt.tm_mday;
            if (!isHour)  tmdt.tm_hour = (isDay)   ? 0 : cur_tmdt.tm_hour;
            if (!isMins)  tmdt.tm_min  = (isHour)  ? 0 : cur_tmdt.tm_min;
            if (!isSecs)  tmdt.tm_sec  = (isMins)  ? 0 : cur_tmdt.tm_min;

            date = mktime(&tmdt);
        } 
        else // scan date by default format
        {
            if ((result = sscanf(str, "%d.%d.%d %d:%d:%d %n", 
                &tmdt.tm_mday, &tmdt.tm_mon, &tmdt.tm_year,
                &tmdt.tm_hour, &tmdt.tm_min, &tmdt.tm_sec,
                &bytes)) == EOF || !result || !bytes || bytes<0)
                throw ParsingException("Error scanning default Date/Time format."
                                       " Processing string: '%s'", str);

            tmdt.tm_mon -= 1; tmdt.tm_year -= 1900; 
            date = mktime(&tmdt);
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


