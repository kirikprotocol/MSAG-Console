
#include "Parsers.h"

namespace smsc { namespace dbsme { namespace io
{
    
Hash<Parser *>  ParserRegistry::parsers;

InputParser::InputParser(ConfigView* config)
{
    static Int8Parser       _Int8Parser;
    static Int16Parser      _Int16Parser;
    static Int32Parser      _Int32Parser;
    static Uint8Parser      _Uint8Parser;
    static Uint16Parser     _Uint16Parser;
    static Uint32Parser     _Uint32Parser;
    static StringParser     _StringParser;
    static FloatParser      _FloatParser;
    static DoubleParser     _DoubleParser;
    static LongDoubleParser _LongDoubleParser;
    static DateTimeParser   _DateTimeParser;

    // ConfigView should containg *.*.input section

    std::set<std::string>* set = config->getSectionNames();
    printf("\nCat '%s'\n", config->getCategory());
    
    for (std::set<std::string>::iterator i=set->begin();i!=set->end();i++)
    {
        const char* section = (const char *)i->c_str();
        const char* line = 0;
        try
        {
            printf("Loading input parameter '%s'.\n", section);
            line = config->getString(section);
            printf("Loaded : '%s'\n", line);
        }
        catch (ConfigException& exc)
        {
            printf("Init of InputParser failed !"
                   " Config exception: %s\n", exc.what());
            if (set) delete set;
            if (line) delete line;
            throw;
        }
        if (line) delete line;
    }

    if (set) delete set;
}
InputParser::~InputParser()
{
    // Emty params here !
}
void InputParser::parse(std::string input, SetAdapter& adapter)
    throw(ParsingException, AdapterException)
{
    for (int i=0; i<params.Count(); i++)
    {
        FormatEntity* entity = params[i];
        Parser* parser = 
            ParserRegistry::getParser(entityTypeStrings[entity->type]);
        if (parser && entity)
        {
            parser->parse(input, *entity, adapter);
        }
        else throw ParsingException();
    }
}

void Int8Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void Int16Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void Int32Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void Uint8Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void Uint16Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void Uint32Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void StringParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void FloatParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void DoubleParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void LongDoubleParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}
void DateTimeParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
}

}}}


