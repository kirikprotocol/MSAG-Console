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
    
    static const char* SMSC_DBSME_IO_FORMAT_ARGUMENT_OPTION  = "arg";
    static const char* SMSC_DBSME_IO_FORMAT_DEFAULT_OPTION   = "default";
    static const char* SMSC_DBSME_IO_FORMAT_PATTERN_OPTION   = "pattern";
    static const char* SMSC_DBSME_IO_FORMAT_PRECISION_OPTION = "precision";
    static const char* SMSC_DBSME_IO_FORMAT_EXPONENT_OPTION  = "exponent";
    static const char* SMSC_DBSME_IO_FORMAT_DIGITS_OPTION    = "digits";
    
    static const char  SMSC_DBSME_IO_FORMAT_ENTITY_DELIMETER = '$';
    static const char  SMSC_DBSME_IO_FORMAT_STRING_DELIMETER = '\"';
    static const char  SMSC_DBSME_IO_FORMAT_ENTITY_ESCAPER   = '\\';
    static const char  SMSC_DBSME_IO_FORMAT_OPTION_ASSIGN    = '=';

    static const char*  ioNowString         = "now";
    static const char*  ioTodayString       = "today";
    static const char*  ioTomorrowString    = "tomorrow";
    static const char*  ioYesterdayString   = "yesterday";
    
    static const char*  ioFullMonthesNames[12] = {
        "January", "February", "March", "April", 
        "May", "June", "July", "August", "September",
        "October", "November", "December"
    };
    static const char*  ioShortMonthesNames[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    static const char*  ioFullWeekDays[7] = {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
    };
    static const char*  ioShortWeekDays[7] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };
    static const char*  ioDayTimeParts[2] = {
        "AM", "PM"
    };
    
    const int ioEntityTypesNumber = 12;
    static const char* ioEntityTypeStrings[ioEntityTypesNumber]  = 
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

        FormatEntity(std::string line, bool io, bool type=true)
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

        FormatEntityRenderer(const char* format, bool io)
            throw(FormatRenderingException);
        virtual ~FormatEntityRenderer();
    };

}}}

#endif


