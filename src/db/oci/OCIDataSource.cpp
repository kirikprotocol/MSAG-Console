
#include "OCIDataSource.h"

using namespace smsc::db;

extern "C"
DataSourceFactory*  getDataSourceFactory(void)
{
    using namespace smsc::db;
    using namespace smsc::db::oci;

    static OCIDataSourceFactory _ociDataSourceFactory;
    return &_ociDataSourceFactory;
}

namespace smsc { namespace db { namespace oci
{

/* ------------------------ Connection implementation ---------------------- */

Mutex OCIConnection::connectLock;

OCIConnection::OCIConnection(const char* instance, 
                             const char* user, const char* password) 
    : Connection(), 
        dbInstance(instance), dbUserName(user), dbUserPassword(password),
            envhp(0), svchp(0), srvhp(0), errhp(0), sesshp(0)
{
    __require__(dbInstance && dbUserName && dbUserPassword);
    //printf("Connection : %s %s %s\n", dbInstance, dbUserName, dbUserPassword);
}
        
OCIConnection::~OCIConnection() 
{
    disconnect();
};
        
void OCIConnection::connect() 
    throw(SQLException)
{
    if (isConnected && isDead) disconnect();
    
    try
    {
        MutexGuard  guard(connectLock);
        
        if (!isConnected)
        {
            // open connection to DB and begin user session 
            check(OCIEnvCreate(&envhp, OCI_OBJECT|OCI_ENV_NO_MUTEX,
                               (dvoid *)0, 0, 0, 0, (size_t) 0, (dvoid **)0));
            check(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&errhp,
                                 OCI_HTYPE_ERROR, (size_t) 0, (dvoid **)0));
            check(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&srvhp,
                                 OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0)); 
            check(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&svchp,
                                 OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0));
            check(OCIServerAttach(srvhp, errhp, (text *)dbInstance,
                                  strlen(dbInstance), OCI_DEFAULT));
            check(OCIAttrSet((dvoid *)svchp, OCI_HTYPE_SVCCTX,
                             (dvoid *)srvhp, (ub4) 0, OCI_ATTR_SERVER, errhp));
            check(OCIHandleAlloc((dvoid *)envhp, (dvoid **)&sesshp,
                                 OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0));   
            check(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
                             (dvoid *)dbUserName, (ub4)strlen(dbUserName),
                             (ub4) OCI_ATTR_USERNAME, errhp));
            check(OCIAttrSet((dvoid *)sesshp, (ub4) OCI_HTYPE_SESSION,
                             (dvoid *)dbUserPassword, 
                             (ub4) strlen(dbUserPassword),
                             (ub4) OCI_ATTR_PASSWORD, errhp));
            check(OCISessionBegin(svchp, errhp, sesshp, OCI_CRED_RDBMS,
                                  (ub4) OCI_DEFAULT));
            check(OCIAttrSet((dvoid *)svchp, (ub4) OCI_HTYPE_SVCCTX,
                             (dvoid *)sesshp, (ub4) 0,
                             (ub4) OCI_ATTR_SESSION, errhp));
            
            isConnected = true; isDead = false;
        }
    }
    catch (SQLException& exc) 
    {
        disconnect();    
        throw exc;
    }
}

void OCIConnection::disconnect()
{
    MutexGuard  guard(connectLock);
    
    if (isConnected)
    {
        while (statements.Count())
        {
            Statement* statement=0L;
            (void) statements.Pop(statement);
            if (statement) delete statement;
        }
        
        if (errhp && svchp) {
        // logoff from database server
            (void) OCILogoff(svchp, errhp);
        }
        if (envhp) {
        // free envirounment handle, all derrived handles will be freed too
            (void) OCIHandleFree(envhp, OCI_HTYPE_ENV);
        }
        
        isConnected = false; isDead = false;
    }
}

void OCIConnection::commit() 
    throw(SQLException)
{
    check(OCITransCommit(svchp, errhp, OCI_DEFAULT));
}

