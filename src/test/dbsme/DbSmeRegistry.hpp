#ifndef TEST_DB_SME_DB_SME_REGISTRY
#define TEST_DB_SME_DB_SME_REGISTRY

#include "core/synchronization/Mutex.hpp"
#include <map>
#include <string>

namespace smsc {
namespace test {
namespace dbsme {

using std::string;
using std::map;
using smsc::core::synchronization::Mutex;

struct DbSmeTestRecord
{
	const int id;
	string job;
	int int16;
	int32_t int32;
	float flt;
	double dbl;
	string dateFormat;
	time_t dt;
	bool quotedString;
	string str;

	DbSmeTestRecord(int _id)
		: id(_id), int16(0), int32(0), flt(0.0), dbl(0.0), dt(0), quotedString(false) {}
};

class DbSmeRegistry
{
	typedef map<int, DbSmeTestRecord*> RecordMap;
	RecordMap recordMap;
	int lastId;
	Mutex mutex;

public:
	DbSmeRegistry() : lastId(0) {}

	virtual ~DbSmeRegistry();

	void putRecord(const DbSmeTestRecord& rec);
	
	DbSmeTestRecord* getRecord(int id);

	bool removeRecord(int id);

	int nextId();

	int size();

	void clear();
};

}
}
}

#endif /* TEST_DB_SME_DB_SME_REGISTRY */

