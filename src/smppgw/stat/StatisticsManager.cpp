
#include "StatisticsManager.h"
#include <memory>

namespace smsc {
namespace smppgw {
namespace stat {

GWStatisticsManager::GWStatisticsManager(DataSource& _ds)
    :  logger(Logger::getInstance("smsc.stat.GWStatisticsManager")),
            ds(_ds), currentIndex(0), isStarted(false), bExternalFlush(false)
{
}

GWStatisticsManager::~GWStatisticsManager()
{
  MutexGuard guard(stopLock);
}

void GWStatisticsManager::incError(IntHash<int>& hash, int errcode)
{
    int* counter = hash.GetPtr(errcode);
    if (!counter) hash.Insert(errcode, 1);
    else (*counter)++;
}

void GWStatisticsManager::updateCounter(int counter,const char* srcSmeId, const char* routeId, int errcode)
{
  MutexGuard  switchGuard(switchLock);

  CommonStat* smeSt=0;
  CommonStat* routeSt=0;
  ServiceStat* srvSt=0;

  if (srcSmeId && srcSmeId[0])
  {
    TotalStat *st = totalStatBySmeId[currentIndex].GetPtr(srcSmeId);
    if(!st)
    {
      TotalStat newStat;
      totalStatBySmeId[currentIndex].Insert(srcSmeId, newStat);
      st=totalStatBySmeId[currentIndex].GetPtr(srcSmeId);
    }
    smeSt=&st->common;
    srvSt=&st->service;
  }

  if (routeId && routeId[0])
  {
    routeSt = commonStatByRoute[currentIndex].GetPtr(srcSmeId);
    if(!routeSt)
    {
      CommonStat newStat;
      commonStatByRoute[currentIndex].Insert(srcSmeId, newStat);
      routeSt=commonStatByRoute[currentIndex].GetPtr(srcSmeId);
    }
  }

  using namespace Counters;

  if(counter<cntServiceBase)
  {
    if(smeSt)incError(smeSt->errors,errcode);
    if(routeSt)incError(smeSt->errors,errcode);
  }

  switch(counter)
  {
#define INC_STAT(cnt,field) case cnt:{\
      statCommon[currentIndex].field++; \
      if(smeSt)smeSt->field++; \
      if(routeSt)routeSt->field++; \
      }break;

    INC_STAT(cntAccepted,accepted)
    INC_STAT(cntRejected,rejected)
    INC_STAT(cntDelivered,delivered)
    INC_STAT(cntTemp,temperror)
    INC_STAT(cntPerm,permerror)

#undef INC_STAT

#define UPDATE_SRV_STAT_CNT(name) \
    case cnt##name: \
      if(srvSt)srvSt->name++; \
      break;

    UPDATE_SRV_STAT_CNT(SmsTrOk)
    UPDATE_SRV_STAT_CNT(SmsTrFailed)
    UPDATE_SRV_STAT_CNT(SmsTrBilled)
    UPDATE_SRV_STAT_CNT(UssdTrFromScOk)
    UPDATE_SRV_STAT_CNT(UssdTrFromScFailed)
    UPDATE_SRV_STAT_CNT(UssdTrFromScFailedBilled)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeOk)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeFailed)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeFailedBilled)

#undef UPDATE_SRV_STAT_CNT
  }

  incError(statCommon[currentIndex].errors, errcode);
}


