#include "MmsDB.hpp"
#include <util/Exception.hpp>

namespace mmbox {
namespace app_specific_db {

MmsDB::MmsDB(const std::string& dbName)
  : smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>(dbName),
    smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>(*(smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>*)this),
  smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_Status_NonUniqKey>(*(smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>*)this) {}

JoinQuery_AbonentAddr_And_Status::JoinQuery_AbonentAddr_And_Status(smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>& db,
                                                                   smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>& db_idx_1,
                                                                   smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_Status_NonUniqKey>& db_idx_2)
  : smsc::db::bdb_framework::Base_Join<MmsDbRecord>(db),
    smsc::db::bdb_framework::Join<smsc::db::bdb_framework::QueryByNonuniqKey<MmsDbRecord, MmsDb_AbonentAddr_NonUniqKey> >(db, db_idx_1),
    smsc::db::bdb_framework::Join<smsc::db::bdb_framework::QueryByNonuniqKey<MmsDbRecord, MmsDb_Status_NonUniqKey> >(db, db_idx_2)
{}

void
MmsDB::open()
{
  smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::open();
  smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>::openIdx();
  smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_Status_NonUniqKey>::openIdx();
}

void
MmsDB::close()
{
  smsc::logger::Logger *logger = smsc::logger::Logger::getInstance("bdb");
  smsc_log_debug(logger, "### MmsDB::close");

  smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_Status_NonUniqKey>::closeIdx();
  smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>::closeIdx();
  smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::close();
}

}}
