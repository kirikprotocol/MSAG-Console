#ifndef SMSC_DBSME_IO_ADAPTERS
#define SMSC_DBSME_IO_ADAPTERS

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <time.h>
#include <inttypes.h>

#include <util/Exception.hpp>

namespace smsc { namespace dbsme { namespace io
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
        
        virtual void setString(int pos, const char* str, bool null=false)
            throw(AdapterException) = 0;
        
        virtual void setInt8(int pos, int8_t val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setInt16(int pos, int16_t val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setInt32(int pos, int32_t val, bool null=false)
            throw(AdapterException) = 0;
        
        virtual void setUint8(int pos, uint8_t val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setUint16(int pos, uint16_t val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setUint32(int pos, uint32_t val, bool null=false)
            throw(AdapterException) = 0;
        
        virtual void setFloat(int pos, float val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setDouble(int pos, double val, bool null=false)
            throw(AdapterException) = 0;
        virtual void setLongDouble(int pos, long double val, bool null=false)
            throw(AdapterException) = 0;
        
        virtual void setDateTime(int pos, time_t time, bool null=false)
            throw(AdapterException) = 0;
    };
    
    class GetAdapter
    {
        protected:

            GetAdapter() {};

        public:

            virtual ~GetAdapter() {};
        
        virtual bool isNull(int pos)
            throw(AdapterException) = 0;
        
        virtual const char* getString(int pos)
            throw(AdapterException) = 0;
        
        virtual int8_t getInt8(int pos)
            throw(AdapterException) = 0;
        virtual int16_t getInt16(int pos)
            throw(AdapterException) = 0;
        virtual int32_t getInt32(int pos)
            throw(AdapterException) = 0;
        
        virtual uint8_t getUint8(int pos)
            throw(AdapterException) = 0;
        virtual uint16_t getUint16(int pos)
            throw(AdapterException) = 0;
        virtual uint32_t getUint32(int pos)
            throw(AdapterException) = 0;
        
        virtual float getFloat(int pos)
            throw(AdapterException) = 0;
        virtual double getDouble(int pos)
            throw(AdapterException) = 0;
        virtual long double getLongDouble(int pos)
            throw(AdapterException) = 0;

        virtual time_t getDateTime(int pos)
            throw(AdapterException) = 0;
    };

}}}

#endif



