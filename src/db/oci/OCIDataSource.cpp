
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

const int FUNCTION_MAX_ARGUMENTS_COUNT  = 100;
const char* FUNCTION_RETURN_ATTR_NAME   = "RETURN";
const int FUNCTION_RETURN_ATTR_LEN      = strlen(FUNCTION_RETURN_ATTR_NAME);
const int MAX_DB_CHAR_STR_LENGTH        = 2000;

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
        throw;
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

Routine* OCIConnection::createRoutine(const char* call, bool func)
    throw(SQLException)
{
    connect();
    
    std::string routine; std::string name = "";
    routine += "BEGIN\n"; 
    if (func) 
    {
        routine += ":"; routine += FUNCTION_RETURN_ATTR_NAME; routine += " := ";
    }
    routine += call; 
    routine += "\nEND;";
    
    int curPos = 0;
    while (call && isspace(call[curPos])) curPos++;
    while (call && isalnum(call[curPos])) name += call[curPos++];
    
    __trace2__("Call:\n%s", routine.c_str());
    return new OCIRoutine(this, routine.c_str(), name.c_str(), func);
}

/* ---------------- Helper for Binds & Defines implementation -------------- */

OCIDataDescriptor::OCIDataDescriptor(ub2 _type, sb4 _size) 
    : type(_type), size(_size), ind(OCI_IND_NOTNULL)
{
    __trace2__("Type is %d, Size is %d", type, _size);
    
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
    case SQLT_CHR: case SQLT_STR: case SQLT_VST:
        if (_size == 0) _size = MAX_DB_CHAR_STR_LENGTH;
        data = (dvoid *)(new uint8_t[size = _size+1]);
        ((char *)data)[0] = '\0';
        type = SQLT_STR;
        break;
    
    case SQLT_BIN:
    case SQLT_PDN:
    default:
        __trace2__("Type %d is not supported.\n", type);
    }
}

OCIDataDescriptor::~OCIDataDescriptor()
{
    if (data && (type == SQLT_STR || type == SQLT_CHR))
    {
        delete ((uint8_t *)data);
    }
}

/* ------------------------ Query implementation ----------------------- */

