
#include <util/debug.h>

#include "Formatters.h"

namespace smsc { namespace util { namespace templates
{

static const char* DEFAULT_VALUE_ERR = "Default value missed (type: %s).";

FormatterRegistry::FakeHash FormatterRegistry::formatters;

OutputFormatter::OutputFormatter(const char* format)
    throw(FormatRenderingException)
        : FormatEntityRenderer(format, true)
{
    static Int8Formatter        _Int8Formatter;
    static Int16Formatter       _Int16Formatter;
    static Int32Formatter       _Int32Formatter;
    static Int64Formatter       _Int64Formatter;
    static Uint8Formatter       _Uint8Formatter;
    static Uint16Formatter      _Uint16Formatter;
    static Uint32Formatter      _Uint32Formatter;
    static Uint64Formatter      _Uint64Formatter;
    static StringFormatter      _StringFormatter;
    static TextFormatter        _TextFormatter;
    static FloatFormatter       _FloatFormatter;
    static DoubleFormatter      _DoubleFormatter;
    static LongDoubleFormatter  _LongDoubleFormatter;
    static DateTimeFormatter    _DateTimeFormatter;
}
void OutputFormatter::format(std::string& output,
    GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    for (int i=0; i<entities.Count(); i++)
    {
        FormatEntity* entity = entities[i];
        if (entity && entity->type >= 0 && entity->type < ioEntityTypesNumber)
        {
            const char* entityTypeStr = ioEntityTypeStrings[entity->type];
            __trace2__("Formatting arg of type %d '%s'", 
                       entity->type, entityTypeStr); 
            Formatter* formatter = FormatterRegistry::getFormatter(entityTypeStr);
            if (formatter) formatter->format(output, *entity, adapter, ctx);
            else throw FormattingException(
                "Formatter for type %d '%s' not defined !",
                entity->type, entityTypeStr);
        }
        else throw FormattingException("Entity type %d is invalid !", i);
    }
}

void Int8Formatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    int8_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (int8_t)impVal; 
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "int8");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getInt8(arg);
        }
    }
    char    buff[64] = "";
    sprintf(buff, "%d", value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, value);
}
void Int16Formatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    int16_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (int16_t)impVal; 
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "int16");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getInt16(arg);
        }
    }
    char    buff[64] = "";
    sprintf(buff, "%d", value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, value);
}
void Int32Formatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
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
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "int32");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getInt32(arg);
        }
    }
    char    buff[64] = "";
    sprintf(buff, "%ld", value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, value);
}
void Int64Formatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    int64_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (int64_t)impVal; 
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "int64");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getInt64(arg);
        }
    }
    char    buff[64] = "";
    sprintf(buff, "%lld", value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, value);
}
void Uint8Formatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    uint8_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (uint8_t)impVal; 
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "uint8");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getUint8(arg);
        }
    }
    char    buff[64] = "";
    sprintf(buff, "%u", value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, value);
}
void Uint16Formatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    uint16_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (uint16_t)impVal; 
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "uint16");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getUint16(arg);
        }
    }
    char    buff[64] = "";
    sprintf(buff, "%u", value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, value);
}
void Uint32Formatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
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
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "uint32");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getUint32(arg);
        }
    }
    char    buff[64] = "";
    sprintf(buff, "%lu", value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, value);
}
void Uint64Formatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    uint64_t value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    int64_t impVal = 0;
    if (imp && ctx.importInt(imp, impVal))
    {
        value = (uint64_t)impVal; 
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "uint64");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getUint64(arg);
        }
    }
    char    buff[64] = "";
    sprintf(buff, "%llu", value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportInt(exp, value);
}
void StringFormatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    const char* str = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    char* impVal = 0;
    if (imp && ctx.importStr(imp, impVal) && impVal)
    {
        str = impVal;
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (!def) 
                throw FormattingException(DEFAULT_VALUE_ERR, "string");
            else str = def;
        }
        else if (!adapter.isNull(arg))
        {
            str = adapter.getString(arg);
        }
    }
    
    if (!str) return;
    
    const char* align = entity.getOption("align");
    const char* length = entity.getOption("length");
    if (align && length)
    {
        int tmp_len = strlen(str);
        int str_len = atoi(length);
        if (str_len < tmp_len || str_len <= 0)
        {
            output += str;
        }
        else if (strcmp(align, "center") == 0)
        {
            int ind_len = (str_len-tmp_len)/2;
            output.append(ind_len, ' ');
            output += str;
            output.append(str_len-tmp_len-ind_len, ' ');
        }
        else if (strcmp(align, "right") == 0)
        {
            output.append(str_len-tmp_len, ' ');
            output += str;
        }
        else if (strcmp(align, "left") == 0)
        {
            output += str; 
            output.append(str_len-tmp_len, ' ');
        }
        else output += str;
    }
    else output += str;

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportStr(exp, str);
}
void TextFormatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    output += entity.str;
}
void FloatFormatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    double value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    double impVal = 0;
    if (imp && ctx.importDbl(imp, impVal))
    {
        value = impVal; 
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "float");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getDouble(arg);
        }
    }
    
    //double dblval 
    char    format[16] = "%lf";
    const char* pre = entity.getOption(SMSC_DBSME_IO_FORMAT_PRECISION_OPTION);
    int precision = 0; // after '.'
    if (pre && (precision = atoi(pre)) >=0)
    {
        sprintf(format, "%%.%dlf", precision);
    }
    char    buff[128] = "";
    sprintf(buff, format, value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportDbl(exp, value);
}
void DoubleFormatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    double value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    double impVal = 0;
    if (imp && ctx.importDbl(imp, impVal))
    {
        value = impVal; 
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "double");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getDouble(arg);
        }
    }
    
    char    format[16] = "%lg";
    const char* dgt = entity.getOption(SMSC_DBSME_IO_FORMAT_DIGITS_OPTION);
    int digits = 0; // significant digits
    if (dgt && (digits = atoi(dgt)) >=0)
    {
        const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPONENT_OPTION);
        if (exp && strcmp(exp, "yes") == 0) {
            sprintf(format, "%%.%dle", digits-1);
        } else {
            sprintf(format, "%%.%dlg", digits);
        }
    }
    char    buff[128] = "";
    sprintf(buff, format, value);
    output += buff;
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportDbl(exp, value);
}
void LongDoubleFormatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    long double value = 0;
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    long double impVal = 0;
    if (imp && ctx.importLdl(imp, impVal))
    {
        value = impVal; 
    }
    else
    {
        const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
        const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
        
        if (!arg || adapter.isNull(arg))
        {
            if (def) output += def;
            else throw FormattingException(DEFAULT_VALUE_ERR, "long-double");
            return;
        }
        else if (!adapter.isNull(arg))
        {
            value = adapter.getLongDouble(arg);
        }
    }
    
    char    format[16] = "%Lg";
    const char* dgt = entity.getOption(SMSC_DBSME_IO_FORMAT_DIGITS_OPTION);
    int digits = 0; // significant digits
    if (dgt && (digits = atoi(dgt)) >=0)
    {
        const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPONENT_OPTION);
        if (exp && strcmp(exp, "yes") == 0) {
            sprintf(format, "%%.%dLe", digits-1);
        } else {
            sprintf(format, "%%.%dLg", digits);
        }
    }
    char    buff[256] = "";
    sprintf(buff, format, value);
    output += buff;

    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportLdl(exp, value);
}
void DateTimeFormatter::format(std::string& output,
    FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
        throw(FormattingException, AdapterException)
{
    tm      tmdt; 
    time_t  date;
    char    buff[256] = "";

    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    const char* pattern = entity.getOption(SMSC_DBSME_IO_FORMAT_PATTERN_OPTION);
    
    const char* imp = entity.getOption(SMSC_DBSME_IO_FORMAT_IMPORT_OPTION);
    if (imp && ctx.importDat(imp, date))
    {
        // date = date;
    }
    else if ((!arg || adapter.isNull(arg)) && def)
    {
        if (strcmp(def, ioNowString) == 0)
        {
            date = time(NULL);
        }
        else if (strcmp(def, ioTodayString) == 0)
        {
            date = time(NULL);
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            date = mktime(&tmdt);
            //date -= (tmdt.tm_isdst==1) ? 3600:0;
        }
        else if (strcmp(def, ioTomorrowString) == 0)
        {
            date = time(NULL) + 3600*24;
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            date = mktime(&tmdt);
            //date -= (tmdt.tm_isdst==1) ? 3600:0;
        }
        else if (strcmp(def, ioYesterdayString) == 0)
        {
            date = time(NULL) - 3600*24;
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            date = mktime(&tmdt);
            //date -= (tmdt.tm_isdst==1) ? 3600:0;
        }
        else
        {
            output += def;
            return;
        }
    }
    else if (!def && !arg)
    {
        throw FormattingException(DEFAULT_VALUE_ERR, "date");
    }
    else if (!adapter.isNull(arg))
    {
        date = adapter.getDateTime(arg);
    }
    else throw FormattingException(DEFAULT_VALUE_ERR, "date");
    
    localtime_r(&date, &tmdt); // ???

    if (pattern) // format date by user-desired pattern
    {
        int curPos = 0;
        
        while (pattern[curPos])
        {
            switch(pattern[curPos])
            {
            case 'w':
            {
                curPos++;
                output += ioShortWeekDays[tmdt.tm_wday];
                continue;
            }
            case 'W':
            {
                curPos++;
                output += ioFullWeekDays[tmdt.tm_wday];
                continue;
            }
            case 'h':
            {
                char str[16]; int hhhh = 0;
                strftime(str, sizeof(str), "%I", &tmdt);
                sscanf(str, "%d", &hhhh);

                if (pattern[++curPos] == 'h') {
                    curPos++;
                    sprintf(buff, "%02d", hhhh);
                } else 
                    sprintf(buff, "%d", hhhh);

                output += buff;
                continue;
            }
            case 'H':
            {
                if (pattern[++curPos] == 'H') {
                    curPos++;
                    sprintf(buff, "%02d", tmdt.tm_hour);
                } else
                    sprintf(buff, "%d", tmdt.tm_hour);

                output += buff;
                continue;
            }
            case 'm':
            {
                if (pattern[++curPos] == 'm') {
                    curPos++;
                    sprintf(buff, "%02d", tmdt.tm_min);
                } else
                    sprintf(buff, "%d", tmdt.tm_min);
                
                output += buff; 
                continue;
            }
            case 's':
            {
                if (pattern[++curPos] == 's') {
                    curPos++;
                    sprintf(buff, "%02d", tmdt.tm_sec);
                } else
                    sprintf(buff, "%d", tmdt.tm_sec);
                
                output += buff; 
                continue;
            }
            case 'M':
            {
                if (pattern[++curPos] == 'M')
                {
                    if (pattern[++curPos] == 'M')
                    {
                        if (pattern[++curPos] == 'M')
                        {
                            curPos++;
                            output += ioFullMonthesNames[tmdt.tm_mon];
                        }
                        else
                            output += ioShortMonthesNames[tmdt.tm_mon];
                    }
                    else {
                        sprintf(buff, "%02d", tmdt.tm_mon+1);
                        output += buff;
                    }
                }
                else {
                    sprintf(buff, "%d", tmdt.tm_mon+1);
                    output += buff;
                }
                
                continue;
            }
            case 'd':
            {
                if (pattern[++curPos] == 'd') {
                    curPos++;
                    sprintf(buff, "%02d", tmdt.tm_mday);
                } else
                    sprintf(buff, "%d", tmdt.tm_mday);
                
                output += buff;
                continue;
            }
            case 't':
            {
                curPos++;
                output += ioDayTimeParts[(tmdt.tm_hour>=0 && tmdt.tm_hour<12) ? 0:1];
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
                            curPos += 4;
                            sprintf(buff, "%04d", tmdt.tm_year+1900);
                            output += buff;
                            continue;
                        }
                        // else => go to default
                    }
                    else
                    {
                        curPos += 2;
                        sprintf(buff, "%02d", (tmdt.tm_year >= 100) ?
                                tmdt.tm_year-100:tmdt.tm_year);
                        output += buff;
                        continue;
                    }
                }
                // break missed => go to default !!!
            }
            default:
                if (isalnum(pattern[curPos]))
                {
                    throw FormattingException("Incorrect Date/Time format! "
                                              "Delimiter or format option expected. "
                                              "Pattern is: '%s', position %d",
                                              pattern, curPos);
                }
                else output += pattern[curPos++];
                break;
            }
        }
    }
    else // format date by default format
    {
        ctime_r(&date, buff);
        int buffLen = strlen(buff);
        while (buffLen-- >= 0)
            if (buff[buffLen] == '\r' || buff[buffLen] == '\n') 
                buff[buffLen] = '\0';
            else break;
        output += buff;
    }
    
    const char* exp = entity.getOption(SMSC_DBSME_IO_FORMAT_EXPORT_OPTION);
    if (exp) ctx.exportDat(exp, date);

}

}}}


