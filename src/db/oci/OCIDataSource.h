#ifndef SMSC_DB_OCI_DATASOURCE
#define SMSC_DB_OCI_DATASOURCE

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */
 
#include <oci.h>
#include <orl.h>

#include <util/debug.h>
#include <db/DataSource.h>

using namespace smsc::db;

extern "C" 
DataSourceFactory*  getDataSourceFactory(void);

namespace smsc { namespace db { namespace oci
{
    struct OCIDataDescriptor 
    {
        ub2         type;
        sb4         size;
        sb2         ind;
        dvoid*      data;
        OCIDate     date;
        OCINumber   number;

        OCIDataDescriptor(ub2 type, sb4 size);
        ~OCIDataDescriptor();
    };
    
    class OCIStatement;
    class OCIResultSet : public ResultSet
    {
    protected:
        
        OCIStatement* owner;

        Array<OCIDataDescriptor *>  descriptors;

        dvoid* getField(int pos)
            throw (InvalidArgumentException);

    public:
        
        OCIResultSet(OCIStatement* statement)
            throw(SQLException);
        virtual ~OCIResultSet();

        virtual bool fetchNext()
            throw(SQLException);
        
        virtual bool isNull(int pos)
            throw(SQLException, InvalidArgumentException);

        virtual const char* getString(int pos)
            throw(SQLException, InvalidArgumentException);
        
        virtual int8_t getInt8(int pos)
            throw(SQLException, InvalidArgumentException);
        virtual int16_t getInt16(int pos)
            throw(SQLException, InvalidArgumentException);
        virtual int32_t getInt32(int pos)
            throw(SQLException, InvalidArgumentException);
        virtual int64_t getInt64(int pos)
            throw(SQLException, InvalidArgumentException);
        
        virtual uint8_t getUint8(int pos)
            throw(SQLException, InvalidArgumentException);
        virtual uint16_t getUint16(int pos)
            throw(SQLException, InvalidArgumentException);
        virtual uint32_t getUint32(int pos)
            throw(SQLException, InvalidArgumentException);
        virtual uint64_t getUint64(int pos)
            throw(SQLException, InvalidArgumentException);
        
        virtual float getFloat(int pos)
            throw(SQLException, InvalidArgumentException);
        virtual double getDouble(int pos)
            throw(SQLException, InvalidArgumentException);
        virtual long double getLongDouble(int pos)
            throw(SQLException, InvalidArgumentException);
        
        virtual time_t getDateTime(int pos)
            throw(SQLException, InvalidArgumentException);
        /* ... */
    };
    
    class OCIConnection;
    class OCIStatement : public Statement
    {
    friend class OCIResultSet;
    protected:

        OCIConnection   *owner;

        Array<OCIDataDescriptor *>  descriptors;

        OCIEnv          *envhp;
        OCISvcCtx       *svchp;
        OCIError        *errhp;
        OCIStmt         *stmt;

        void convertDateToOCIDate(time_t* sms_date, OCIDate* oci_date);
        void convertOCIDateToDate(OCIDate* oci_date, time_t* sms_date);
        
        void check(sword status) 
            throw(SQLException);

        void bind(ub4 pos, ub2 type, 
                  dvoid* placeholder, sb4 size, dvoid* indp = 0) 
            throw(SQLException);
        void bind(CONST text* name, sb4 name_len, ub2 type,
                  dvoid* placeholder, sb4 size, dvoid* indp = 0)
            throw(SQLException);
        void define(ub4 pos, ub2 type, 
                    dvoid* placeholder, sb4 size, dvoid* indp = 0)
            throw(SQLException);
        
        sword execute(ub4 mode, ub4 iters, ub4 rowoff)
            throw(SQLException);
        sword fetch()
            throw(SQLException);
        
        ub4 getRowsAffectedCount();

        OCIDataDescriptor* setField(int pos, ub2 type, ub4 size,
                                    bool null=false);
    public:

        OCIStatement(OCIConnection* connection, const char* sql) 
            throw(SQLException);
        virtual ~OCIStatement();
        
        virtual void execute() 
            throw(SQLException);
        virtual uint32_t executeUpdate() 
            throw(SQLException);
        virtual ResultSet* executeQuery() 
            throw(SQLException);
        
        virtual void setString(int pos, const char* str, bool null=false)
            throw(SQLException);
        
        virtual void setInt8(int pos, int8_t val, bool null=false)
            throw(SQLException);
        virtual void setInt16(int pos, int16_t val, bool null=false)
            throw(SQLException);
        virtual void setInt32(int pos, int32_t val, bool null=false)
            throw(SQLException);
        virtual void setInt64(int pos, int64_t val, bool null=false)
            throw(SQLException);
        
        virtual void setUint8(int pos, uint8_t val, bool null=false)
            throw(SQLException);
        virtual void setUint16(int pos, uint16_t val, bool null=false)
            throw(SQLException);
        virtual void setUint32(int pos, uint32_t val, bool null=false)
            throw(SQLException);
        virtual void setUint64(int pos, uint64_t val, bool null=false)
            throw(SQLException);
        
        virtual void setFloat(int pos, float val, bool null=false)
            throw(SQLException);
        virtual void setDouble(int pos, double val, bool null=false)
            throw(SQLException);
        virtual void setLongDouble(int pos, long double val, bool null=false)
            throw(SQLException);
        
        virtual void setDateTime(int pos, time_t time, bool null=false)
            throw(SQLException);
    };
    
    class OCIConnection : public Connection
    {
    friend class OCIStatement;
    private:
        
        static Mutex    connectLock;
        
        const char*     dbInstance;
        const char*     dbUserName;
        const char*     dbUserPassword;
        
        OCIEnv*         envhp;  // OCI envirounment handle
        OCISvcCtx*      svchp;  // OCI service handle
        OCIServer*      srvhp;  // OCI server handle
        OCIError*       errhp;  // OCI error handle
        OCISession*     sesshp; // OCI session handle

    public:

        OCIConnection(const char* instance, 
                      const char* user, const char* password);
        virtual ~OCIConnection();
            
        virtual Statement* createStatement(const char* sql) 
            throw(SQLException);
        
        virtual void connect() 
            throw(SQLException);
        virtual void disconnect();
        
        virtual void commit() 
            throw(SQLException);
        virtual void rollback() 
            throw(SQLException);
        
        void check(sword status) 
            throw(SQLException);
    };

    class OCIDriver : public DBDriver
    {
    private:
        
        char*     dbInstance;
        char*     dbUserName;
        char*     dbUserPassword;

    public:
        
        OCIDriver(ConfigView* config)
            throw (ConfigException);
        virtual ~OCIDriver();

        virtual Connection* newConnection();
    };
    
    class OCIDataSource : public PoolledDataSource
    {
    public:
        
        OCIDataSource() : PoolledDataSource() {};
        virtual ~OCIDataSource() {};

        virtual void init(ConfigView* config)
            throw(ConfigException)
        {
            driver = new OCIDriver(config);
            PoolledDataSource::init(config);
        };
    };
    
    class OCIDataSourceFactory : public DataSourceFactory
    {
    protected:

        virtual DataSource* createDataSource()
        {
            return new OCIDataSource();
        };
        
    public:

        OCIDataSourceFactory() 
            : DataSourceFactory() {};
        virtual ~OCIDataSourceFactory() {};

    };

}}}

#endif
