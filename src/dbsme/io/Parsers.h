#ifndef SMSC_DBSME_IO_PARSERS
#define SMSC_DBSME_IO_PARSERS

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <core/buffers/Array.hpp>
#include <core/buffers/Hash.hpp>
#include <util/Exception.hpp>

#include "Adapters.h"
#include "Entities.h"

namespace smsc { namespace dbsme { namespace io
{
    using smsc::core::buffers::Array;
    using smsc::core::buffers::Hash;
    
    using smsc::util::Exception;

    class ParsingException : public Exception
    {
    public:
        
        ParsingException(const char* fmt,...)
            : Exception() 
        {
            SMSC_UTIL_EX_FILL(fmt);
        };
        ParsingException() 
            : Exception("Exception occurred during input parsing !") {};
        
        virtual ~ParsingException() throw() {};
    };
    
    class Parser;
    class ParserRegistry
    {
    private:
        
        static Hash<Parser *>  parsers;
    
    public:

        static void registerParser(const char* key, Parser* parser)
        {
            if (key && parser && !parsers.Exists(key))
            {
                parsers.Insert(key, parser);
            }
        };
        static Parser* getParser(const char* key)
        {
            return (parsers.Exists(key) ? parsers.Get(key):0);
        };
    };

    class Parser 
    {
    protected:
        
        Parser(EntityType type) 
        {
            ParserRegistry::registerParser(entityTypeStrings[type], this);
        };

    public:

        virtual ~Parser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException) = 0;
    };
    
    class Int8Parser : public Parser 
    {
    public:
        
        Int8Parser() : Parser(INT8) {};
        virtual ~Int8Parser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };
    
    class Int16Parser : public Parser
    {
    public:
        
        Int16Parser() : Parser(INT16) {};
        virtual ~Int16Parser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };

    class Int32Parser : public Parser
    {
    public:
        
        Int32Parser() : Parser(INT32) {};
        virtual ~Int32Parser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };
    
    class Uint8Parser : public Parser
    {
    public:
        
        Uint8Parser() : Parser(UINT8) {};
        virtual ~Uint8Parser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };
    
    class Uint16Parser : public Parser 
    {
    public:
        
        Uint16Parser() : Parser(UINT16) {};
        virtual ~Uint16Parser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };

    class Uint32Parser : public Parser 
    {
    public:
        
        Uint32Parser() : Parser(UINT32) {};
        virtual ~Uint32Parser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };
    
    class StringParser : public Parser 
    {
    public:
        
        StringParser() : Parser(STRING) {};
        virtual ~StringParser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };
    
    class FloatParser : public Parser 
    {
    public:
        
        FloatParser() : Parser(FLOAT) {};
        virtual ~FloatParser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };

    class DoubleParser : public Parser 
    {
    public:
        
        DoubleParser() : Parser(DOUBLE) {};
        virtual ~DoubleParser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };
    
    class LongDoubleParser : public Parser 
    {
    public:
        
        LongDoubleParser() : Parser(LDOUBLE) {};
        virtual ~LongDoubleParser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };

    class DateTimeParser : public Parser 
    {
    public:
        
        DateTimeParser() : Parser(DATE) {};
        virtual ~DateTimeParser() {};
        
        virtual void parse(
            std::string& input, FormatEntity& entity, SetAdapter& adapter)
                throw(ParsingException, AdapterException);
    };

    class InputParser : public FormatEntityRenderer 
    {
    public:

        InputParser(const char* format)
            throw(FormatRenderingException);
        virtual ~InputParser() {};

        void parse(std::string input, SetAdapter& adapter)
            throw(ParsingException, AdapterException);
    };

}}}

#endif


