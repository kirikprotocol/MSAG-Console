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
        
        static Hash<Formatter *>    formatters;
    
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
            FormatterRegistry::registerFormatter(
                ioEntityTypeStrings[type], this);
        };

    public:

        virtual ~Formatter() {};
        
        virtual void format(std::string& output, 
                            FormatEntity& entity,
                            GetAdapter& adapter)
            throw(FormattingException, AdapterException) = 0;
    };

    class Int8Formatter : public Formatter 
    {
    public:
        
        Int8Formatter() : Formatter(INT8) {};
        virtual ~Int8Formatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };
    
    class Int16Formatter : public Formatter
    {
    public:
        
        Int16Formatter() : Formatter(INT16) {};
        virtual ~Int16Formatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };

    class Int32Formatter : public Formatter
    {
    public:
        
        Int32Formatter() : Formatter(INT32) {};
        virtual ~Int32Formatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };
    
    class Uint8Formatter : public Formatter
    {
    public:
        
        Uint8Formatter() : Formatter(UINT8) {};
        virtual ~Uint8Formatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };
    
    class Uint16Formatter : public Formatter 
    {
    public:
        
        Uint16Formatter() : Formatter(UINT16) {};
        virtual ~Uint16Formatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };

    class Uint32Formatter : public Formatter 
    {
    public:
        
        Uint32Formatter() : Formatter(UINT32) {};
        virtual ~Uint32Formatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };
    
    class StringFormatter : public Formatter 
    {
    public:
        
        StringFormatter() : Formatter(STRING) {};
        virtual ~StringFormatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };
    
    class TextFormatter : public Formatter 
    {
    public:
        
        TextFormatter() : Formatter(TEXT) {};
        virtual ~TextFormatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };

    class FloatFormatter : public Formatter 
    {
    public:
        
        FloatFormatter() : Formatter(FLOAT) {};
        virtual ~FloatFormatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };

    class DoubleFormatter : public Formatter 
    {
    public:
        
        DoubleFormatter() : Formatter(DOUBLE) {};
        virtual ~DoubleFormatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };
    
    class LongDoubleFormatter : public Formatter 
    {
    public:
        
        LongDoubleFormatter() : Formatter(LDOUBLE) {};
        virtual ~LongDoubleFormatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };

    class DateTimeFormatter : public Formatter 
    {
    public:
        
        DateTimeFormatter() : Formatter(DATE) {};
        virtual ~DateTimeFormatter() {};
        
        virtual void format(
            std::string& output, FormatEntity& entity, GetAdapter& adapter)
                throw(FormattingException, AdapterException);
    };
    
    class OutputFormatter : public FormatEntityRenderer
    {
    public:

        OutputFormatter(const char* format)
            throw(FormatRenderingException);
        virtual ~OutputFormatter() {};

        void format(std::string& output, GetAdapter& adapter)
            throw(FormattingException, AdapterException);
    };

}}}

#endif


