#ifndef SMSC_UTIL_TEMPLATES_PARSERS
#define SMSC_UTIL_TEMPLATES_PARSERS

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

namespace smsc { namespace util { namespace templates
{
    using smsc::core::buffers::Array;
    using smsc::core::buffers::Hash;
    
    using smsc::util::Exception;

    struct ParsingException : public Exception
    {
        ParsingException(const char* fmt,...)
            : Exception() 
        {
            SMSC_UTIL_EX_FILL(fmt);
        };
        ParsingException() 
            : Exception("Exception occurred during input parsing !") {};
        
        virtual ~ParsingException() throw() {};
    };
    struct ParsingWarning : public ParsingException
    {
        ParsingWarning(const char* fmt,...)
            : ParsingException("") 
        {
            SMSC_UTIL_EX_FILL(fmt);
        };
        
        virtual ~ParsingWarning() throw() {};
    };
    
    class Parser;
    class ParserRegistry
    {
    private:
        
        class FakeHash
        {
            Hash<Parser *>& GetInstance() {
                static Hash<Parser *>  _parsers;
                return _parsers;
            }

        public:

            operator Hash<Parser *> () { 
                return GetInstance(); 
            }
            int Exists(const char* key) { 
                return GetInstance().Exists(key); 
            }
            int Insert(const char* key, Parser* &value) {
                return GetInstance().Insert(key, value);
            }
            Parser*& Get(const char* key) {
                return GetInstance().Get(key);
            }
        };

        static FakeHash parsers;

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
            ParserRegistry::registerParser(ioEntityTypeStrings[type], this);
        };

    public:

        virtual ~Parser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException) = 0;
    };
    
    class Int8Parser : public Parser 
    {
    public:
        
        Int8Parser() : Parser(ET_INT8) {};
        virtual ~Int8Parser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };
    
    class Int16Parser : public Parser
    {
    public:
        
        Int16Parser() : Parser(ET_INT16) {};
        virtual ~Int16Parser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };

    class Int32Parser : public Parser
    {
    public:
        
        Int32Parser() : Parser(ET_INT32) {};
        virtual ~Int32Parser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };
    
    class Int64Parser : public Parser
    {
    public:
        
        Int64Parser() : Parser(ET_INT64) {};
        virtual ~Int64Parser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };
    
    class Uint8Parser : public Parser
    {
    public:
        
        Uint8Parser() : Parser(ET_UINT8) {};
        virtual ~Uint8Parser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };
    
    class Uint16Parser : public Parser 
    {
    public:
        
        Uint16Parser() : Parser(ET_UINT16) {};
        virtual ~Uint16Parser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };

    class Uint32Parser : public Parser 
    {
    public:
        
        Uint32Parser() : Parser(ET_UINT32) {};
        virtual ~Uint32Parser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };
    
    class Uint64Parser : public Parser 
    {
    public:
        
        Uint64Parser() : Parser(ET_UINT64) {};
        virtual ~Uint64Parser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };
    
    class StringParser : public Parser 
    {
    public:
        
        StringParser() : Parser(ET_STRING) {};
        virtual ~StringParser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };
    
    class FloatParser : public Parser 
    {
    public:
        
        FloatParser() : Parser(ET_FLOAT) {};
        virtual ~FloatParser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };

    class DoubleParser : public Parser 
    {
    public:
        
        DoubleParser() : Parser(ET_DOUBLE) {};
        virtual ~DoubleParser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };
    
    class LongDoubleParser : public Parser 
    {
    public:
        
        LongDoubleParser() : Parser(ET_LDOUBLE) {};
        virtual ~LongDoubleParser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };

    class DateTimeParser : public Parser 
    {
    public:
        
        DateTimeParser() : Parser(ET_DATE) {};
        virtual ~DateTimeParser() {};
        
        virtual void parse(std::string& input,
            FormatEntity& entity, SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };

    class InputParser : public FormatEntityRenderer 
    {
    public:

        InputParser(const char* format)
            throw(FormatRenderingException);
        virtual ~InputParser() {};

        virtual void parse(std::string& input,
            SetAdapter& adapter, ContextEnvironment& ctx)
                throw(ParsingException, AdapterException);
    };

}}}

#endif


