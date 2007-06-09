#include "ApplicationRequestsHandler.hpp"

#include "bdb_objects_transmitter.hpp"

#include <mmbox/app_specific_db/MmsDB.hpp>
#include <mmbox/app_specific_db/MmsDB_impl.hpp>

#include <mmbox/app_protocol/messages/AppResponse_MmsRecordsResultSet.hpp>

#include <logger/Logger.h>

extern mmbox::app_specific_db::MmsDB* mmsDbConnet;

namespace mmbox {
namespace app_pck_handler {

void
ApplicationRequestsHandler::handleEvent(const mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddress* appReq,
                                        const std::string& sourceConnectId)
{
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
  smsc_log_info(logger, "ApplicationRequestsHandler::handleEvent::: handle object [%s] getting from connect with id=[%s]", appReq->toString().c_str(), sourceConnectId.c_str());

  std::auto_ptr<mmbox::app_protocol::AppResponse_MmsRecordsResultSet> queryMmsResult(appReq->createAppResponse(mmbox::app_protocol::ResponseApplicationPacket::STATUS_OK));
  try {
    std::auto_ptr<smsc::db::bdb_framework::QueryByNonuniqKey<mmbox::app_specific_db::MmsDbRecord, mmbox::app_specific_db::MmsDb_AbonentAddr_NonUniqKey> > statement = mmsDbConnet->createStatement<smsc::db::bdb_framework::QueryByNonuniqKey<mmbox::app_specific_db::MmsDbRecord, mmbox::app_specific_db::MmsDb_AbonentAddr_NonUniqKey> >();

    statement->setQueryKey(mmbox::app_specific_db::MmsDb_AbonentAddr_NonUniqKey(appReq->getAbonentAddress()));
    smsc::db::bdb_framework::ResultSet<mmbox::app_specific_db::MmsDbRecord>& resutlSet = statement->executeQuery();

    mmbox::app_specific_db::MmsDbRecord result;
    while ( resutlSet.fetchNextValue(result) ) {
      smsc_log_info(logger, "QueryMmsRecordsByAbonentAddressAppRequest_Subscriber::: fetched row=[%s]", result.toString().c_str());
      queryMmsResult->addRecord(result);
    }

  } catch (const std::exception& ex) {
    smsc_log_error(logger, "InsertMmsRecordAppRequest_Subscriber::handleEvent::: can't insert record to DB: [%s]", ex.what());
    queryMmsResult.reset(appReq->createAppResponse(mmbox::app_protocol::ResponseApplicationPacket::GENERIC_FAILURE));
  }
  bdb_objects_transmitter_t::getInstance().scheduleObjectForWrite(*queryMmsResult,
                                                                  sourceConnectId);
}

void
ApplicationRequestsHandler::handleEvent(const mmbox::app_protocol::AppRequest_InsertMmsRecord* appReq,
                                        const std::string& sourceConnectId)
{
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
  smsc_log_info(logger, "ApplicationRequestsHandler::handleEvent::: handle object [%s] getting from connect with id=[%s]", appReq->toString().c_str(), sourceConnectId.c_str());
  std::auto_ptr<mmbox::app_protocol::ResponseApplicationPacket> insertMmsResult(appReq->createAppResponse(mmbox::app_protocol::ResponseApplicationPacket::STATUS_OK));
  try {
    std::auto_ptr<smsc::db::bdb_framework::InsertStatement<mmbox::app_specific_db::MmsDbRecord> > statement
      = mmsDbConnet->createStatement<smsc::db::bdb_framework::InsertStatement<mmbox::app_specific_db::MmsDbRecord> >();
    statement->setValue(mmbox::app_specific_db::MmsDbRecord(appReq->getMmsId(), appReq->getAbonentAddress(), appReq->getMmsStatus(), appReq->getSentTime()));
    int numOfInsertedRecs = statement->execute();

    if ( numOfInsertedRecs != 1 )
      insertMmsResult.reset(appReq->createAppResponse(mmbox::app_protocol::ResponseApplicationPacket::BDB_FAILURE));
  } catch (const std::exception& ex) {
    smsc_log_error(logger, "InsertMmsRecordAppRequest_Subscriber::handleEvent::: can't insert record to DB: [%s]", ex.what());
    insertMmsResult.reset(appReq->createAppResponse(mmbox::app_protocol::ResponseApplicationPacket::GENERIC_FAILURE));
  }
  bdb_objects_transmitter_t::getInstance().scheduleObjectForWrite(*insertMmsResult,
                                                                  sourceConnectId);
}

void
ApplicationRequestsHandler::handleEvent(const mmbox::app_protocol::AppRequest_QueryMmsRecordsByAbonentAddressAndStatus* appReq,
                                        const std::string& sourceConnectId)
{
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
  smsc_log_info(logger, "ApplicationRequestsHandler::handleEvent::: handle object [%s] getting from connect with id=[%s]", appReq->toString().c_str(), sourceConnectId.c_str());

  std::auto_ptr<mmbox::app_protocol::AppResponse_MmsRecordsResultSet> queryMmsResult(appReq->createAppResponse(mmbox::app_protocol::ResponseApplicationPacket::STATUS_OK));
  try {
    std::auto_ptr<mmbox::app_specific_db::JoinQuery_AbonentAddr_And_Status> statement = mmsDbConnet->createStatement<mmbox::app_specific_db::JoinQuery_AbonentAddr_And_Status>();

    smsc_log_info(logger, "ApplicationRequestsHandler::handleEvent::: JoinQuery_AbonentAddr_And_Status.setQueryKey(%s)", mmbox::app_specific_db::MmsDb_AbonentAddr_NonUniqKey(appReq->getAbonentAddress()).toString().c_str());
    smsc_log_info(logger, "ApplicationRequestsHandler::handleEvent::: JoinQuery_AbonentAddr_And_Status.setQueryKey(%s)", mmbox::app_specific_db::MmsDb_Status_NonUniqKey(appReq->getMmsStatus()).toString().c_str());
    statement->setQueryKey(mmbox::app_specific_db::MmsDb_AbonentAddr_NonUniqKey(appReq->getAbonentAddress()));
    statement->setQueryKey(mmbox::app_specific_db::MmsDb_Status_NonUniqKey(appReq->getMmsStatus()));

    smsc_log_info(logger, "ApplicationRequestsHandler::handleEvent::: JoinQuery_AbonentAddr_And_Status.execute()");
    int st = statement->execute();
    smsc::db::bdb_framework::ResultSet<mmbox::app_specific_db::MmsDbRecord>& resutlSet = statement->getResultSet();

    mmbox::app_specific_db::MmsDbRecord result;
    while ( resutlSet.fetchNextValue(result) ) {
      smsc_log_info(logger, "ApplicationRequestsHandler::handleEvent(AppRequest_QueryMmsRecordByAbonentAddressAndStatus)::: fetched row=[%s]", result.toString().c_str());
      queryMmsResult->addRecord(result);
    }

  } catch (const std::exception& ex) {
    smsc_log_error(logger, "ApplicationRequestsHandler::handleEvent(AppRequest_QueryMmsRecordByAbonentAddressAndStatus)::: can't fetch record from DB: [%s]", ex.what());
    queryMmsResult.reset(appReq->createAppResponse(mmbox::app_protocol::ResponseApplicationPacket::GENERIC_FAILURE));
  }
  bdb_objects_transmitter_t::getInstance().scheduleObjectForWrite(*queryMmsResult,
                                                                  sourceConnectId);
}


}}