void OCIConnection::rollback() 
    throw(SQLException)
{
    check(OCITransRollback(svchp, errhp, OCI_DEFAULT));
}

void OCIConnection::check(sword status) 
    throw(SQLException)
{
    text        errbuf[1024];
    sb4         errcode = status;
    
    switch (status)
    {
    case OCI_SUCCESS:
        return;

    case OCI_SUCCESS_WITH_INFO:
        //throw StoreException(status, "OCI_SUCCESS_WITH_INFO");
        return;
    
    case OCI_NO_DATA:
        strcpy((char *)errbuf, "OCI_NODATA");
        break;
        
    case OCI_NEED_DATA:
        strcpy((char *)errbuf, "OCI_NEED_DATA");
        break;
        
    case OCI_STILL_EXECUTING:
        strcpy((char *)errbuf, "OCI_STILL_EXECUTE");
        break;
    
    case OCI_INVALID_HANDLE:
        strcpy((char *)errbuf, "OCI_INVALID_HANDLE");
        break;
                
    case OCI_ERROR:
        if (errhp)
        {
            (void) OCIErrorGet (errhp, (ub4) 1, (text *) NULL,
                                (sb4 *)&errcode, errbuf,
                                (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
        }
        else 
        {
            strcpy((char *)errbuf, "NO_ERROR_DESCRIPTION");
        }
        break;  

    default:
        strcpy((char *)errbuf, "OCI_UNKNOWN_ERROR");
        break;
    }
    
    isDead = true;

    SQLException exc((const char *)errbuf, (int)errcode, (int)status);
    log.error("SQL Exception : %s\n", exc.what());
    throw exc;
}

Statement* OCIConnection::createStatement(const char* sql)
    throw(SQLException)
{
    connect();
    return new OCIStatement(this, sql);
}

/* ---------------- Helper for Binds & Defines implementation -------------- */

OCIDataDescriptor::OCIDataDescriptor(ub2 _type, sb4 _size) 
    : type(_type), size(_size), ind(OCI_IND_NOTNULL)
{
    __trace2__("Type is %d, Size is %d\n", type, (int)size);
    
    switch (type)
    {
    case SQLT_INT: case SQLT_UIN: case SQLT_NUM: case SQLT_FLT:
        data = (dvoid *)&number;
        size = sizeof(OCINumber);
        type = SQLT_VNU;
        break;
    case SQLT_ODT: case SQLT_DAT:
        data = (dvoid *)&date;
        size = sizeof(OCIDate);
        type = SQLT_ODT;
        break;
    case SQLT_CHR: case SQLT_STR:
        data = (dvoid *)(new uint8_t[size = (int)size+1]);
        ((char *)data)[0] = '\0';
        type = SQLT_STR;
        break;
    
    case SQLT_BIN:
    case SQLT_PDN:
    default:
        printf("Type %d is not supported.\n", type);
    }
}

OCIDataDescriptor::~OCIDataDescriptor()
{
    if (data && (type == SQLT_STR || type == SQLT_CHR))
    {
        delete ((uint8_t *)data);
    }
}

/* ------------------------ Statement implementation ----------------------- */

OCIStatement::OCIStatement(OCIConnection* connection, const char* sql) 
    throw(SQLException)
        : Statement(), owner(connection)
{
    __require__(owner);
    
    envhp = owner->envhp; svchp = owner->svchp; errhp = owner->errhp;
    
    check(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmt,
                         OCI_HTYPE_STMT, 0, (dvoid **)0));
    check(OCIStmtPrepare(stmt, errhp, (text *)sql, (ub4) strlen(sql),
                         (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
}

OCIStatement::~OCIStatement()
{
    while (descriptors.Count())
    {
        OCIDataDescriptor* descriptor=0L;
        (void) descriptors.Pop(descriptor);
        if (descriptor) delete descriptor;
    }
    (void) OCIHandleFree(stmt, OCI_HTYPE_STMT);
}

void OCIStatement::bind(ub4 pos, ub2 type, 
                        dvoid* placeholder, sb4 size, dvoid* indp)
    throw(SQLException)
{
    OCIBind *bind;
    check(OCIBindByPos(stmt, &bind, errhp, pos, 
                       placeholder, size, type, indp,
                       (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                       (ub4) OCI_DEFAULT));
}

void OCIStatement::bind(CONST text* name, sb4 name_len, ub2 type,
                        dvoid* placeholder, sb4 size, dvoid* indp)
    throw(SQLException)
{
    OCIBind *bind;
    check(OCIBindByName(stmt, &bind, errhp, name, name_len,
                        placeholder, size, type, indp,
                        (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                        (ub4) OCI_DEFAULT));
}

void OCIStatement::define(ub4 pos, ub2 type, 
                          dvoid* placeholder, sb4 size, dvoid* indp)
    throw(SQLException)
{
    OCIDefine*  define;
    check(OCIDefineByPos(stmt, &define, errhp, pos, 
                         placeholder, size, type, indp,
                         (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT));
}
void OCIStatement::convertDateToOCIDate(time_t* sms_date, OCIDate* oci_date)
{
    tm dt; localtime_r(sms_date, &dt);

    OCIDateSetDate(oci_date, (sb2)(1900+dt.tm_year), 
                   (ub1)(1+dt.tm_mon), (ub1)(dt.tm_mday));
    OCIDateSetTime(oci_date, (ub1)(dt.tm_hour), 
                   (ub1)(dt.tm_min), (ub1)(dt.tm_sec));
}

void OCIStatement::convertOCIDateToDate(OCIDate* oci_date, time_t* sms_date)
{
    tm  dt;
    sb2 year;
    ub1 mon, mday, hour, min, sec;

    OCIDateGetTime(oci_date, (ub1 *) &hour, (ub1 *) &min, (ub1 *) &sec);
    OCIDateGetDate(oci_date, (sb2 *) &year, (ub1 *) &mon, (ub1 *) &mday);
    dt.tm_year = year - 1900; dt.tm_mon = mon - 1; dt.tm_mday = mday;
    dt.tm_hour = hour; dt.tm_min = min; dt.tm_sec = sec;
    *sms_date = mktime(&dt);
}

ub4 OCIStatement::getRowsAffectedCount()
{
    ub4 res = 0; 
    if (OCIAttrGet((CONST dvoid *)stmt, OCI_HTYPE_STMT, 
                   &res, NULL, OCI_ATTR_ROW_COUNT, errhp) != OCI_SUCCESS)
    {
        return 0;
    }
    return res;
}

void OCIStatement::check(sword status) 
    throw(SQLException) 
{
    __require__(owner);
    owner->check(status);
}
        
sword OCIStatement::execute(ub4 mode, ub4 iters, ub4 rowoff)
    throw(SQLException)
{
    return OCIStmtExecute(svchp, stmt, errhp, iters, rowoff,
                          (CONST OCISnapshot *) NULL, 
                          (OCISnapshot *) NULL, mode);
}

void OCIStatement::execute() 
    throw(SQLException)
{
    check(execute(OCI_DEFAULT, 1, 0));
}

sword OCIStatement::fetch()
    throw(SQLException)
{
    return OCIStmtFetch(stmt, errhp, (ub4) 1, (ub4) OCI_FETCH_NEXT,
                        (ub4) OCI_DEFAULT);
}

uint32_t OCIStatement::executeUpdate() 
    throw(SQLException)
{
    execute();
    return (uint32_t)getRowsAffectedCount();
}
            
ResultSet* OCIStatement::executeQuery() 
    throw(SQLException)
{
    return new OCIResultSet(this);
}

OCIDataDescriptor* OCIStatement::setField(int pos, ub2 type, ub4 size, 
                                          bool null)
{
    OCIDataDescriptor* descriptor = 0;
    if (descriptors.Count() < pos)
    {
        while (descriptors.Count() < pos-1)
        {
            descriptors.Push((OCIDataDescriptor *)0);
        }
        descriptor = new OCIDataDescriptor(type, size);
        descriptors.Push(descriptor);
    }
    else
    {
        descriptor = descriptors[pos-1];
        if (descriptor) delete descriptor;
        descriptor = new OCIDataDescriptor(type, size);
        descriptors[pos-1] = descriptor;
    }
    descriptor->ind = (null) ? OCI_IND_NULL : OCI_IND_NOTNULL;
    return descriptor;
}

void OCIStatement::setString(int pos, const char* str, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_CHR, (str) ? strlen(str):0, null);
    
    if (!null) 
    {
        strcpy((char *)descriptor->data, str);
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setInt8(int pos, int8_t val, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_INT, sizeof(int8_t), null);
    
    if (!null)
    {
        check(OCINumberFromInt(errhp, (CONST dvoid *)&val,
                               (uword)sizeof(int8_t),
                               (uword)OCI_NUMBER_SIGNED,
                               (OCINumber *)&(descriptor->number)));
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setInt16(int pos, int16_t val, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_INT, sizeof(int16_t), null);
    
    if (!null)
    {
        check(OCINumberFromInt(errhp, (CONST dvoid *)&val,
                               (uword)sizeof(int16_t),
                               (uword)OCI_NUMBER_SIGNED,
                               (OCINumber *)&(descriptor->number)));
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setInt32(int pos, int32_t val, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_INT, sizeof(int32_t), null);
    
    if (!null)
    {
        check(OCINumberFromInt(errhp, (CONST dvoid *)&val,
                               (uword)sizeof(int32_t),
                               (uword)OCI_NUMBER_SIGNED,
                               (OCINumber *)&(descriptor->number)));
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setUint8(int pos, uint8_t val, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_UIN, sizeof(uint8_t), null);
    
    if (!null)
    {
        check(OCINumberFromInt(errhp, (CONST dvoid *)&val,
                               (uword)sizeof(uint8_t),
                               (uword)OCI_NUMBER_UNSIGNED,
                               (OCINumber *)&(descriptor->number)));
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setUint16(int pos, uint16_t val, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_UIN, sizeof(uint16_t), null);
    
    if (!null)
    {
        check(OCINumberFromInt(errhp, (CONST dvoid *)&val,
                               (uword)sizeof(uint16_t),
                               (uword)OCI_NUMBER_UNSIGNED,
                               (OCINumber *)&(descriptor->number)));
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setUint32(int pos, uint32_t val, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_UIN, sizeof(uint32_t), null);
    
    if (!null)
    {
        check(OCINumberFromInt(errhp, (CONST dvoid *)&val,
                               (uword)sizeof(uint32_t),
                               (uword)OCI_NUMBER_UNSIGNED,
                               (OCINumber *)&(descriptor->number)));
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setFloat(int pos, float val, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_FLT, sizeof(float), null);
    
    if (!null)
    {
        check(OCINumberFromReal(errhp, (CONST dvoid *)&val,
                               (uword)sizeof(float),
                               (OCINumber *)&(descriptor->number)));
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setDouble(int pos, double val, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_FLT, sizeof(double), null);
    
    if (!null)
    {
        check(OCINumberFromReal(errhp, (CONST dvoid *)&val,
                               (uword)sizeof(double),
                               (OCINumber *)&(descriptor->number)));
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setLongDouble(int pos, long double val, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_FLT, sizeof(long double), null);
    
    if (!null)
    {
        check(OCINumberFromReal(errhp, (CONST dvoid *)&val,
                               (uword)sizeof(long double),
                               (OCINumber *)&(descriptor->number)));
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
void OCIStatement::setDateTime(int pos, time_t time, bool null)
    throw(SQLException)
{
    OCIDataDescriptor* descriptor = 
        setField(pos, SQLT_ODT, sizeof(OCIDate), null);
    
    if (!null)
    {
        convertDateToOCIDate(&time, (OCIDate *)descriptor->data);
    }
    bind(pos, descriptor->type, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}

/* ------------------------ ResultSet implementation ----------------------- */

OCIResultSet::OCIResultSet(OCIStatement* statement)
    throw(SQLException)
        : ResultSet(), owner(statement)
{
    __require__(owner);
    
    owner->check(owner->execute(OCI_DEFAULT, 0, 0));
    // retrive describe info and process the set of defines here !
    
    OCIParam *param = 0;
    ub4 counter = 1;
    
    /* Request a parameter descriptor for position 1 in the select-list */
    sb4 status = OCIParamGet(owner->stmt, OCI_HTYPE_STMT, 
                             owner->errhp, (dvoid **)&param, (ub4) counter);
    /* Loop only if a descriptor was successfully retrieved for
    current position, starting at 1 */
    while (status==OCI_SUCCESS) 
    {
        ub2 type = 0; ub4 size = 0;

        /* Retrieve the data type attribute */
        owner->check(OCIAttrGet((dvoid *) param, (ub4) OCI_DTYPE_PARAM,
                                (dvoid *) &type, (ub4 *) 0, 
                                (ub4) OCI_ATTR_DATA_TYPE,
                                (OCIError*) owner->errhp));
        /* Retrieve max data size */
        owner->check(OCIAttrGet((dvoid *) param, (ub4) OCI_DTYPE_PARAM,
                                (dvoid *) &size, (ub4 *) 0,
                                (ub4) OCI_ATTR_DATA_SIZE,
                                (OCIError *) owner->errhp));
        
        OCIDataDescriptor* descriptor = new OCIDataDescriptor(type, size);
        owner->define(counter, descriptor->type, descriptor->data,
                      descriptor->size, (dvoid *) &descriptor->ind);
        descriptors.Push(descriptor);
        
        owner->check(OCIDescriptorFree((dvoid *) param, OCI_DTYPE_PARAM));
        /* increment counter and get next descriptor, if there is one */
        status = OCIParamGet(owner->stmt, OCI_HTYPE_STMT, 
                             owner->errhp, (dvoid **)&param, (ub4) ++counter);
    }
}

OCIResultSet::~OCIResultSet()
{
    while (descriptors.Count())
    {
        OCIDataDescriptor* descriptor=0L;
        (void) descriptors.Pop(descriptor);
        if (descriptor) delete descriptor;
    }
}

bool OCIResultSet::fetchNext()
    throw(SQLException)
{
    __require__(owner);

    sword status = owner->fetch();
    if (status != OCI_NO_DATA)
    {
        owner->check(status);
        return true;
    }
    return false;
}

dvoid* OCIResultSet::getField(int pos)
    throw (InvalidArgumentException)
{
    OCIDataDescriptor* descriptor = descriptors[pos-1];
    if (descriptor)
    {
        return descriptor->data;
    }
    throw InvalidArgumentException();
}

bool OCIResultSet::isNull(int pos)
    throw (SQLException, InvalidArgumentException)
{
    OCIDataDescriptor* descriptor = descriptors[pos-1];
    if (descriptor)
    {
        return ((descriptor->ind == OCI_IND_NOTNULL) ? false : true);
    }
    throw InvalidArgumentException();
}

const char* OCIResultSet::getString(int pos)
    throw(SQLException, InvalidArgumentException)
{
    return (const char*)getField(pos);
}

int8_t OCIResultSet::getInt8(int pos)
    throw(SQLException, InvalidArgumentException)
{
    return (int8_t)getInt32(pos);
}
int16_t OCIResultSet::getInt16(int pos)
    throw(SQLException, InvalidArgumentException)
{
    return (int16_t)getInt32(pos);
}
int32_t OCIResultSet::getInt32(int pos)
    throw(SQLException, InvalidArgumentException)
{
    __require__(owner);

    boolean     ok = FALSE;
    int32_t     result = 0;
    
    OCINumber*  number = (OCINumber*)getField(pos);
    owner->check(OCINumberIsInt(owner->errhp, (CONST OCINumber *)number, &ok));
    if (ok != TRUE) 
    {
        throw InvalidArgumentException();
    }
    owner->check(OCINumberToInt(owner->errhp, (CONST OCINumber *)number,
                                (uword) sizeof(int32_t), 
                                (uword)OCI_NUMBER_SIGNED,
                                (dvoid *) &result)); 
    return result;
}
uint8_t OCIResultSet::getUint8(int pos)
    throw(SQLException, InvalidArgumentException)
{
    return (uint8_t)getUint32(pos);
}
uint16_t OCIResultSet::getUint16(int pos)
    throw(SQLException, InvalidArgumentException)
{
    return (uint16_t)getUint32(pos);
}
uint32_t OCIResultSet::getUint32(int pos)
    throw(SQLException, InvalidArgumentException)
{
    __require__(owner);

    boolean     ok = FALSE;
    uint32_t    result = 0;
    
    OCINumber*  number = (OCINumber*)getField(pos);
    owner->check(OCINumberIsInt(owner->errhp, (CONST OCINumber *)number, &ok));
    if (ok != TRUE) 
    {
        throw InvalidArgumentException();
    }
    owner->check(OCINumberToInt(owner->errhp, (CONST OCINumber *)number,
                                (uword) sizeof(uint32_t),
                                (uword)OCI_NUMBER_UNSIGNED,
                                (dvoid *) &result));    
    return result;
}
float OCIResultSet::getFloat(int pos)
    throw(SQLException, InvalidArgumentException)
{
    __require__(owner);

    boolean     ok = FALSE;
    float       result = 0;
    
    OCINumber*  number = (OCINumber*)getField(pos);
    owner->check(OCINumberToReal(owner->errhp, (CONST OCINumber *)number,
                                 (uword) sizeof(float),
                                 (dvoid *) &result));    
    return result;
}
double OCIResultSet::getDouble(int pos)
    throw(SQLException, InvalidArgumentException)
{
    boolean     ok = FALSE;
    double      result = 0;
    
    OCINumber*  number = (OCINumber*)getField(pos);
    owner->check(OCINumberToReal(owner->errhp, (CONST OCINumber *)number,
                                 (uword) sizeof(double),
                                 (dvoid *) &result));    
    return result;
}
long double OCIResultSet::getLongDouble(int pos)
    throw(SQLException, InvalidArgumentException)
{
    boolean     ok = FALSE;
    long double result = 0;
    
    OCINumber*  number = (OCINumber*)getField(pos);
    owner->check(OCINumberToReal(owner->errhp, (CONST OCINumber *)number,
                                 (uword) sizeof(long double),
                                 (dvoid *) &result));    
    return result;
}

time_t OCIResultSet::getDateTime(int pos)
    throw(SQLException, InvalidArgumentException)
{
    time_t sys_date;
    OCIDate* oci_date = (OCIDate *)getField(pos);
    owner->convertOCIDateToDate(oci_date, &sys_date);
    return sys_date;
}

/* -------------------------- Driver implementation ------------------------ */

OCIDriver::OCIDriver(ConfigView* config)
    throw (ConfigException)
        : DBDriver(config),
            dbInstance(0), dbUserName(0), dbUserPassword(0)
{
    __require__(config);

    dbInstance = config->getString("dbInstance",
                                   "DB instance name wasn't specified !");
    dbUserName = config->getString("dbUserName",
                                   "DB user name wasn't specified !");
    dbUserPassword = config->getString("dbUserPassword", 
                                    "DB user password wasn't specified !");
    //printf("Driver : %s %s %s\n", dbInstance, dbUserName, dbUserPassword);
}

OCIDriver::~OCIDriver() 
{
    if (dbInstance) delete dbInstance;
    if (dbUserName) delete dbUserName;
    if (dbUserPassword) delete dbUserPassword;
}

Connection* OCIDriver::newConnection() 
{
    return new OCIConnection(dbInstance, dbUserName, dbUserPassword);
}

}}}

