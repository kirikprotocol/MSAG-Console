
#include <util/debug.h>

#include "Formatters.h"

namespace smsc { namespace dbsme { namespace io
{

Hash<Formatter *>  FormatterRegistry::formatters;

OutputFormatter::OutputFormatter(const char* format)
    throw(FormatRenderingException)
        : FormatEntityRenderer(format, true)
{
    static Int8Formatter        _Int8Formatter;
    static Int16Formatter       _Int16Formatter;
    static Int32Formatter       _Int32Formatter;
    static Uint8Formatter       _Uint8Formatter;
    static Uint16Formatter      _Uint16Formatter;
    static Uint32Formatter      _Uint32Formatter;
    static StringFormatter      _StringFormatter;
    static TextFormatter        _TextFormatter;
    static FloatFormatter       _FloatFormatter;
    static DoubleFormatter      _DoubleFormatter;
    static LongDoubleFormatter  _LongDoubleFormatter;
    static DateTimeFormatter    _DateTimeFormatter;
}
void OutputFormatter::format(std::string& output, GetAdapter& adapter)
    throw(FormattingException, AdapterException)
{
    for (int i=0; i<entities.Count(); i++)
    {
        FormatEntity* entity = entities[i];
        if (entity)
        {
            __trace2__("Formatting arg of type: %s", 
                       ioEntityTypeStrings[entity->type]); 
            Formatter* formatter = 
                FormatterRegistry::getFormatter(ioEntityTypeStrings[entity->type]);
            if (formatter && entity)
            {
                formatter->format(output, *entity, adapter);
            }
            else throw FormattingException(
                "Formatter for type <%s> not defined !",
                ioEntityTypeStrings[entity->type]);
        }
        else throw FormattingException("Type <%s> is invalid !",
                                       ioEntityTypeStrings[entity->type]);
    }
}

void Int8Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);

    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        char    buff[64] = "";
        sprintf(buff, "%hd", adapter.getInt8(arg));
        output += buff;
    }
}
void Int16Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);

    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        char    buff[64] = "";
        sprintf(buff, "%d", adapter.getInt16(arg));
        output += buff;
    }
}
void Int32Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);

    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        char    buff[64] = "";
        sprintf(buff, "%ld", adapter.getInt32(arg));
        output += buff;
    }
}
void Uint8Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);

    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        char    buff[64] = "";
        sprintf(buff, "%hu", adapter.getUint8(arg));
        output += buff;
    }
}
void Uint16Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);

    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        char    buff[64] = "";
        sprintf(buff, "%u", adapter.getUint16(arg));
        output += buff;
    }
}
void Uint32Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);

    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        char    buff[64] = "";
        sprintf(buff, "%lu", adapter.getUint32(arg));
        output += buff;
    }
}
void StringFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);

    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        const char* str = adapter.getString(arg);
        if (str)
        {
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
                    output.append((str_len-tmp_len)/2, ' ');
                    output += str;
                    output.append((str_len-tmp_len)/2, ' ');
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
        }
    }
}
void TextFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    output += entity.str;
}
void FloatFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    
    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        char    format[8] = "%f";
        const char* pre = 
            entity.getOption(SMSC_DBSME_IO_FORMAT_PRECISION_OPTION);
        int precision = 0; // after '.'
        if (pre && (precision = atoi(pre)) >=0)
        {
            sprintf(format, "%%.%d%%f", precision);
        }
        char    buff[128] = "";
        sprintf(buff, format, adapter.getFloat(arg));
        output += buff;
    }
}
void DoubleFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);

    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        char    format[8] = "%lg";
        const char* dgt = 
            entity.getOption(SMSC_DBSME_IO_FORMAT_DIGITS_OPTION);
        int digits = 0; // significant digits
        if (dgt && (digits = atoi(dgt)) >=0)
        {
            const char* exp = 
                entity.getOption(SMSC_DBSME_IO_FORMAT_EXPONENT_OPTION);
            if (exp && strcmp(exp, "yes") == 0) {
                sprintf(format, "%%.%d%%le", digits);
            } else {
                sprintf(format, "%%.%d%%lg", digits);
            }
        }
        char    buff[128] = "";
        sprintf(buff, format, adapter.getDouble(arg));
        output += buff;
    }
}
void LongDoubleFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);

    if ((!arg || adapter.isNull(arg)) && def)
    {
        output += def;
    }
    else if (!adapter.isNull(arg))
    {
        char    format[8] = "%Lg";
        const char* dgt = 
            entity.getOption(SMSC_DBSME_IO_FORMAT_DIGITS_OPTION);
        int digits = 0; // significant digits
        if (dgt && (digits = atoi(dgt)) >=0)
        {
            const char* exp = 
                entity.getOption(SMSC_DBSME_IO_FORMAT_EXPONENT_OPTION);
            if (exp && strcmp(exp, "yes") == 0) {
                sprintf(format, "%%.%d%%Le", digits);
            } else {
                sprintf(format, "%%.%d%%Lg", digits);
            }
        }
        char    buff[256] = "";
        sprintf(buff, format, adapter.getLongDouble(arg));
        output += buff;
    }
}
void DateTimeFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    tm      tmdt;
    time_t  date;
    char    buff[256] = "";

    const char* def = entity.getOption(SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION);
    const char* arg = entity.getOption(SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION);
    const char* pattern = entity.getOption(SMSC_DBSME_IO_FORMAT_PATTERN_OPTION);
    
    if ((!arg || adapter.isNull(arg)) && def)
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
        }
        else if (strcmp(def, ioTomorrowString) == 0)
        {
            date = time(NULL) + 3600*24;
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            date = mktime(&tmdt);
        }
        else if (strcmp(def, ioYesterdayString) == 0)
        {
            date = time(NULL) - 3600*24;
            localtime_r(&date, &tmdt);
            tmdt.tm_hour = tmdt.tm_min = tmdt.tm_sec = 0;
            date = mktime(&tmdt);
        }
        else
        {
            output += def;
            return;
        }
    }
    else if (!adapter.isNull(arg))
    {
        date = adapter.getDateTime(arg);
    }
    else return;
    
    localtime_r(&date, &tmdt);

    if (pattern) // format date by user-desired pattern
    {
        int curPos = 0;
        bool needAMPM = false;
        
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
                if (pattern[++curPos] == 'h') {
                    curPos++;
                    sprintf(buff, "%02d", tmdt.tm_hour%12);
                } else 
                    sprintf(buff, "%d", tmdt.tm_hour%12);

                needAMPM = true;
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

                output += buff; needAMPM = false;
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
                    else
                        sprintf(buff, "%02d", tmdt.tm_mon+1);
                }
                else 
                    sprintf(buff, "%d", tmdt.tm_mon+1);
                
                output += buff;
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
                if (needAMPM)
                {
                    output += ioDayTimeParts[
                        (tmdt.tm_hour>=0 && tmdt.tm_hour<12) ? 0:1];
                }
                continue;
            }
            case 'y':
            {
                if (pattern[curPos+1] == 'y')
                {
                    curPos += 2;
                    if (pattern[curPos] == 'y' && 
                        pattern[curPos+1] == 'y') 
                    {
                        curPos += 2;
                        sprintf(buff, "%04d", tmdt.tm_year+1900);
                    }
                    else
                        sprintf(buff, "%02d", (tmdt.tm_year >= 100) ?
                                tmdt.tm_year-100:tmdt.tm_year);
                    output += buff;
                    continue;
                }
                // break missed !!!
            }
            default:
                output += pattern[curPos++];
                break;
            }
        }
    }
    else // format date by default format
    {
        ctime_r(&date, buff);   
        output += buff;
    }
}

}}}


