#ifndef SMSC_INFO_SME_SQL_ADAPTERS
#define SMSC_INFO_SME_SQL_ADAPTERS

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */

#include <stdlib.h>

#include <util/templates/Adapters.h>
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

namespace smsc { namespace infosme
{
    using namespace smsc::util::templates;
    using namespace smsc::db;

    class SQLSetAdapter : public SetAdapter
    {
    protected:
        
        Statement*  base;

    public:
        
        SQLSetAdapter(Statement* statement) 
            : SetAdapter(), base(statement) {};
        virtual ~SQLSetAdapter() {};
        
        virtual void setString(const char* key, const char* str, bool null=false)
            throw(AdapterException) 
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setString(pos, str, null));
        };
        virtual void setInt8(const char* key, int8_t val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setInt8(pos, val, null));
        };
        virtual void setInt16(const char* key, int16_t val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setInt16(pos, val, null));
        };
        virtual void setInt32(const char* key, int32_t val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setInt32(pos, val, null));
        };
        virtual void setInt64(const char* key, int64_t val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setInt64(pos, val, null));
        };
        virtual void setUint8(const char* key, uint8_t val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setUint8(pos, val, null));
        };
        virtual void setUint16(const char* key, uint16_t val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setUint16(pos, val, null));
        };
        virtual void setUint32(const char* key, uint32_t val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setUint32(pos, val, null));
        };
        virtual void setUint64(const char* key, uint64_t val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setUint64(pos, val, null));
        };
        virtual void setFloat(const char* key, float val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setFloat(pos, val, null));
        };
        virtual void setDouble(const char* key, double val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setDouble(pos, val, null));
        };
        virtual void setLongDouble(const char* key, long double val, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
            SET_THROW_CAUGHT_EXCEPTION(base->setLongDouble(pos, val, null));
        };
        virtual void setDateTime(const char* key, time_t time, bool null=false)
            throw(AdapterException)
        {
            int pos = atoi(key);
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
        
        virtual bool isNull(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->isNull(pos));
        };
        virtual const char* getString(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getString(pos));
        };
        virtual int8_t getInt8(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getInt8(pos));
        };
        virtual int16_t getInt16(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getInt16(pos));
        };
        virtual int32_t getInt32(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getInt32(pos));
        };
        virtual int64_t getInt64(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getInt64(pos));
        };
        virtual uint8_t getUint8(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getUint8(pos));
        };
        virtual uint16_t getUint16(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getUint16(pos));
        };
        virtual uint32_t getUint32(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getUint32(pos));
        };
        virtual uint64_t getUint64(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getUint64(pos));
        };
        virtual float getFloat(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getFloat(pos));
        };
        virtual double getDouble(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getDouble(pos));
        };
        virtual long double getLongDouble(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
            GET_THROW_CAUGHT_EXCEPTION(base->getLongDouble(pos));
        };
        virtual time_t getDateTime(const char* key)
            throw(AdapterException)
        {
            int pos = atoi(key);
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
        
        virtual bool isNull(const char* key)
            throw(AdapterException)
        {
            return false;
        };
        virtual const char* getString(const char* key)
            throw(AdapterException)
        {
            sprintf(buff, "%lu", rows);
            return ((const char *)buff);
        };
        virtual int8_t getInt8(const char* key)
            throw(AdapterException)
        {
            return (int8_t)rows;
        };
        virtual int16_t getInt16(const char* key)
            throw(AdapterException)
        {
            return (int16_t)rows;
        };
        virtual int32_t getInt32(const char* key)
            throw(AdapterException)
        {
            return (int32_t)rows;
        };
        virtual int64_t getInt64(const char* key)
            throw(AdapterException)
        {
            return (int64_t)rows;
        };
        virtual uint8_t getUint8(const char* key)
            throw(AdapterException)
        {
            return (uint8_t)rows;
        };
        virtual uint16_t getUint16(const char* key)
            throw(AdapterException)
        {
            return (uint16_t)rows;
        };
        virtual uint32_t getUint32(const char* key)
            throw(AdapterException)
        {
            return (uint32_t)rows;
        };
        virtual uint64_t getUint64(const char* key)
            throw(AdapterException)
        {
            return (uint64_t)rows;
        };
        virtual float getFloat(const char* key)
            throw(AdapterException)
        {
            return (float)rows;
        };
        virtual double getDouble(const char* key)
            throw(AdapterException)
        {
            return (double)rows;
        };
        virtual long double getLongDouble(const char* key)
            throw(AdapterException)
        {
            return (long double)rows;
        };
        virtual time_t getDateTime(const char* key)
            throw(AdapterException)
        {
            return time(NULL);
        };
    };
    
    class SQLRoutineAdapter : public SetAdapter, public GetAdapter
    {
    protected:
        
        Routine*  base;

    public:
        
        SQLRoutineAdapter(Routine* routine) 
            : SetAdapter(), GetAdapter(), base(routine) {};
        virtual ~SQLRoutineAdapter() {};
        
        virtual bool isNull(const char* key)
            throw(AdapterException) 
        {
            GET_THROW_CAUGHT_EXCEPTION(base->isNull(key));
        };
        
        virtual void setString(const char* key, const char* str, bool null=false)
            throw(AdapterException) 
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setString(key, str, null));
        };
        virtual const char* getString(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getString(key));
        };
        
        virtual void setInt8(const char* key, int8_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setInt8(key, val, null));
        };
        virtual int8_t getInt8(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getInt8(key));
        };
        virtual void setInt16(const char* key, int16_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setInt16(key, val, null));
        };
        virtual int16_t getInt16(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getInt16(key));
        };
        virtual void setInt32(const char* key, int32_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setInt32(key, val, null));
        };
        virtual int32_t getInt32(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getInt32(key));
        };
        virtual void setInt64(const char* key, int64_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setInt64(key, val, null));
        };
        virtual int64_t getInt64(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getInt64(key));
        };
        virtual void setUint8(const char* key, uint8_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setUint8(key, val, null));
        };
        virtual uint8_t getUint8(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getUint8(key));
        };
        virtual void setUint16(const char* key, uint16_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setUint16(key, val, null));
        };
        virtual uint16_t getUint16(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getUint16(key));
        };
        virtual void setUint32(const char* key, uint32_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setUint32(key, val, null));
        };
        virtual uint32_t getUint32(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getUint32(key));
        };
        virtual void setUint64(const char* key, uint64_t val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setUint64(key, val, null));
        };
        virtual uint64_t getUint64(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getUint64(key));
        };
        virtual void setFloat(const char* key, float val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setFloat(key, val, null));
        };
        virtual float getFloat(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getFloat(key));
        };
        virtual void setDouble(const char* key, double val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setDouble(key, val, null));
        };
        virtual double getDouble(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getDouble(key));
        };
        virtual void setLongDouble(const char* key, long double val, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setLongDouble(key, val, null));
        };
        virtual long double getLongDouble(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getLongDouble(key));
        };
        virtual void setDateTime(const char* key, time_t time, bool null=false)
            throw(AdapterException)
        {
            SET_THROW_CAUGHT_EXCEPTION(base->setDateTime(key, time, null));
        };
        virtual time_t getDateTime(const char* key)
            throw(AdapterException)
        {
            GET_THROW_CAUGHT_EXCEPTION(base->getDateTime(key));
        };
    };

}}

#endif // SMSC_INFO_SME_

