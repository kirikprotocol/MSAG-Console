#ifndef TEST_DB_SME_DB_SME_REGISTRY
#define TEST_DB_SME_DB_SME_REGISTRY

#include "core/synchronization/Mutex.hpp"
#include "test/core/PduUtil.hpp"
#include <map>
#include <string>
#include <vector>

namespace smsc {
namespace test {
namespace dbsme {

using std::string;
using std::map;
using std::vector;
using smsc::core::synchronization::Mutex;
using smsc::test::core::PduDataObject;

#define __field__(idx, type, name) \
	type __##name; \
	type get##name() const { return __##name; } \
	void set##name(const type& val) { __##name = val; mask[idx] = true; } \
	bool check##name() const { return mask[idx]; }

struct DbSmeTestRecord : public PduDataObject
{
	vector<bool> mask;
	__field__(0, string, Job)
	__field__(1, int, Id)
	__field__(2, int, Int16)
	__field__(3, int, Int32)
	__field__(4, double, Float)
	__field__(5, double, Double)
	__field__(6, time_t, Date)
	__field__(7, string, String)
	__field__(8, string, QuotedString)
	__field__(9, string, FromAddr)

	DbSmeTestRecord() : mask(10, false) {}
};

class DbSmeRegistry
{
	typedef map<int, DbSmeTestRecord*> RecordMap;
	RecordMap recordMap;
	int lastId;
	Mutex mutex;

public:
	struct DbSmeTestRecordIterator
	{
		RecordMap::const_iterator it1;
		RecordMap::const_iterator it2;
		DbSmeTestRecordIterator(RecordMap::const_iterator i1,
			RecordMap::const_iterator i2) : it1(i1), it2(i2) {}
		virtual DbSmeTestRecord* next()
		{
			return (it1 != it2 ? (it1++)->second : NULL);
		}
	};

	DbSmeRegistry() : lastId(0) {}

	virtual ~DbSmeRegistry();
	
	Mutex& getMutex() { return mutex; }

	void putRecord(DbSmeTestRecord* rec);

	void updateRecord(int newId, DbSmeTestRecord* rec);

	DbSmeTestRecord* getRecord(int id);

	DbSmeTestRecordIterator* getRecords() const;

	bool removeRecord(int id);

	int nextId();

	int getExistentId();

	int size();

	void clear();
};

}
}
}

#endif /* TEST_DB_SME_DB_SME_REGISTRY */

