#ifndef SMSC_DBSME_OCIDATASOURCE
#define SMSC_DBSME_OCIDATASOURCE

/**
 * 
 * @author Victor V. Makarov
 * @version 1.0
 * @see 
 */
 
#include <oci.h>
#include <orl.h>

#include <util/debug.h>

#include <dbsme/DataSource.h>

namespace smsc { namespace dbsme
{
    class OCIStatement;
    class OCIResultSet : public ResultSet
    {
    protected:
        
        OCIStatement* owner;

    public:
        
        OCIResultSet(OCIStatement* statement)
            throw(SQLException);
        virtual ~OCIResultSet();

        virtual bool fetchNext()
            throw(SQLException);
        
        virtual char* getString(int pos)
            throw(SQLException);
        virtual int8_t getInt8(int pos)
            throw(SQLException);
        virtual int16_t getInt16(int pos)
            throw(SQLException);
        virtual int32_t getInt32(int pos)
            throw(SQLException);
        virtual uint8_t getUint8(int pos)
            throw(SQLException);
        virtual uint16_t getUint16(int pos)
            throw(SQLException);
        virtual uint32_t getUint32(int pos)
            throw(SQLException);
        virtual time_t getDateTime(int pos)
            throw(SQLException);
        /* ... */
    };
    
    class OCIConnection;
    class OCIStatement : public Statement
    {
    friend class OCIResultSet;
    protected:

        OCIConnection   *owner;

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
        
        ub4 getRowsAffectedCount();

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
        
        virtual void setString(int pos, char* str)
            throw(SQLException);
        virtual void setInt8(int pos, int8_t val)
            throw(SQLException);
        virtual void setInt16(int pos, int16_t val)
            throw(SQLException);
        virtual void setInt32(int pos, int32_t val)
            throw(SQLException);
        virtual void setUint8(int pos, uint8_t val)
            throw(SQLException);
        virtual void setUint16(int pos, uint16_t val)
            throw(SQLException);
        virtual void setUint32(int pos, uint32_t val)
            throw(SQLException);
        virtual void setDateTime(int pos, time_t time)
            throw(SQLException);
        /* ... */
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
        
        OCIDriver(Manager& config, const char* cat)
            throw (ConfigException);
        virtual ~OCIDriver();

        virtual Connection* newConnection();
    };
    
    class OCIDataSource : public PoolledDataSource
    {
    public:
        
        OCIDataSource() : PoolledDataSource() {};
        virtual ~OCIDataSource() {};

        virtual void init(Manager& config, const char* cat)
            throw(ConfigException)
        {
            driver = new OCIDriver(config, cat);
            PoolledDataSource::init(config, cat);
        };
    };
    
    const char* OCI_FACTORY_IDENTITY = "OCIDataSourceFactory";

    class OCIDataSourceFactory : public DataSourceFactory
    {
    protected:

        OCIDataSourceFactory() 
            : DataSourceFactory(OCI_FACTORY_IDENTITY) {};
        
        virtual ~OCIDataSourceFactory() {};

    public:

        virtual DataSource* createDataSource()
        {
            return new OCIDataSource();
        };
    };

}}

#endif
