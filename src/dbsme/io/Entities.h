#ifndef SMSC_DBSME_IO_ENTITIES
#define SMSC_DBSME_IO_ENTITIES

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <iostream.h>
#include <core/buffers/Hash.hpp>
#include <core/buffers/Array.hpp>
#include <util/Exception.hpp>

namespace smsc { namespace dbsme { namespace io
{
    using smsc::core::buffers::Hash;
    using smsc::core::buffers::Array;

    using smsc::util::Exception;
    
    class FormatRenderingException : public Exception
    {
    public:

        FormatRenderingException(const char* fmt,...)
            : Exception() 
        {
            SMSC_UTIL_EX_FILL(fmt);
        };
        FormatRenderingException() 
            : Exception("Exception occurred during format rendering !") {};
        
        virtual ~FormatRenderingException() throw() {};
    };
    
    static const char* SMSC_DBSME_IO_FORMAT_ARGUMENT_NUMBER  = "arg";

    static const char  SMSC_DBSME_IO_FORMAT_ENTITY_DELIMETER = '$';
    static const char  SMSC_DBSME_IO_FORMAT_STRING_DELIMETER = '\"';
    static const char  SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER   = '\\';
    static const char  SMSC_DBSME_IO_FORMAT_OPTION_ASSIGN    = '=';

    const int entityTypesNumber = 12;
    static const char* entityTypeStrings[entityTypesNumber]  = 
    { 
        "int8", "int16", "int32", "uint8", "uint16", "uint32",
        "float", "double", "long-double", "date",
        "string", "text"
    };
    
    typedef enum { 
        INT8 = 0, INT16 = 1, INT32 = 2, UINT8 = 3, UINT16 = 4, UINT32 = 5,
        FLOAT = 6, DOUBLE = 7, LDOUBLE = 8, DATE = 9,
        STRING = 10, TEXT = 11
    } EntityType;
    
    struct FormatEntity
    {
        
        EntityType          type;
        Hash<std::string>   options;
        std::string         str;
        int                 position;

        FormatEntity(std::string line, bool type=true)
            throw(FormatRenderingException);
        virtual ~FormatEntity();

        const char* getOption(const char* name);

    private:

        void renderOptions(const char* line)
            throw(FormatRenderingException);
    };

    class FormatEntityRenderer
    {
    protected:

        Array<FormatEntity *>   entities;

        void clearEntities();

    public:

        FormatEntityRenderer(const char* format, bool text = false)
            throw(FormatRenderingException);
        virtual ~FormatEntityRenderer();
    };

}}}

#endif


