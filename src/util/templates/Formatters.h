#ifndef SMSC_UTIL_TEMPLATES_FORMATTERS
#define SMSC_UTIL_TEMPLATES_FORMATTERS

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

    class FormattingException : public Exception
    {
    public:

        FormattingException(const char* fmt,...)
            : Exception() 
        {
            SMSC_UTIL_EX_FILL(fmt);
        };
        FormattingException() 
            : Exception("Exception occurred during output formatting !") {};
        
        virtual ~FormattingException() throw() {};
    };
    
    class Formatter;
    class FormatterRegistry
    {
    private:
        
        class FakeHash
        {
            Hash<Formatter *>& GetInstance() {
                static Hash<Formatter *>  formatters;
                return formatters;
            }

        public:

            operator Hash<Formatter *> () { 
                return GetInstance(); 
            }
            int Exists(const char* key) { 
                return GetInstance().Exists(key); 
            }
            int Insert(const char* key, Formatter* &value) {
                return GetInstance().Insert(key, value);
            }
            Formatter*& Get(const char* key) {
                return GetInstance().Get(key);
            }
        };

        static FakeHash formatters;

    public:

        static void registerFormatter(const char* key, Formatter* formatter)
        {
            if (key && formatter && !formatters.Exists(key))
            {
                formatters.Insert(key, formatter);
            }
        };
        static Formatter* getFormatter(const char* key)
        {
            return (formatters.Exists(key) ? formatters.Get(key):0);
        };
    };

    class Formatter 
    {
    protected:

        Formatter(EntityType type)
        {
            FormatterRegistry::registerFormatter(ioEntityTypeStrings[type], this);
        };

    public:

        virtual ~Formatter() {};
        
        virtual void format(std::string& output, 
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException) = 0;
    };

    class Int8Formatter : public Formatter 
    {
    public:
        
        Int8Formatter() : Formatter(ET_INT8) {};
        virtual ~Int8Formatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };
    
    class Int16Formatter : public Formatter
    {
    public:
        
        Int16Formatter() : Formatter(ET_INT16) {};
        virtual ~Int16Formatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };

    class Int32Formatter : public Formatter
    {
    public:
        
        Int32Formatter() : Formatter(ET_INT32) {};
        virtual ~Int32Formatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };
    
    class Int64Formatter : public Formatter
    {
    public:
        
        Int64Formatter() : Formatter(ET_INT64) {};
        virtual ~Int64Formatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };
    
    class Uint8Formatter : public Formatter
    {
    public:
        
        Uint8Formatter() : Formatter(ET_UINT8) {};
        virtual ~Uint8Formatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };
    
    class Uint16Formatter : public Formatter 
    {
    public:
        
        Uint16Formatter() : Formatter(ET_UINT16) {};
        virtual ~Uint16Formatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };

    class Uint32Formatter : public Formatter 
    {
    public:
        
        Uint32Formatter() : Formatter(ET_UINT32) {};
        virtual ~Uint32Formatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };
    
    class Uint64Formatter : public Formatter
    {
    public:
        
        Uint64Formatter() : Formatter(ET_UINT64) {};
        virtual ~Uint64Formatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };
    
    class StringFormatter : public Formatter 
    {
    public:
        
        StringFormatter() : Formatter(ET_STRING) {};
        virtual ~StringFormatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };
    
    class TextFormatter : public Formatter 
    {
    public:
        
        TextFormatter() : Formatter(ET_TEXT) {};
        virtual ~TextFormatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };

    class FloatFormatter : public Formatter 
    {
    public:
        
        FloatFormatter() : Formatter(ET_FLOAT) {};
        virtual ~FloatFormatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };

    class DoubleFormatter : public Formatter 
    {
    public:
        
        DoubleFormatter() : Formatter(ET_DOUBLE) {};
        virtual ~DoubleFormatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };
    
    class LongDoubleFormatter : public Formatter 
    {
    public:
        
        LongDoubleFormatter() : Formatter(ET_LDOUBLE) {};
        virtual ~LongDoubleFormatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };

    class DateTimeFormatter : public Formatter 
    {
    public:
        
        DateTimeFormatter() : Formatter(ET_DATE) {};
        virtual ~DateTimeFormatter() {};
        
        virtual void format(std::string& output,
            FormatEntity& entity, GetAdapter& adapter, ContextEnvironment& ctx)
                throw(FormattingException, AdapterException);
    };
    
    class OutputFormatter : public FormatEntityRenderer
    {
    public:

        OutputFormatter(const char* format)
            throw(FormatRenderingException);
        virtual ~OutputFormatter() {};

        void format(std::string& output,
                    GetAdapter& adapter, ContextEnvironment& ctx)
            throw(FormattingException, AdapterException);
    };

}}}

#endif


