#ifndef SMSC_DBSME_IO_SQL_ADAPTERS
#define SMSC_DBSME_IO_SQL_ADAPTERS

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <dbsme/io/Adapters.h>
#include <db/DataSource.h>

#define SET_THROW_CAUGHT_EXCEPTION(stmt)    \
try                                         \
{                                           \
    stmt;                                   \
}                                           \
catch (Exception& exc)                      \
{                                           \
    /*log.error(exc.what());*/              \
    throw AdapterException(exc);            \
}

#define GET_THROW_CAUGHT_EXCEPTION(stmt)    \
try                                         \
{                                           \
    return (stmt);                          \
}                                           \
catch (Exception& exc)                      \
{                                           \
    /*log.error(exc.what());*/              \
    throw AdapterException(exc);            \
}

namespace smsc { namespace dbsme { namespace io
{
    using namespace smsc::db;

    class SQLSetAdapter : public SetAdapter
    {
    protected:
        
        Statement*  base;

    public:
        
        SQLSetAdapter(Statement* statement) 
            : SetAdapter(), base(statement) {};
        virtual ~SQLSetAdapter() {};
        
        virtual void setString(int pos, const char* str, bool null=false)
            throw(AdapterException) 
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setString(pos, str, null));
        };
        virtual void setInt8(int pos, int8_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setInt8(pos, val, null));
        };
        virtual void setInt16(int pos, int16_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setInt16(pos, val, null));
        };
        virtual void setInt32(int pos, int32_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setInt32(pos, val, null));
        };
        virtual void setUint8(int pos, uint8_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setUint8(pos, val, null));
        };
        virtual void setUint16(int pos, uint16_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setUint16(pos, val, null));
        };
        virtual void setUint32(int pos, uint32_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setUint32(pos, val, null));
        };
        virtual void setFloat(int pos, float val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setFloat(pos, val, null));
        };
        virtual void setDouble(int pos, double val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setDouble(pos, val, null));
        };
        virtual void setLongDouble(int pos, long double val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setLongDouble(pos, val, null));
        };
        virtual void setDateTime(int pos, time_t time, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setDateTime(pos, time, null));
        };

    };
    
    class SQLGetAdapter : public GetAdapter
    {
    protected:
        
        ResultSet*  base;

    public:
        
        SQLGetAdapter(ResultSet* resultset) 
            : GetAdapter(), base(resultset) {};
        virtual ~SQLGetAdapter() {};
        
        virtual bool isNull(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->isNull(pos));
        };
        virtual const char* getString(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getString(pos));
        };
        virtual int8_t getInt8(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getInt8(pos));
        };
        virtual int16_t getInt16(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getInt16(pos));
        };
        virtual int32_t getInt32(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getInt32(pos));
        };
        virtual uint8_t getUint8(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getUint8(pos));
        };
        virtual uint16_t getUint16(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getUint16(pos));
        };
        virtual uint32_t getUint32(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getUint32(pos));
        };
        virtual float getFloat(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getFloat(pos));
        };
        virtual double getDouble(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getDouble(pos));
        };
        virtual long double getLongDouble(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getLongDouble(pos));
        };
        virtual time_t getDateTime(int pos)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getDateTime(pos));
        };
    
    };
    
    class SQLGetRowsAdapter : public GetAdapter
    {
    protected:
        
        uint32_t    rows;
        char        buff[32];

    public:
        
        SQLGetRowsAdapter(uint32_t result) 
            : GetAdapter(), rows(result) {};
        virtual ~SQLGetRowsAdapter() {};
        
        virtual bool isNull(int pos)
            throw(AdapterException)
        {
            return false;
        };
        virtual const char* getString(int pos)
            throw(AdapterException)
        {
            sprintf(buff, "%lu", rows);
            return ((const char *)buff);
        };
        virtual int8_t getInt8(int pos)
            throw(AdapterException)
        {
            return (int8_t)rows;
        };
        virtual int16_t getInt16(int pos)
            throw(AdapterException)
        {
            return (int16_t)rows;
        };
        virtual int32_t getInt32(int pos)
            throw(AdapterException)
        {
            return (int32_t)rows;
        };
        virtual uint8_t getUint8(int pos)
            throw(AdapterException)
        {
            return (uint8_t)rows;
        };
        virtual uint16_t getUint16(int pos)
            throw(AdapterException)
        {
            return (uint16_t)rows;
        };
        virtual uint32_t getUint32(int pos)
            throw(AdapterException)
        {
            return (uint32_t)rows;
        };
        virtual float getFloat(int pos)
            throw(AdapterException)
        {
            return (float)rows;
        };
        virtual double getDouble(int pos)
            throw(AdapterException)
        {
            return (double)rows;
        };
        virtual long double getLongDouble(int pos)
            throw(AdapterException)
        {
            return (long double)rows;
        };
        virtual time_t getDateTime(int pos)
            throw(AdapterException)
        {
            return time(NULL);
        };
    
    };

}}}

#endif

