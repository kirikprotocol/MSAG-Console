#ifndef SMSC_UTIL_TEMPLATES_ADAPTERS
#define SMSC_UTIL_TEMPLATES_ADAPTERS

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <time.h>
#include <inttypes.h>

#include <util/Exception.hpp>

namespace smsc { namespace util { namespace templates
{
    using smsc::util::Exception;
    
    class AdapterException : public Exception
    {
    public:

        AdapterException(Exception& exc) 
            : Exception(exc.what()) {};
        virtual ~AdapterException() throw() {};
    };
    
    class SetAdapter
    {
    protected:
        
        SetAdapter() {};

    public:
        
        virtual ~SetAdapter() {};
        
        virtual void setString(const char* key, const char* str, bool null=false)
            throw(AdapterException) = 0;
        virtual void setInt8(const char* key, int8_t val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setInt16(const char* key, int16_t val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setInt32(const char* key, int32_t val, bool null=false)
            throw(AdapterException) = 0;
        
        virtual void setUint8(const char* key, uint8_t val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setUint16(const char* key, uint16_t val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setUint32(const char* key, uint32_t val, bool null=false)
            throw(AdapterException) = 0;
        
        virtual void setFloat(const char* key, float val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setDouble(const char* key, double val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setLongDouble(const char* key, long double val, bool null=false)
            throw(AdapterException) = 0;
        
        virtual void setDateTime(const char* key, time_t time, bool null=false)
            throw(AdapterException) = 0;
    };
    
    class GetAdapter
    {
        protected:

            GetAdapter() {};

        public:

            virtual ~GetAdapter() {};
        
        virtual bool isNull(const char* key)
            throw(AdapterException) = 0;
        
        virtual const char* getString(const char* key)
            throw(AdapterException) = 0;
        
        virtual int8_t getInt8(const char* key)
            throw(AdapterException) = 0;
        virtual int16_t getInt16(const char* key)
            throw(AdapterException) = 0;
        virtual int32_t getInt32(const char* key)
            throw(AdapterException) = 0;
        
        virtual uint8_t getUint8(const char* key)
            throw(AdapterException) = 0;
        virtual uint16_t getUint16(const char* key)
            throw(AdapterException) = 0;
        virtual uint32_t getUint32(const char* key)
            throw(AdapterException) = 0;
        
        virtual float getFloat(const char* key)
            throw(AdapterException) = 0;
        virtual double getDouble(const char* key)
            throw(AdapterException) = 0;
        virtual long double getLongDouble(const char* key)
            throw(AdapterException) = 0;

        virtual time_t getDateTime(const char* key)
            throw(AdapterException) = 0;
    };

}}}

#endif



