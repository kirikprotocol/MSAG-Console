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

typedef enum
{
	DT_NOW = 0x0,
	DT_TODAY = 0x1,
	DT_YESTERDAY = 0x2,
	DT_TOMORROW = 0x3
} DateType;

#define __field__(idx, type, name) \
	type __##name; \
	type get##name() const { return __##name; } \
	void set##name(const type& val) { __##name = val; mask[idx] = true; } \
	bool check##name() const { return mask[idx]; }

class DbSmeTestRecord : public PduDataObject
{
	vector<bool> mask;
	DbSmeTestRecord* defInput;

	DbSmeTestRecord(const DbSmeTestRecord&) {}
	DbSmeTestRecord& operator=(const DbSmeTestRecord& rec) {}
public:
	__field__(0, string, Job)
	__field__(1, int, Id)
	__field__(2, int, Int8)
	__field__(3, int, Int16)
	__field__(4, int, Int32)
	__field__(5, int, Int64)
	__field__(6, double, Float)
	__field__(7, double, Double)
	__field__(8, double, LongDouble)
	__field__(9, DateType, DateType)
	__field__(10, time_t, Date)
	__field__(11, string, String)
	__field__(12, string, QuotedString)
	__field__(13, string, FromAddr)
	__field__(14, string, ToAddr)
	DbSmeTestRecord* getDefInput() const { return defInput; }
	void setDefInput(DbSmeTestRecord* defInput);
	bool checkDefInput() const { return defInput; }

	DbSmeTestRecord() : mask(15, false), defInput(NULL) {}
	virtual ~DbSmeTestRecord();
	//Обновляет все поля, кроме Job, Id и DateType
	void update(const DbSmeTestRecord& rec);
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

