
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
            printf("Parsing arg of type: %s\n", 
                   ioEntityTypeStrings[entity->type]);
            Parser* parser = 
                ParserRegistry::getParser(ioEntityTypeStrings[entity->type]);
            if (parser)
            {
                parser->parse(input, *entity, adapter);
            }
            else throw ParsingException("Parser for type <%s> not defined !",
                                         ioEntityTypeStrings[entity->type]);
        }
        else throw ParsingException("Type <%s> is invalid !",
                                    ioEntityTypeStrings[entity->type]);
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
    adapter.setInt32(entity.position, value);
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
    adapter.setUint8(entity.position, (uint8_t)value);
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
    adapter.setUint16(entity.position, (uint16_t)value);
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
    adapter.setUint32(entity.position, value);
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
    adapter.setFloat(entity.position, value);
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
    adapter.setDouble(entity.position, value);
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
    adapter.setLongDouble(entity.position, value);
    input.erase(0, bytes);
    printf("Pos: %d, Value: %Le, Less: <%s>\n", 
           entity.position, value, input.c_str());
}
void DateTimeParser::parse(
    std::string& input, FormatEntity& entity, SetAdapter& adapter)
        throw(ParsingException, AdapterException)
{
    char* str = (char *)input.c_str();
    const char* pattern = entity.getOption("pattern");
    const char* error = "Error scanning date type. "
                        "Processing string: '%s' "
                        "Pattern is: '%s'";
    tm  tmdt;
    int strPos = 0;
    int bytes, result = 0;
    memset(&tmdt, 0, sizeof(tmdt));
    
    if (pattern) // scan date by user-desired pattern
    {
        bool    needAMPM = false;
        bool    AMPM = false;
        int     curPos = 0;
        
        while (pattern[curPos] && str)
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
                    throw ParsingException(error, str, pattern);
                
                if (pattern[curPos] == 'd') curPos++;
                strPos += bytes;
                continue;
            }
            case 'M':
            {
                if (pattern[++curPos] == 'M')
                    if (pattern[++curPos] == 'M')
                    {
                        std::string buff = ""; bytes = 0;
                        while (isalpha(str[strPos])) buff += str[strPos++];

                        printf("Buff is: '%s'\n", buff.c_str());

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
                            throw ParsingException(error, str, pattern);
                        
                        if (pattern[curPos] == 'M') curPos++;
                    }
                    else if ((result = sscanf(&str[strPos], "%2d%n", 
                            &tmdt.tm_mon, &bytes)) == EOF ||
                            !result || !bytes || bytes<0)
                            throw ParsingException(error, str, pattern);
                else if ((result = sscanf(&str[strPos], "%d%n", 
                        &tmdt.tm_mon, &bytes)) == EOF ||
                        !result || !bytes || bytes<0)
                        throw ParsingException(error, str, pattern);
                     
                tmdt.tm_mon--;
                strPos += bytes;
                continue;
            }
            case 'H':
            {
                if ((result = sscanf(&str[strPos],
                    (pattern[++curPos] == 'H') ? "%2d%n":"%d%n", 
                    &tmdt.tm_hour, &bytes)) == EOF ||
                    !result || !bytes || bytes<0 || 
                    tmdt.tm_hour<0 || tmdt.tm_hour>23)
                    throw ParsingException(error, str, pattern);
                
                if (pattern[curPos] == 'H') curPos++;
                strPos += bytes;
                continue;
            }
            case 'h':
            {
                if ((result = sscanf(&str[strPos], 
                    (pattern[++curPos] == 'h') ? "%2d%n":"%d%n", 
                    &tmdt.tm_hour, &bytes)) == EOF ||
                    !result || !bytes || bytes<0 || 
                    tmdt.tm_hour<=0 || tmdt.tm_hour>12)
                    throw ParsingException(error, str, pattern);
                
                needAMPM = true;
                if (pattern[curPos] == 'h') curPos++;
                strPos += bytes;
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
                else throw ParsingException(error, str, pattern);
                continue;
            }
            case 'm':
            {
                if ((result = sscanf(&str[strPos], 
                    (pattern[++curPos] == 'm') ? "%2d%n":"%d%n", 
                    &tmdt.tm_min, &bytes)) == EOF ||
                    !result || !bytes || bytes<0 || 
                    tmdt.tm_min<0 || tmdt.tm_min>59)
                    throw ParsingException(error, str, pattern);
                
                if (pattern[curPos] == 'm') curPos++;
                strPos += bytes;
                continue;
            }
            case 's':
            {
                if ((result = sscanf(&str[strPos], 
                    (pattern[++curPos] == 's') ? "%2d%n":"%d%n", 
                    &tmdt.tm_sec, &bytes)) == EOF ||
                    !result || !bytes || bytes<0 || 
                    tmdt.tm_sec<0 || tmdt.tm_sec>61)
                    throw ParsingException(error, str, pattern);
                
                if (pattern[curPos] == 's') curPos++;
                strPos += bytes;
                continue;
            }
            case 'y':
            {
                if (pattern[curPos+1] == 'y')
                {
                    curPos += 2;
                    if ((result = sscanf(&str[strPos], "%d%n", 
                        &tmdt.tm_year, &bytes)) == EOF ||
                        !result || !bytes || bytes<0)
                        throw ParsingException(error, str, pattern);
                    
                    if (pattern[curPos] == 'y' && pattern[curPos+1] == 'y') 
                    {
                        curPos += 2;
                        tmdt.tm_year -= 1900;
                    } 
                    else
                    {
                        tmdt.tm_year = (tmdt.tm_year < 70) ? 
                            tmdt.tm_year+100 : tmdt.tm_year;
                    }
                    strPos += bytes;
                    continue;
                }
                // break missed !!!
            }
            default:
                printf("default '%c' && '%c'\n", 
                       str[strPos], pattern[curPos]);
                if (str[strPos] != pattern[curPos])
                    throw ParsingException(error, str, pattern);
                strPos++; curPos++;
                break;
            }
        }
        
        tmdt.tm_hour += (needAMPM && AMPM && tmdt.tm_hour<12) ? 12:0;
    } 
    else // scan date by default format
    {
        if ((result = sscanf(str, "%d.%d.%d %d:%d:%d %n", 
            &tmdt.tm_mday, &tmdt.tm_mon, &tmdt.tm_year,
            &tmdt.tm_hour, &tmdt.tm_min, &tmdt.tm_sec,
            &bytes)) == EOF || !result || !bytes || bytes<0)
            throw ParsingException("Error scanning default date type. "
                                   "Processing string: '%s'", str);
        
        tmdt.tm_mon -= 1; tmdt.tm_year -= 1900; strPos = bytes;
    }
    
    time_t date = mktime(&tmdt);
    printf("Scanned date is: %s\n", ctime(&date));
    adapter.setDateTime(entity.position, date);
    input.erase(0, strPos);
}

}}}


