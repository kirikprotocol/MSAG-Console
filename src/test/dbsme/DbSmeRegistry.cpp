#include "DbSmeRegistry.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace dbsme {

using smsc::core::synchronization::MutexGuard;

void DbSmeTestRecord::setDefInput(DbSmeTestRecord* rec)
{
	__require__(rec);
	rec->ref();
	if (defInput)
	{
		defInput->unref();
	}
	defInput = rec;
}

DbSmeTestRecord::~DbSmeTestRecord()
{
	if (defInput)
	{
		defInput->unref();
	}
}

DbSmeRegistry::~DbSmeRegistry()
{
	clear();
}

void DbSmeRegistry::putRecord(DbSmeTestRecord* rec)
{
	__require__(rec && rec->checkId());
	RecordMap::iterator it = recordMap.find(rec->getId());
	__require__(it == recordMap.end());
	recordMap[rec->getId()] = rec;
	rec->ref();
}
	
void DbSmeRegistry::updateRecord(int newId, DbSmeTestRecord* rec)
{
	__require__(rec && rec->checkId());
	int res = recordMap.erase(rec->getId());
	__require__(res);
	rec->setId(newId);
	recordMap[rec->getId()] = rec;
}

DbSmeTestRecord* DbSmeRegistry::getRecord(int id)
{
	RecordMap::iterator it = recordMap.find(id);
	return (it != recordMap.end() ? it->second : NULL);
}

DbSmeRegistry::DbSmeTestRecordIterator* DbSmeRegistry::getRecords() const
{
	return new DbSmeTestRecordIterator(recordMap.begin(), recordMap.end());
}

bool DbSmeRegistry::removeRecord(int id)
{
	RecordMap::iterator it = recordMap.find(id);
	if (it != recordMap.end())
	{
		__require__(it->second);
		it->second->unref();
		recordMap.erase(it);
		return true;
	}
	return false;
}

int DbSmeRegistry::nextId()
{
	MutexGuard mguard(mutex);
	return lastId++;
}

int DbSmeRegistry::getExistentId()
{
	return (recordMap.begin() == recordMap.end() ?
		0 : recordMap.begin()->second->getId());
}

int DbSmeRegistry::size()
{
	return recordMap.size();
}

void DbSmeRegistry::clear()
{
	for (RecordMap::iterator it = recordMap.begin(); it != recordMap.end(); it++)
	{
		__require__(it->second);
		it->second->unref();
	}
	recordMap.clear();
}

}
}
}

