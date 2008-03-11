#pragma ident "$Id$"
/* ************************************************************************** *
 * DB Abonent Provider: implements functionality for quering Oracle DB
 * for abonent contract.
 * ************************************************************************** *
 * Expects Provider Config subsection formed as follow:
 *
 *  <section name="Config">
 *      <section name="DataSource">
 *          <param name="type" type="string">OCI</param> <!-- Oracle DB -->
 *          <param name="dbInstance" type="string">....</param>
 *          <param name="dbUserName" type="string">....</param>
 *          <param name="dbUserPassword" type="string">....</param>
 *          <param name="connections" type="int">5</param>  <!--  max allowed db connections -->
 *          <param name="maxQueries" type="int">15</param>  <!--  max allowed db queries -->
 *          <param name="watchdog" type="bool">true</param>  <!--  turn on timeouts handling -->
 *          <param name="timeout" type="int">15</param>  <!--  timeout on connection queries, units: secs  [1..65535] -->
 *
 *       <!--  SQL function returning Abonent type has following prototype:
 *              FUNCTION abonentQueryFunc (iqueryFuncArg  IN VARCHAR) RETURN NUMBER
 *        -->
 *          <param name="abonentQueryFunc" type="string">....</param>
 *          <param name="queryFuncArg" type="string">....</param>
 *      </section>
 *      <section name="DataSourceDrivers">
 *          <section name="OCIDataSourceDriver">
 *              <param name="type" type="string">OCI</param>
 *              <param name="loadup" type="string">libdb_oci.so</param>
 *          </section>
 *      </section>
 *  </section>
 * ************************************************************************** */
#ifndef SMSC_INMAN_IAPROVIDER_DB_HPP
#define SMSC_INMAN_IAPROVIDER_DB_HPP

#include "inman/abprov/IAProvider.hpp"
using smsc::inman::iaprvd::IAProvider;
using smsc::inman::iaprvd::IAProviderCreatorITF;
using smsc::inman::iaprvd::IAProviderITF;

#include "inman/abprov/facility/IAPThrFacility.hpp"
using smsc::inman::iaprvd::IAPQueryAC;
using smsc::inman::iaprvd::IAPQueryManagerITF;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

#include "db/DataSource.h"
using smsc::db::DataSource;

namespace smsc {
namespace inman {
namespace iaprvd {
namespace db { 

struct IAPQueryDB_CFG {
    DataSource *    ds;
    std::string     sqlRtId;   //SQL function name
    std::string     sqlRtKey;  //SQL function argument name
    unsigned        timeOut_secs;

    IAPQueryDB_CFG() : ds(0), timeOut_secs(0)
    { }
    inline const char * rtId(void) const { return sqlRtId.c_str(); }
    inline const char * rtKey(void) const { return sqlRtKey.c_str(); }
};

class IAPQueryDB : public IAPQueryAC {
protected:
    IAPQueryDB_CFG      _cfg;
    std::string         callStr;

public:
    IAPQueryDB(unsigned q_id, IAPQueryManagerITF * owner, 
               Logger * use_log, const IAPQueryDB_CFG & use_cfg);
    // ****************************************
    //-- IAPQueryAC implementation:
    // ****************************************
    int Execute(void);
    const char * taskType(void) const { return "IAPQueryDB"; }
};

struct DBSourceCFG {
    IAPQueryDB_CFG  qryCfg;
    unsigned        max_queries;
    unsigned        init_threads;

    DBSourceCFG() { max_queries = init_threads = 0; }
};

class DBQueryFactory : public IAPQueryFactoryITF {
private:
    IAPQueryDB_CFG  _cfg;
    Logger *        logger;

public:
    DBQueryFactory(const IAPQueryDB_CFG &in_cfg, unsigned timeout_secs,
                   Logger * uselog = NULL);
    ~DBQueryFactory() { }

    // ****************************************
    //-- IAPQueryFactoryITF implementation:
    // ****************************************
    IAPQueryAC * newQuery(unsigned q_id, IAPQueryManagerITF * owner, Logger * use_log);
};


class IAProviderCreatorDB: public IAProviderCreatorITF {
protected:
    mutable Mutex           _sync;
    std::auto_ptr<IAProviderThreaded> prvd;
    IAProviderThreadedCFG   prvdCfg;
    IAPQueryDB_CFG          qryCfg;
    Logger *                logger;
    ICSIdsSet               icsDeps; //there are no dependencies!

public:
    IAProviderCreatorDB(const DBSourceCFG & use_cfg, Logger * use_log = NULL);
    ~IAProviderCreatorDB();

    // ****************************************
    // -- IAProviderCreatorITF interface
    // ****************************************
    inline IAProvider::Type    type(void)      const { return IAProvider::iapDB; }
    inline IAProvider::Ability ability(void)   const { return IAProvider::abContract; }
    inline const char *        ident(void)     const { return "iapDB_OCI"; }
    inline const ICSIdsSet &   ICSDeps(void)   const { return icsDeps; }
    //
    void                logConfig(Logger * use_log = NULL) const;
    //Ensures the provider is properly initialized and returns its interface
    IAProviderITF *     startProvider(const ICServicesHostITF * use_host);
    void                stopProvider(bool do_wait = false);
};


} //db
} //iaprvd
} //inman
} //smsc

#endif /* SMSC_INMAN_IAPROVIDER_DB_HPP */

