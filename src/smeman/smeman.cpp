/*
	$Id$
*/
#include "smeman.h"

namespace smsc {
namespace smeman {

#define __synchronized__ MutexGuard mguard(lock);

// ---- SmeAdministrator implementation --------------

void SmeManager::addSme(const SmeInfo& info)
{
__synchronized__
	SmeRecord record;
	record.info = info;
	record.proxy = 0;
	record.deleted = false;
	record.idx = records.size();
	SmeIndex index = internalLookup(info.systemId);
	if ( index != INVALID_SME_INDEX ) throw SmeError;
	records.push_back(record);
}

void SmeManager::deleteSme(const SmeSystemId& systemid)
{
__synchronized__	
	SmeIndex index = internalLookup(info.systemId);
	if ( index == INVALID_SME_INDEX ) throw SmeError;
	if ( records[index].proxy )
	{
		// ???????	что делать если уже в работе , шутдаунить прокси, как корректно или абортом
		// к примеру так, но тогда , что делает close?
		dispatcher.detachSmeProxy(records[index].proxy);
		records[index].proxy->close();
		delete records[index].proxy;
		records[index].proxy = 0;
	}
	records[index].deleted = true;
}

void SmeManager::store()
{
}

class SmeIteratorImpl
{
	RecordsVector::iterator begin;
	RecordsVector::iterator end;
	RecordsVector::iterator ptr;
	bool started;
public:
	SmeIteratorImpl(RecordsVector::iterator& begin,RecordsVector::iterator& end):
		begin(begin),end(end),ptr(begin),started(false) {}
	
	virtual bool next()
	{
		if (started&&ptr!=end) ++ptr;
		return ptr != end;
	}

	virtual SmeProxy* getSmeProxy() const
	{
		return (*ptr).proxy;
	}
	virtual SmeInfo  getSmeInfo() const
	{
		return (*ptr).info;
	}
	virtual SmeIndex getSmeIndex() const
	{
		return (*ptr).idx;
	}
};

SmeIterator SmeManager::iterator()
{
__synchronized__	
	return SmeIteratorImpl(records.begin(),records.end());
}

void SmeManager::disableSme(const SmeSystemId& systemid)
{
	SmeIndex index = internalLookup(info.systemId);
	if ( index == INVALID_SME_INDEX ) throw SmeError;
	// ???????	что делать если уже в работе , шутдаунить прокси, как корректно или абортом
	records[index].info.disabled = true;
}

void SmeManager::enableSme(const SmeSystemId& systemId)
{
	SmeIndex index = internalLookup(info.systemId);
	if ( index != INVALID_SME_INDEX ) throw SmeError;
	// ???????	что делать если уже в работе , шутдаунить прокси, как корректно или абортом
	records[index].info.disabled = false;
}

// ----- SmeTable implementation ---------------------------

SmeIndex SmeManager::lookup(const SmeSystemId& systemId) const
{
	SmeIndex index = internalLookup(info.systemId);
	if ( index == NVALID_SME_INDEX ) throw SmeError;
	return index;
}

SmeProxy* SmeManager::getSmeProxy(SmeIndex index) const
{
__synchronized__	
	SmeRecord& record = records.at(index);
	if ( record.deleted ) throw SmeError;
	return record.proxy;
}

SmeInfo SmeManager::getSmeInfo(SmeIndex index) const
{
__synchronized__	
	SmeRecord& record = records.at(index);
	if ( record.deleted ) throw SmeError;
	return record.info;
}

// ------ SmeRegistrar implementation --------------------------

void SmeManager::registerSmeProxy(const SmeSystemId& systemId, SmeProxy* smeProxy)
{
__synchronized__

	__require__ ( smeProxy != NULL );

	SmeIndex index = internalLookup(info.systemId);
	if ( index == INVALID_SME_INDEX ) throw SmeError;
	if ( records[index].proxy ) throw SmeError;
	records[index].proxy = smeProxy;
	dispatcher.attachSmeProxy(smeProxy);
}

// SmeDispatcher implementation
SmeProxy* SmeManager::selectSmeProxy()
{
	return dispatcher.dispatchIn();
}

SmeIndex internalLookup(const SmeSystemId& systemId)
{
//__synchronized__ не нужно поскольку вызывается из синхронизированных методов
	for ( RecordsVector::iterator it = records.begin(); p != records.end(); ++p )
	{
		if ( p->info.systemId.compare(systemId) == 0 ) return p->idx;
	}
	return INVALID_SME_INDEX;
}

}; // namespace smeman
}; // namespace smsc
