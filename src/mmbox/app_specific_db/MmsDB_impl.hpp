#ifndef __BDB_APP_SPECIFIC_DB_MMSDB_IMPL_HPP__
# define __BDB_APP_SPECIFIC_DB_MMSDB_IMPL_HPP__1 

# include <mmbox/app_specific_db/MmsDB.hpp>

using namespace smsc::db::bdb_framework;
using namespace mmbox::app_specific_db;

template <>
std::auto_ptr<InsertStatement<MmsDbRecord> >
GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::createStatement<InsertStatement<MmsDbRecord> >() {
  std::auto_ptr<InsertStatement<MmsDbRecord> > statement(new InsertStatement<MmsDbRecord>(*this));

  return statement;
}

template <>
std::auto_ptr<QueryByNonuniqKey<MmsDbRecord, MmsDb_AbonentAddr_NonUniqKey> >
GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::createStatement<QueryByNonuniqKey<MmsDbRecord, MmsDb_AbonentAddr_NonUniqKey> >() {
  GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>* secondaryIdx
    = static_cast<GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>* >(get_bdb_idx(MmsDb_AbonentAddr_NonUniqKey::getKeyName()));

  return secondaryIdx->createQueryStatement();
}

template <>
std::auto_ptr<JoinQuery_AbonentAddr_And_Status>
GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::createStatement<JoinQuery_AbonentAddr_And_Status>() {
  GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>* abonentAddr_secondaryIdx
    = static_cast<GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_AbonentAddr_NonUniqKey>* >(get_bdb_idx(MmsDb_AbonentAddr_NonUniqKey::getKeyName()));

  GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_Status_NonUniqKey>* status_secondaryIdx
    = static_cast<GenericBDB<MmsDbRecord, MmsDbRecord::PrimaryKey>::SecondaryIndex<MmsDb_Status_NonUniqKey>* >(get_bdb_idx(MmsDb_Status_NonUniqKey::getKeyName()));

  std::auto_ptr<JoinQuery_AbonentAddr_And_Status> statement(new JoinQuery_AbonentAddr_And_Status(*this, *abonentAddr_secondaryIdx, *status_secondaryIdx));

  return statement;
}

#endif
