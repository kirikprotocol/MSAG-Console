
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
                   entityTypeStrings[entity->type]); 
            Formatter* formatter = 
                FormatterRegistry::getFormatter(entityTypeStrings[entity->type]);
            if (formatter && entity)
            {
                formatter->format(output, *entity, adapter);
            }
            else throw FormattingException(
                "Formatter for type <%s> not defined !",
                entityTypeStrings[entity->type]);
        }
        else throw FormattingException("Type <%s> is invalid !",
                                       entityTypeStrings[entity->type]);
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
    char    buff[128] = "";
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
    char    buff[256] = "";
    if (!adapter.isNull(entity.position))
    {
        time_t date = adapter.getDateTime(entity.position);
        //sprintf(buff, "%", );
    }
    output += buff;
}

}}}


