#include "DbSmeRegistry.hpp"
#include "util/debug.h"

namespace smsc {
namespace test {
namespace dbsme {

using smsc::core::synchronization::MutexGuard;

#define __sync__ \
	MutexGuard mguard(mutex)

DbSmeRegistry::~DbSmeRegistry()
{
	clear();
}

void DbSmeRegistry::putRecord(const DbSmeTestRecord& rec)
{
	__sync__;
	RecordMap::iterator it = recordMap.find(rec.id);
	__require__(it == recordMap.end());
	recordMap[rec.id] = new DbSmeTestRecord(rec);
}
	
DbSmeTestRecord* DbSmeRegistry::getRecord(int id)
{
	RecordMap::iterator it = recordMap.find(id);
	return (it != recordMap.end() ? it->second : NULL);
}

bool DbSmeRegistry::removeRecord(int id)
{
	__sync__;
	return recordMap.erase(id);
}

int DbSmeRegistry::nextId()
{
	__sync__;
	return lastId++;
}

int DbSmeRegistry::size()
{
	__sync__;
	return recordMap.size();
}

void DbSmeRegistry::clear()
{
	__sync__;
	for (RecordMap::iterator it = recordMap.begin(); it != recordMap.end(); it++)
	{
		__require__(it->second);
		delete it->second;
	}
}

}
}
}

