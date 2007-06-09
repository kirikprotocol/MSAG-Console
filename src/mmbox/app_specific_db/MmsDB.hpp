#ifndef __BDB_APP_SPECIFIC_DB_MMSDB_HPP__
# define __BDB_APP_SPECIFIC_DB_MMSDB_HPP__ 1

# include <db/bdb_framework/StorableDbRecord_Iface.hpp>
# include <db/bdb_framework/GenericBDB.hpp>

# include <mmbox/app_specific_db/MmsRecord.hpp>

# include <util/Exception.hpp>

# include <string>

namespace mmbox {
namespace app_specific_db {

class JoinQuery_AbonentAddr_And_Status : public smsc::db::bdb_framework::Join<smsc::db::bdb_framework::QueryByNonuniqKey<MmsDbRecord, MmsDb_AbonentAddr_NonUniqKey> >,
                                         public smsc::db::bdb_framework::Join<smsc::db::bdb_framework::QueryByNonuniqKey<MmsDbRecord, MmsDb_Status_NonUniqKey> >
{
public:
  JoinQuery_AbonentAddr_And_Status(smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>& db,
                                   smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>& db_idx_1,
                                   smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_Status_NonUniqKey>& db_idx_2);

  using smsc::db::bdb_framework::Join<smsc::db::bdb_framework::QueryByNonuniqKey<MmsDbRecord, MmsDb_AbonentAddr_NonUniqKey> >::setQueryKey;
  using smsc::db::bdb_framework::Join<smsc::db::bdb_framework::QueryByNonuniqKey<MmsDbRecord, MmsDb_Status_NonUniqKey> >::setQueryKey;
  using smsc::db::bdb_framework::Base_Join<MmsDbRecord>::execute;
};

class MmsDB : public smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>,
              public smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>,
              public smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_Status_NonUniqKey> {
public:
  MmsDB(const std::string& dbName);
  void open();
  void close();
  using smsc::db::bdb_framework::GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::createStatement;
};

}}

#endif
