
#include "Formatters.h"

namespace smsc { namespace dbsme { namespace io
{

Hash<Formatter *>  FormatterRegistry::formatters;

OutputFormatter::OutputFormatter(ConfigView* config)
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
OutputFormatter::~OutputFormatter()
{
    // Emty params here !
}
void OutputFormatter::format(std::string& output, GetAdapter& adapter)
    throw(FormattingException, AdapterException)
{
    for (int i=0; i<params.Count(); i++)
    {
        FormatEntity* entity = params[i];
        Formatter* formatter = 
            FormatterRegistry::getFormatter(entityTypeStrings[entity->type]);
        if (formatter && entity)
        {
            formatter->format(output, *entity, adapter);
        }
        else throw FormattingException();
    }
}

void Int8Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void Int16Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void Int32Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void Uint8Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void Uint16Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void Uint32Formatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void StringFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void TextFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void FloatFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void DoubleFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void LongDoubleFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}
void DateTimeFormatter::format(
    std::string& output, FormatEntity& entity, GetAdapter& adapter)
        throw(FormattingException, AdapterException)
{
}

}}}


