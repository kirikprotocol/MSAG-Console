#ifndef SMSC_DBSME_IO_ENTITIES
#define SMSC_DBSME_IO_ENTITIES

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <iostream.h>
#include <core/buffers/Array.hpp>

namespace smsc { namespace dbsme { namespace io
{
    using smsc::core::buffers::Array;
    
    struct Option
    {
        std::string     name;
        std::string     value;
        
        Option(std::string _name, std::string _value)
            : name(_name), value(_value) {};
        Option(const Option& option)
            : name(option.name), value(option.value) {};
        
        Option& operator =(const Option& option) 
        {
            name = option.name; 
            value = option.value;
            return (*this);
        };
    };

    static const char* entityTypeStrings[]  = 
    { 
        "int8", "int16", "int32", "uint8", "uint16", "uint32",
        "float", "double", "long-double", "date"
        "string", "text"
    };
    
    typedef enum { 
        INT8 = 0, INT16 = 1, INT32 = 2, UINT8 = 3, UINT16 = 4, UINT32 = 5,
        FLOAT = 6, DOUBLE = 7, LDOUBLE = 8, DATE = 9,
        STRING = 10, TEXT = 11
    } EntityType;
    
    struct FormatEntity
    {
        
        EntityType      type;
        Array<Option>   options;

        FormatEntity(std::string line);
        virtual ~FormatEntity();
    };

}}}

#endif