int GWStatisticsManager::Execute()
{
    smsc_log_debug(logger, "Execute() started (%d)", isStopping);
    isStarted = true; bExternalFlush = false;
    while (!isStopping)
    {
        int toSleep = calculateToSleep();
        smsc_log_debug(logger, "Execute() >> Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        smsc_log_debug(logger, "Execute() >> End wait");

        flushCounters(switchCounters());
        bExternalFlush = false;
        doneEvent.Signal();
        smsc_log_debug(logger, "Execute() >> Flushed");
    }
    isStarted = false;
    exitEvent.Signal();
    smsc_log_debug(logger, "Execute() exited");
    return 0;
}

void GWStatisticsManager::stop()
{
    MutexGuard guard(stopLock);

    smsc_log_debug(logger, "stop() called, started=%d", isStarted);
    ThreadedTask::stop();
    if (isStarted)
    {
        bExternalFlush = true;
        awakeEvent.Signal();
        smsc_log_debug(logger, "stop() waiting finish ...");
        exitEvent.Wait();
    }
    smsc_log_debug(logger, "stop() exited");
}

void GWStatisticsManager::flushStatistics()
{
    MutexGuard flushGuard(flushLock);

    if (doneEvent.isSignaled()) doneEvent.Wait(0);
    bExternalFlush = true;
    awakeEvent.Signal();
    doneEvent.Wait();
}

int GWStatisticsManager::switchCounters()
{
    MutexGuard  switchGuard(switchLock);

    int flushIndex = currentIndex;
    currentIndex ^= 1; //switch between 0 and 1
    return flushIndex;
}

uint32_t GWStatisticsManager::calculatePeriod()
{
    time_t currTime = time(0);
    if (!bExternalFlush) currTime -= 60;
    tm tmCT;
    localtime_r(&currTime, &tmCT);
    return  (tmCT.tm_year+1900)*1000000+(tmCT.tm_mon+1)*10000+
            (tmCT.tm_mday)*100+tmCT.tm_hour;
}
int GWStatisticsManager::calculateToSleep() // returns msecs to next hour
{
    time_t currTime = time(0);
    time_t nextTime = currTime + 60;
    tm tmNT; localtime_r(&nextTime, &tmNT);
    tmNT.tm_sec = 0;
    nextTime = mktime(&tmNT);
    return (((nextTime-currTime)*1000)+1);
}

const char* insertStatSmsSql = (const char*)
"INSERT INTO smppgw_stat (period,accepted,rejected,delivered,temperror,permerror) "
"VALUES (:period,:accepted,:rejected,:delivered,:temperror,:permerror)";

const char* insertStatSmeSql = (const char*)
"INSERT INTO smppgw_stat_sme ("
  "period,systemid,accepted,rejected,delivered,temperror,permerror,"
  "SmsTrOk,SmsTrFailed,SmsTrBilled,"
  "UssdTrFromScOk,UssdTrFromScFailed,UssdTrFromScFailedBilled,"
  "UssdTrFromSmeOk,UssdTrFromSmeFailed,UssdTrFromSmeFailedBilled"
") VALUES ("
  ":period,:systemid,:accepted,:rejected,:delivered,:temperror,:permerror,"
  ":SmsTrOk,:SmsTrFailed,:SmsTrBilled,"
  ":UssdTrFromScOk,:UssdTrFromScFailed,:UssdTrFromScFailedBilled,"
  ":UssdTrFromSmeOk,:UssdTrFromSmeFailed,:UssdTrFromSmeFailedBilled"
")";

const char* insertStatRouteSql = (const char*)
"INSERT INTO smppgw_stat_route (period,routeid,accepted,rejected,delivered,temperror,permerror)"
"VALUES (:period,:routeid,:accepted,:rejected,:delivered,:temperror,:permerror)";

const char* insertStatErrSql = (const char*)
"INSERT INTO smppgw_stat_errors (period, errcode, counter) "
"VALUES (:period, :errcode, :counter)";
const char* insertStatSmeErrSql = (const char*)
"INSERT INTO SMPPGW_STAT_SME_ERRORS (period, systemid, errcode, counter) "
"VALUES (:period, :systemid, :errcode, :counter)";
const char* insertStatRouteErrSql = (const char*)
"INSERT INTO SMPPGW_STAT_ROUTE_ERRORS (period, routeid, errcode, counter) "
"VALUES (:period, :routeid, :errcode, :counter)";

void GWStatisticsManager::flushCounters(int index)
{

    uint32_t period = calculatePeriod();

    smsc_log_debug(logger, "Flushing statistics for period: %d / %d", period, time(0));

    Connection* connection = 0;

    using std::auto_ptr;

    auto_ptr<Statement> insertStatSmeStmt;
    auto_ptr<Statement> insertStatSmsStmt;
    auto_ptr<Statement> insertStatRouteStmt;
    auto_ptr<Statement> insertStatErrStmt;
    auto_ptr<Statement> insertStatSmeErrStmt;
    auto_ptr<Statement> insertStatRouteErrStmt;

    try
    {
        if (!(connection = ds.getConnection()))
            throw SQLException("Statistics: Failed to obtain DB connection!");

#define CREATE_STATEMENT(st) st##Stmt   = auto_ptr<Statement>(connection->createStatement(st##Sql))
      CREATE_STATEMENT(insertStatSme);
      CREATE_STATEMENT(insertStatSms);
      CREATE_STATEMENT(insertStatRoute);
      CREATE_STATEMENT(insertStatErr);
      CREATE_STATEMENT(insertStatSmeErr);
      CREATE_STATEMENT(insertStatRouteErr);
#undef CREATE_STATEMENT

        if (!insertStatSmeStmt.get() || !insertStatSmsStmt.get() || !insertStatRouteStmt.get() ||
            !insertStatErrStmt.get() || !insertStatSmeErrStmt.get() || !insertStatRouteErrStmt.get())
            throw SQLException("Statistics: Failed to create service statements!");

        insertStatSmsStmt->setUint32(1, period);
        insertStatSmsStmt->setInt32 (2, statCommon[index].accepted);
        insertStatSmsStmt->setInt32 (3, statCommon[index].rejected);
        insertStatSmsStmt->setInt32 (4, statCommon[index].delivered);
        insertStatSmsStmt->setInt32 (5, statCommon[index].temperror);
        insertStatSmsStmt->setInt32 (6, statCommon[index].permerror);
        insertStatSmsStmt->executeUpdate();

        insertStatErrStmt->setUint32(1, period);
        IntHash<int>::Iterator it = statCommon[index].errors.First();
        int ecError, eCounter;
        while (it.Next(ecError, eCounter))
        {
            insertStatErrStmt->setInt32(2, ecError);
            insertStatErrStmt->setInt32(3, eCounter);
            insertStatErrStmt->executeUpdate();
        }

        insertStatRouteStmt->setUint32(1, period);
        insertStatRouteErrStmt->setUint32(1, period);
        commonStatByRoute[index].First();
        char* routeId = 0;
        CommonStat* routeStat = 0;
        while (commonStatByRoute[index].Next(routeId, routeStat))
        {
            if (!routeStat || !routeId || routeId[0] == '\0') continue;
            insertStatRouteStmt->setString(2 , routeId);
            insertStatRouteStmt->setInt32 (3 , routeStat->accepted);
            insertStatRouteStmt->setInt32 (4 , routeStat->rejected);
            insertStatRouteStmt->setInt32 (5 , routeStat->delivered);
            insertStatRouteStmt->setInt32 (6 , routeStat->temperror);
            insertStatRouteStmt->setInt32 (7 , routeStat->permerror);
            insertStatRouteStmt->executeUpdate();

            insertStatRouteErrStmt->setString(2, routeId);
            IntHash<int>::Iterator rit = routeStat->errors.First();
            int recError, reCounter;
            while (rit.Next(recError, reCounter))
            {
                insertStatRouteErrStmt->setInt32(3, recError);
                insertStatRouteErrStmt->setInt32(4, reCounter);
                insertStatRouteErrStmt->executeUpdate();
            }
            routeStat = 0;
        }

        insertStatSmeStmt->setUint32(1, period);
        insertStatSmeErrStmt->setUint32(1, period);
        totalStatBySmeId[index].First();
        char* smeId = 0;
        TotalStat* smeStat = 0;
        while (totalStatBySmeId[index].Next(smeId, smeStat))
        {
            if (!smeStat || !smeId || smeId[0] == '\0') continue;
            insertStatSmeStmt->setString(2 , smeId);
            insertStatSmeStmt->setInt32 (3 , smeStat->common.accepted);
            insertStatSmeStmt->setInt32 (4 , smeStat->common.rejected);
            insertStatSmeStmt->setInt32 (5 , smeStat->common.delivered);
            insertStatSmeStmt->setInt32 (6 , smeStat->common.temperror);
            insertStatSmeStmt->setInt32 (7 , smeStat->common.permerror);
            insertStatSmeStmt->setInt32 (8 , smeStat->service.SmsTrOk);
            insertStatSmeStmt->setInt32 (9 , smeStat->service.SmsTrFailed);
            insertStatSmeStmt->setInt32 (10 , smeStat->service.SmsTrBilled);
            insertStatSmeStmt->setInt32 (11 , smeStat->service.UssdTrFromScOk);
            insertStatSmeStmt->setInt32 (12 , smeStat->service.UssdTrFromScFailed);
            insertStatSmeStmt->setInt32 (13 , smeStat->service.UssdTrFromScFailedBilled);
            insertStatSmeStmt->setInt32 (14 , smeStat->service.UssdTrFromSmeOk);
            insertStatSmeStmt->setInt32 (15 , smeStat->service.UssdTrFromSmeFailed);
            insertStatSmeStmt->setInt32 (16 , smeStat->service.UssdTrFromSmeFailedBilled);

            insertStatSmeStmt->executeUpdate();

            insertStatSmeErrStmt->setString(2, smeId);
            IntHash<int>::Iterator sit = smeStat->common.errors.First();
            int secError, seCounter;
            while (sit.Next(secError, seCounter))
            {
                insertStatSmeErrStmt->setInt32(3, secError);
                insertStatSmeErrStmt->setInt32(4, seCounter);
                insertStatSmeErrStmt->executeUpdate();
            }
            smeStat = 0;
        }

        connection->commit();
    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, exc.what());
        try { if (connection) connection->rollback(); } catch (...) {}
    }
    if (connection) ds.freeConnection(connection);

    resetCounters(index);

}

void GWStatisticsManager::resetCounters(int index)
{
  statCommon[index].Reset();
  totalStatBySmeId[index].Empty();
  commonStatByRoute[index].Empty();
}

}//namespace stat
}//namespace smppgw
}//namespace smsc
