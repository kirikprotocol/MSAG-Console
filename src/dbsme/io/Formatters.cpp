
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
            printf("Formatting arg of type: %s\n", 
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
    char    buff[64] = "";
    if (!adapter.isNull(entity.position))
    {
        sprintf(buff, "%hd", adapter.getInt8(entity.position));
    }
    output += buff;
}
void Int16Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    char    buff[64] = "";
    if (!adapter.isNull(entity.position))
    {
        sprintf(buff, "%d", adapter.getInt16(entity.position));
    }
    output += buff;
}
void Int32Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    char    buff[64] = "";
    if (!adapter.isNull(entity.position))
    {
        sprintf(buff, "%ld", adapter.getInt32(entity.position));
    }
    output += buff;
}
void Uint8Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    char    buff[64] = "";
    if (!adapter.isNull(entity.position))
    {
        sprintf(buff, "%hu", adapter.getUint8(entity.position));
    }
    output += buff;
}
void Uint16Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    char    buff[64] = "";
    if (!adapter.isNull(entity.position))
    {
        sprintf(buff, "%u", adapter.getUint16(entity.position));
    }
    output += buff;
}
void Uint32Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    char    buff[64] = "";
    if (!adapter.isNull(entity.position))
    {
        sprintf(buff, "%lu", adapter.getUint32(entity.position));
    }
    output += buff;
}
void StringFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    if (!adapter.isNull(entity.position))
    {
        const char* str = adapter.getString(entity.position);
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
                else if (align == strstr(align, "center"))
                {
                    output.append((str_len-tmp_len)/2, ' ');
                    output += str;
                    output.append((str_len-tmp_len)/2, ' ');
                }
                else if (align == strstr(align, "right"))
                {
                    output.append(str_len-tmp_len, ' ');
                    output += str;
                }
                else if (align == strstr(align, "left"))
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
    char    buff[128] = "";
    if (!adapter.isNull(entity.position))
    {
        sprintf(buff, "%f", adapter.getFloat(entity.position));
    }
    output += buff;
}
void DoubleFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    char    buff[128] = "";
    if (!adapter.isNull(entity.position))
    {
        sprintf(buff, "%e", adapter.getDouble(entity.position));
    }
    output += buff;
}
void LongDoubleFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    char    buff[256] = "";
    if (!adapter.isNull(entity.position))
    {
        sprintf(buff, "%Le", adapter.getLongDouble(entity.position));
    }
    output += buff;
}
void DateTimeFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
    if (!adapter.isNull(entity.position))
    {
        char    buff[256] = "";
        time_t date = adapter.getDateTime(entity.position);
        const char* pattern = entity.getOption("pattern");
        
        if (pattern) // format date by user-desired pattern
        {
            tm  tmdt;
            int curPos = 0;
            localtime_r(&date, &tmdt);
            
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
                    int val = tmdt.tm_hour%12;
                    if (pattern[++curPos] == 'h') {
                        curPos++;
                        sprintf(buff, ((val<10) ? "0%d":"%d"), val);
                    } else 
                        sprintf(buff, "%d", val);
                    output += buff;
                    continue;
                }
                case 'H':
                {
                    int val = tmdt.tm_hour;
                    if (pattern[++curPos] == 'H') {
                        curPos++;
                        sprintf(buff, ((val<10) ? "0%d":"%d"), val);
                    } else
                        sprintf(buff, "%d", val);
                    output += buff;
                    continue;
                }
                case 'm':
                {
                    int val = tmdt.tm_min;
                    if (pattern[++curPos] == 'm') {
                        curPos++;
                        sprintf(buff, ((val<10) ? "0%d":"%d"), val);
                    } else
                        sprintf(buff, "%d", val);
                    output += buff;
                    continue;
                }
                case 's':
                {
                    int val = tmdt.tm_sec;
                    if (pattern[++curPos] == 's') {
                        curPos++;
                        sprintf(buff, ((val<10) ? "0%d":"%d"), val);
                    } else
                        sprintf(buff, "%d", val);
                    output += buff;
                    continue;
                }
                case 'M':
                {
                    int val = tmdt.tm_mon;
                    if (pattern[++curPos] == 'M')
                    {
                        if (pattern[++curPos] == 'M')
                        {
                            if (pattern[++curPos] == 'M')
                            {
                                curPos++;
                                output += ioFullMonthesNames[val];
                            }
                            else
                                output += ioShortMonthesNames[val];
                        }
                        else
                            sprintf(buff, ((val<9) ? "0%d":"%d"), val+1);
                    }
                    else 
                        sprintf(buff, "%d", val+1);
                    output += buff;
                    continue;
                }
                case 'd':
                {
                    int val = tmdt.tm_mday;
                    if (pattern[++curPos] == 'd') {
                        curPos++;
                        sprintf(buff, ((val<10) ? "0%d":"%d"), val);
                    } else
                        sprintf(buff, "%d", val);
                    output += buff;
                    continue;
                }
                case 't':
                {
                    curPos++;
                    output += ioDayTimeParts[
                        (tmdt.tm_hour>=0 && tmdt.tm_hour<12) ? 0:1];
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
                            sprintf(buff, "%d", tmdt.tm_year+1900);
                        }
                        else
                            sprintf(buff, "%d", (tmdt.tm_year >= 100) ?
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
}

}}}