OCIQuery::OCIQuery(OCIConnection* connection, const char* query) 
    throw(SQLException)
        : owner(connection)
{
    __require__(owner && query);

    ub4 querylen = strlen(query);
    sqlquery = new char[querylen+1];
    strcpy(sqlquery, query);
    
    envhp = owner->envhp; svchp = owner->svchp; errhp = owner->errhp;
    check(OCIHandleAlloc((dvoid *)envhp, (dvoid **) &stmt,
                         OCI_HTYPE_STMT, 0, (dvoid **)0));
    check(OCIStmtPrepare(stmt, errhp, (text *)sqlquery, (ub4) querylen,
                         (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));
}
OCIQuery::~OCIQuery()
{
    (void) OCIHandleFree(stmt, OCI_HTYPE_STMT);
    if (sqlquery) delete sqlquery;
}

void OCIQuery::bind(ub4 pos, ub2 type, 
                        dvoid* placeholder, sb4 size, dvoid* indp)
    throw(SQLException)
{
    __trace2__("Bind by pos: %d, type %d", pos, type);
    OCIBind *bind = 0;
    check(OCIBindByPos(stmt, &bind, errhp, pos, 
                       placeholder, size, type, indp,
                       (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                       (ub4) OCI_DEFAULT));
}

void OCIQuery::bind(CONST text* name, sb4 name_len, ub2 type,
                        dvoid* placeholder, sb4 size, dvoid* indp)
    throw(SQLException)
{
    __trace2__("Bind by name: %s, type %d", name, type);
    OCIBind *bind = 0;
    check(OCIBindByName(stmt, &bind, errhp, name, name_len,
                        placeholder, size, type, indp,
                        (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0,
                        (ub4) OCI_DEFAULT));
}

void OCIQuery::define(ub4 pos, ub2 type, 
                          dvoid* placeholder, sb4 size, dvoid* indp)
    throw(SQLException)
{
    OCIDefine*  define = 0;
    check(OCIDefineByPos(stmt, &define, errhp, pos, 
                         placeholder, size, type, indp,
                         (ub2 *) 0, (ub2 *) 0, OCI_DEFAULT));
}

void OCIQuery::convertDateToOCIDate(time_t* sms_date, OCIDate* oci_date)
{
    tm dt; gmtime_r(sms_date, &dt);

    OCIDateSetDate(oci_date, (sb2)(1900+dt.tm_year), 
                   (ub1)(1+dt.tm_mon), (ub1)(dt.tm_mday));
    OCIDateSetTime(oci_date, (ub1)(dt.tm_hour), 
                   (ub1)(dt.tm_min), (ub1)(dt.tm_sec));
}

void OCIQuery::convertOCIDateToDate(OCIDate* oci_date, time_t* sms_date)
{
    sb2 year;
    ub1 mon, mday, hour, min, sec;

    OCIDateGetTime(oci_date, (ub1 *) &hour, (ub1 *) &min, (ub1 *) &sec);
    OCIDateGetDate(oci_date, (sb2 *) &year, (ub1 *) &mon, (ub1 *) &mday);

    tm  dt; dt.tm_isdst = 0;
    dt.tm_year = year - 1900; dt.tm_mon = mon - 1; dt.tm_mday = mday;
    dt.tm_hour = hour; dt.tm_min = min; dt.tm_sec = sec;
    *sms_date = mktime(&dt) - timezone;
}

void OCIQuery::check(sword status) 
    throw(SQLException) 
{
    __require__(owner);
    owner->check(status);
}
        
sword OCIQuery::execute(ub4 mode, ub4 iters, ub4 rowoff)
    throw(SQLException)
{
    return OCIStmtExecute(svchp, stmt, errhp, iters, rowoff,
                          (CONST OCISnapshot *) NULL, 
                          (OCISnapshot *) NULL, mode);
}

sword OCIQuery::fetch()
    throw(SQLException)
{
    return OCIStmtFetch(stmt, errhp, (ub4) 1, (ub4) OCI_FETCH_NEXT,
                        (ub4) OCI_DEFAULT);
}


/* ------------------------ Statement implementation ----------------------- */

OCIStatement::OCIStatement(OCIConnection* connection, const char* sql) 
    throw(SQLException)
        : Statement(), OCIQuery(connection, sql)
{
}

OCIStatement::~OCIStatement()
{
    while (descriptors.Count())
    {
        OCIDataDescriptor* descriptor=0L;
        (void) descriptors.Pop(descriptor);
        if (descriptor) delete descriptor;
    }
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

void OCIStatement::execute() 
    throw(SQLException)
{
    check(OCIQuery::execute(OCI_DEFAULT, 1, 0));
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
void OCIStatement::setInt64(int pos, int64_t val, bool null)
    throw(SQLException)
{
    setInt32(pos, (int32_t)val, null);
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
void OCIStatement::setUint64(int pos, uint64_t val, bool null)
    throw(SQLException)
{
    setUint32(pos, (uint32_t)val, null);
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
int64_t OCIResultSet::getInt64(int pos)
    throw(SQLException, InvalidArgumentException)
{
    return (int64_t)getInt32(pos);
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
uint64_t OCIResultSet::getUint64(int pos)
    throw(SQLException, InvalidArgumentException)
{
    return (uint64_t)getUint32(pos);
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

/* ------------------------ OCIRoutine implementation ---------------------- */
        
OCIRoutine::OCIRoutine(OCIConnection* connection, 
                       const char* call, const char* name, bool func) 
    throw(SQLException)
        : Routine(), OCIQuery(connection, call)
{
    // Loadup routine parameters set here.

    ub4      bndSize  = FUNCTION_MAX_ARGUMENTS_COUNT;
    sb4      bndFound = 0 ;
    sb4      bndCurrent = 0;
    text*    bndBvnp[bndSize];
    ub1      bndBvnl[bndSize];
    text*    bndInvp[bndSize];
    ub1      bndInpl[bndSize];
    ub1      bndDupl[bndSize];
    OCIBind* bndHndl[bndSize];

    check(OCIStmtGetBindInfo (stmt, errhp, bndSize, 1, &bndFound,
                              bndBvnp, bndBvnl, bndInvp, bndInpl, bndDupl,
                              bndHndl));
    if (bndFound < 0) throw SQLException("Too many bind variables "
                                         "in PL/SQL call! Max allowed is %d.", 
                                         FUNCTION_MAX_ARGUMENTS_COUNT);
    for (int x=0; x<bndFound; x++)
    {
        std::string bndstr(bndBvnp[x] ? (const char *)bndBvnp[x]:
                            "", bndBvnp[x] ? bndBvnl[x]:0);
        __trace2__("#%02d Bind variable '%s'", x+1, bndstr.c_str());
    }
    
    OCIParam    *parmh  = 0;    // parameter handle 
    OCIParam    *arglst = 0;    // list of args
    OCIParam    *arg    = 0;    // argument handle
    OCIDescribe *dschp  = 0;    // describe handle
    
    ub2         numargs, pos, level;
    /* allocate describe handle for routine */
    check(OCIHandleAlloc((dvoid *) envhp, (dvoid **) &dschp,
                         (ub4) OCI_HTYPE_DESCRIBE,
                         (size_t) 0, (dvoid **) 0));                     
    try
    {
        /* get the describe handle for routine */
        check(OCIDescribeAny(svchp, errhp, (text *)name, (ub4)strlen(name),
                             OCI_OTYPE_NAME, 0, 
                             (func) ? OCI_PTYPE_FUNC : OCI_PTYPE_PROC, dschp));
        /* get the parameter handle */
        check(OCIAttrGet(dschp, OCI_HTYPE_DESCRIBE, &parmh, 
                         0, OCI_ATTR_PARAM, errhp));
        /* get the number of arguments and the arglist */
        check(OCIAttrGet(parmh, OCI_DTYPE_PARAM, &arglst,
                         0, OCI_ATTR_LIST_ARGUMENTS, errhp));
        check(OCIAttrGet(arglst, OCI_DTYPE_PARAM, &numargs, 
                         0, OCI_ATTR_NUM_PARAMS, errhp));
        
        __trace2__("Num-args is %d", numargs);

        // For procedure, begin with 1 for function, begin with 0. 
        ub1 shift = ((func) ? 0:1);
        for (ub4 i=shift; i<numargs+shift; i++) 
        {
            text*       atr; 
            ub4         atrlen;
            OCIParam   *innerlst;   

            check(OCIParamGet(arglst, OCI_DTYPE_PARAM, errhp, (void **)&arg, i));

            try
            {
                ub2 type = 0; ub4 size = 0;
                if (i != 0) 
                {
                    check(OCIAttrGet(arg, OCI_DTYPE_PARAM, &atr, &atrlen, 
                                     OCI_ATTR_NAME, errhp)); 
                }
                check(OCIAttrGet(arg, OCI_DTYPE_PARAM, &type, 0, 
                                 OCI_ATTR_DATA_TYPE, errhp));
                check(OCIAttrGet((dvoid *)arg, (ub4)OCI_DTYPE_PARAM, 
                                 (dvoid *)&size, (ub4 *)0,
                                 (ub4)OCI_ATTR_DATA_SIZE, errhp));
                
                check(OCIAttrGet(arg, OCI_DTYPE_PARAM, &innerlst, 0,
                                 OCI_ATTR_LIST_ARGUMENTS, errhp));
                if (innerlst) // check if the current argument is a record.
                    throw SQLException("RecordSet type is not supported for use "
                                       "in context of PL/SQL calls!");

                std::string argstr(atr ? (const char *)atr:"", atr ? atrlen:0);
                __trace2__("Arg #%d: '%s'", i, argstr.c_str());

                if (i == 0)
                {
                    OCIDataDescriptor* descriptor = new OCIDataDescriptor(type, size);
                    descriptors.Insert(FUNCTION_RETURN_ATTR_NAME, descriptor);
                    descriptor->ind = OCI_IND_NULL;
                    bind((ub4) 1, descriptor->type, descriptor->data,
                         descriptor->size, (dvoid *) &descriptor->ind);
                    ++bndCurrent;
                }
                else if (bndCurrent < bndFound && 
                        isTextsEqual(bndBvnp[bndCurrent], bndBvnl[bndCurrent],
                                     atr, atrlen))
                {
                    OCIDataDescriptor* descriptor = new OCIDataDescriptor(type, size);
                    descriptors.Insert(argstr.c_str(), descriptor);
                    descriptor->ind = OCI_IND_NULL;
                    bind(atr, atrlen, descriptor->type, 
                         descriptor->data, descriptor->size, (dvoid *) &descriptor->ind);
                    ++bndCurrent;
                }
                else
                {
                    __trace2__("Arg #%d: '%s' is unbound !", i, argstr.c_str());
                }
            }
            catch (SQLException& exc)
            {
                if (arg) check(OCIDescriptorFree(arg, OCI_DTYPE_PARAM));
                throw; 
            }
            
            check(OCIDescriptorFree(arg, OCI_DTYPE_PARAM)); arg = 0;
        }
    }
    catch (SQLException& exc)
    {
        if (dschp)
            check(OCIHandleFree((dvoid *) dschp, (ub4) OCI_HTYPE_DESCRIBE));
        cleanupDescriptors();
        throw; 
    }
}

bool OCIRoutine::isTextsEqual(text* txt1, ub4 len1, text* txt2, ub4 len2)
{
    if (len1 != len2 || (!txt1 && txt2) || (!txt2 && txt1)) return false;
    return (txt1 && txt2) ? 
            (strncmp((const char *)txt1, (const char *)txt2, len1)==0) : true;
}

void OCIRoutine::cleanupDescriptors()
{
    // Clean up descriptors hash here.

    descriptors.First();
    char* key; OCIDataDescriptor* descriptor = 0;
    while (descriptors.Next(key, descriptor))
        if (descriptor) delete descriptor;
}

OCIRoutine::~OCIRoutine() 
{
    cleanupDescriptors();
}

char* convertStrToUpperCase(const char* str)
{
    if (!str) return 0;
    char* up = new char[strlen(str)+1];
    int cur = 0;
    while (*str) up[cur++] = (char)toupper(*str++);
    up[cur] = '\0';
    return up;
}
OCIDataDescriptor* OCIRoutine::findDescriptor(const char* key)
    throw(InvalidArgumentException)
{
    char* up = convertStrToUpperCase(key);

    OCIDataDescriptor* descriptor = 0;
    if (!descriptors.Exists(up) || 
        !(descriptor = descriptors.Get(up))) 
    { 
        if (up) delete up;
        throw InvalidArgumentException();
    }
            
    if (up) delete up;
    return descriptor;
}
dvoid* OCIRoutine::getField(const char* key)
    throw (InvalidArgumentException)
{
    OCIDataDescriptor* descriptor = findDescriptor(key);
    return descriptor->data;
}

void OCIRoutine::execute() 
    throw(SQLException)
{
    __trace__("Executing ...");
    check(OCIQuery::execute(OCI_DEFAULT, 1, 0));
    __trace__("Execute ok");
}
bool OCIRoutine::isNull(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    OCIDataDescriptor* descriptor = findDescriptor(key);
    return ((descriptor->ind == OCI_IND_NOTNULL) ? false : true);
}
void OCIRoutine::setString(const char* key, const char* str, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    OCIDataDescriptor* descriptor = findDescriptor(key);
    if (descriptor->type != SQLT_STR)
        throw InvalidArgumentException();
    
    if (!null) 
    {
        if (str)
        {
            if (!descriptor->size)
            {
                if (descriptor->data) delete ((uint8_t *)descriptor->data);
                descriptor->size = strlen(str)+1;
                descriptor->data = new uint8_t[descriptor->size];
            }
            strncpy((char *)descriptor->data, str, descriptor->size-1);
        }
        descriptor->ind = (str) ? OCI_IND_NOTNULL : OCI_IND_NULL;
    }
    else descriptor->ind = OCI_IND_NULL;

    __trace2__("setString > Arg '%s': size: %d, value '%s'", 
                key, descriptor->size, (char *)descriptor->data);

    bind((text *)key, strlen(key), SQLT_STR, descriptor->data,
         descriptor->size, (dvoid *) &descriptor->ind);
}
const char* OCIRoutine::getString(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    return (const char*)getField(key);
}

#define defineSetInt(key, val, sign, null)                              \
{                                                                       \
    OCIDataDescriptor* descriptor = findDescriptor(key);                \
    if (descriptor->type != SQLT_VNU)                                   \
        throw InvalidArgumentException();                               \
    if (!null)                                                          \
    {                                                                   \
        check(OCINumberFromInt(errhp, (CONST dvoid *)&val,              \
                               (uword)sizeof(val),                      \
                               (uword)((sign) ?                         \
                               OCI_NUMBER_SIGNED:OCI_NUMBER_UNSIGNED),  \
                               (OCINumber *)&(descriptor->number)));    \
        descriptor->ind = OCI_IND_NOTNULL;                              \
    }                                                                   \
    else descriptor->ind = OCI_IND_NULL;                                \
    __trace2__("setInt > Arg '%s': size: %d",                           \
                key, descriptor->size);                                 \
    bind((text *)key, strlen(key), descriptor->type, descriptor->data,  \
         descriptor->size, (dvoid *) &descriptor->ind);                 \
}

void OCIRoutine::setInt8(const char* key, int8_t val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    defineSetInt(key, val, true, null);
}
int8_t OCIRoutine::getInt8(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    return (int8_t)getInt32(key);
}
void OCIRoutine::setInt16(const char* key, int16_t val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    defineSetInt(key, val, true, null);
}
int16_t OCIRoutine::getInt16(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    return (int16_t)getInt32(key);
}
void OCIRoutine::setInt32(const char* key, int32_t val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    defineSetInt(key, val, true, null);
}
int32_t OCIRoutine::getInt32(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    boolean     ok = FALSE;
    int32_t     result = 0;
    
    OCINumber*  number = (OCINumber*)getField(key);
    check(OCINumberIsInt(errhp, (CONST OCINumber *)number, &ok));
    if (ok != TRUE) throw InvalidArgumentException();
    check(OCINumberToInt(errhp, (CONST OCINumber *)number,
                         (uword) sizeof(int32_t), 
                         (uword)OCI_NUMBER_SIGNED,
                         (dvoid *) &result)); 
    return result;
}
void OCIRoutine::setInt64(const char* key, int64_t val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    int32_t value = (int32_t)val;
    defineSetInt(key, value, true, null);
}
int64_t OCIRoutine::getInt64(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    return (int64_t)getInt32(key);
}
void OCIRoutine::setUint8(const char* key, uint8_t val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    defineSetInt(key, val, false, null);
}
uint8_t OCIRoutine::getUint8(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    return (uint8_t)getUint32(key);
}
void OCIRoutine::setUint16(const char* key, uint16_t val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    defineSetInt(key, val, false, null);
}
uint16_t OCIRoutine::getUint16(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    return (uint16_t)getUint32(key);
}
void OCIRoutine::setUint32(const char* key, uint32_t val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    defineSetInt(key, val, false, null);
}
uint32_t OCIRoutine::getUint32(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    boolean     ok = FALSE;
    uint32_t    result = 0;
    
    OCINumber*  number = (OCINumber*)getField(key);
    check(OCINumberIsInt(errhp, (CONST OCINumber *)number, &ok));
    if (ok != TRUE) throw InvalidArgumentException();
    check(OCINumberToInt(errhp, (CONST OCINumber *)number,
                         (uword) sizeof(uint32_t),
                         (uword)OCI_NUMBER_UNSIGNED,
                         (dvoid *) &result));    
    return result;
}
void OCIRoutine::setUint64(const char* key, uint64_t val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    uint32_t value = (uint32_t)val;
    defineSetInt(key, value, false, null);
}
uint64_t OCIRoutine::getUint64(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    return (uint64_t)getUint32(key);
}
    
#define defineSetFloat(key, val, null)                                  \
{                                                                       \
    OCIDataDescriptor* descriptor = findDescriptor(key);                \
    if (descriptor->type != SQLT_FLT)                                   \
        throw InvalidArgumentException();                               \
    if (!null)                                                          \
    {                                                                   \
        check(OCINumberFromReal(errhp, (CONST dvoid *)&val,             \
                               (uword)sizeof(val),                      \
                               (OCINumber *)&(descriptor->number)));    \
        descriptor->ind = OCI_IND_NOTNULL;                              \
    }                                                                   \
    else descriptor->ind = OCI_IND_NULL;                                \
    bind((text *)key, strlen(key), descriptor->type, descriptor->data,  \
         descriptor->size, (dvoid *) &descriptor->ind);                 \
}

void OCIRoutine::setFloat(const char* key, float val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    defineSetFloat(key, val, null);
}
float OCIRoutine::getFloat(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    boolean     ok = FALSE;
    float       result = 0;
    
    OCINumber*  number = (OCINumber*)getField(key);
    check(OCINumberToReal(errhp, (CONST OCINumber *)number,
                          (uword) sizeof(float),
                          (dvoid *) &result));    
    return result;
}
void OCIRoutine::setDouble(const char* key, double val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    defineSetFloat(key, val, null);
}
double OCIRoutine::getDouble(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    boolean     ok = FALSE;
    double      result = 0;
    
    OCINumber*  number = (OCINumber*)getField(key);
    check(OCINumberToReal(errhp, (CONST OCINumber *)number,
                          (uword) sizeof(double),
                          (dvoid *) &result));    
    return result;
}
void OCIRoutine::setLongDouble(const char* key, long double val, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    defineSetFloat(key, val, null);
}
long double OCIRoutine::getLongDouble(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    boolean     ok = FALSE;
    long double result = 0;
    
    OCINumber*  number = (OCINumber*)getField(key);
    check(OCINumberToReal(errhp, (CONST OCINumber *)number,
                          (uword) sizeof(long double),
                          (dvoid *) &result));    
    return result;
}
void OCIRoutine::setDateTime(const char* key, time_t time, bool null=false)
    throw(SQLException, InvalidArgumentException)
{
    OCIDataDescriptor* descriptor = findDescriptor(key);
    if (descriptor->type != SQLT_ODT)
        throw InvalidArgumentException();
    
    if (!null) 
    {
        convertDateToOCIDate(&time, (OCIDate *)descriptor->data);
        descriptor->ind = OCI_IND_NOTNULL;
    }
    else descriptor->ind = OCI_IND_NULL;
    bind((text *)key, strlen(key), descriptor->type, descriptor->data,\
         descriptor->size, (dvoid *) &descriptor->ind);\
}
time_t OCIRoutine::getDateTime(const char* key)
    throw(SQLException, InvalidArgumentException)
{
    time_t sys_date;
    OCIDate* oci_date = (OCIDate *)getField(key);
    convertOCIDateToDate(oci_date, &sys_date);
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

