
#include <stdlib.h>

#include "Parsers.h"

namespace smsc { namespace dbsme { namespace io
{
    
Hash<Parser *>  ParserRegistry::parsers;

InputParser::InputParser(const char* format)
    throw(FormatRenderingException)
        : FormatEntityRenderer(format, false)
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
}
void InputParser::parse(std::string input, SetAdapter& adapter)
    throw(ParsingException, AdapterException)
{
    printf("Command arguments count: %d\n", entities.Count());
    for (int i=0; i<entities.Count(); i++)
    {
        FormatEntity* entity = entities[i];
        if (entity && entity->type < TEXT)
        {
            printf("Parsing arg of type: %s\n", entityTypeStrings[entity->type]);
            Parser* parser = 
                ParserRegistry::getParser(entityTypeStrings[entity->type]);
            if (parser)
            {
                parser->parse(input, *entity, adapter);
            }
            else throw ParsingException("Parser for type <%s> not defined !",
                                         entityTypeStrings[entity->type]);
        }
        else throw ParsingException("Type <%s> is invalid !",
                                    entityTypeStrings[entity->type]);
    }
}

void Int8Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    int32_t value = 0;
    int bytes, result;
    const char* str = input.c_str();
    if ((result = sscanf(str, "%d%n", &value, &bytes)) == EOF
        || !result || !bytes || bytes<0)
        throw ParsingException("Error scanning int8 type. "
                               "Processing string: '%s'", str);
    adapter.setInt8(entity.position, (int8_t)value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %hd, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void Int16Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    int32_t value = 0;
    int bytes =0;
    int result;
    const char* str = input.c_str();
    if ((result = sscanf(str, "%hd%n", &value, &bytes)) == EOF
        || !result || !bytes || bytes<0)
        throw ParsingException("Error scanning int16 type. "
                               "Processing string: '%s'", str);
    adapter.setInt16(entity.position, (int16_t)value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %d, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void Int32Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    int32_t value = 0;
    int bytes, result;
    const char* str = input.c_str();
    if ((result = sscanf(str, "%ld%n", &value, &bytes)) == EOF
        || !result || !bytes || bytes<0)
        throw ParsingException("Error scanning int32 type. "
                               "Processing string: '%s'", str);
    //adapter.setInt32(entity.position, value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %ld, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void Uint8Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    uint32_t value = 0;
    int bytes, result;
    const char* str = input.c_str();
    if ((result = sscanf(str, "%hu%n", &value, &bytes)) == EOF
        || !result || !bytes || bytes<0)
        throw ParsingException("Error scanning uint8 type. "
                               "Processing string: '%s'", str);
    //adapter.setUint8(entity.position, (uint8_t)value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %hu, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void Uint16Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    uint32_t value = 0;
    int bytes, result;
    const char* str = input.c_str();
    if ((result = sscanf(str, "%u%n", &value, &bytes)) == EOF
        || !result || !bytes || bytes<0)
        throw ParsingException("Error scanning uint16 type. "
                               "Processing string: '%s'", str);
    //adapter.setUint16(entity.position, (uint16_t)value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %u, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void Uint32Parser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    uint32_t value = 0;
    int bytes, result;
    const char* str = input.c_str();
    if ((result = sscanf(str, "%lu%n", &value, &bytes)) == EOF
        || !result || !bytes || bytes<0)
        throw ParsingException("Error scanning uint32 type. "
                               "Processing string: '%s'", str);
    //adapter.setUint32(entity.position, value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %lu, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void StringParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    int curPos = 0;
    std::string line = "";
    const char* str = input.c_str();
    
    while (isspace(str[curPos])) curPos++;
        
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

    if (!line.length())
        throw ParsingException("Parameter of type string missed."
                               "Processing string: '%s'", str);
    
    adapter.setString(entity.position, line.c_str());
    input.erase(0, curPos);
    printf("Pos: %d, Value: '%s', Less: <%s>\n", 
           entity.position, line.c_str(), input.c_str());
}
void FloatParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    float value = 0.0;
    int bytes, result;
    const char* str = input.c_str();
    if ((result = sscanf(str, "%f%n", &value, &bytes)) == EOF
        || !result || !bytes || bytes<0)
        throw ParsingException("Error scanning float type. "
                               "Processing string: '%s'", str);
    //adapter.setFloat(entity.position, value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %f, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void DoubleParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    double value = 0.0;
    int bytes, result;
    const char* str = input.c_str();
    if ((result = sscanf(str, "%le%n", &value, &bytes)) == EOF
        || !result || !bytes || bytes<0)
        throw ParsingException("Error scanning double type. "
                               "Processing string: '%s'", str);
    //adapter.setDouble(entity.position, value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %le, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void LongDoubleParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    long double value = 0.0;
    int bytes, result;
    const char* str = input.c_str();
    if ((result = sscanf(str, "%Le%n", &value, &bytes)) == EOF
        || !result || !bytes || bytes<0)
        throw ParsingException("Error scanning long double type. "
                               "Processing string: '%s'", str);
    //adapter.setLongDouble(entity.position, value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %Le, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void DateTimeParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    printf("Parser for date-time type called !\n");
}

}}}


