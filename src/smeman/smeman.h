/*
	$Id$
*/

#include "util/debug.h"
#include <vector>

#if !defined __Cpp_Header__smeman_smeman_h__
#define __Cpp_Header__smeman_smeman_h__
#include <inttypes.h>

#include "smetypes.h"
#include "smeproxy.h"
#include "smeinfo.h"
#include "smeadmin.h"
#include "smetable.h"
#include "smeiter.h"
#include "smereg.h"
#include "smedispatch.h"
#include "core/synchronization/mutex.hpp"

namespace smsc {
namespace smeman {

class SmeManager : 
	public SmeAdministrator,
	public SmeTable,
	public SmeIterator,
	public SmeRegistrar,
	public SmeDispatcher
{
	Mutex lock;
	class SmeProxyDispatcher
	{
		ProxyMonitor mon;
	public:
		SmeProxy* dispatchIn();
		void attachSmeProxy(SmeProxy* proxy);
		void detachSmeProxy(SmeProxy* proxy);
	};
	SmeProxyDispatcher dispatcher;
	struct SmeRecord
	{
		SmeInfo info;
		SmeProxy* proxy;
		bool deleted;
		SmeIndex idx;
	};
	typedef std::vector<SmeRecord> Records;
	Records records;
	SmeIndex internalLookup(const SmeSystemId& systemId);
public:	
	//....
	// SmeAdministrator implementation
	virtual void addSme(const SmeInfo& info);
	virtual void deleteSme(const SmeSystemId& systemId);
	virtual void store();
	virtual SmeIterator iterator();
	virtual void disableSme(const SmeSystemId& systemId);
	virtual void enableSme(const SmeSystemId& systemId);

	// SmeTable implementation
	virtual SmeIndex lookup(const SmeSystemId& systemId) const;
	virtual SmeProxy* getSmeProxy(SmeIndex index) const;
	virtual SmeInfo getSmeInfo(SmeIndex index) const;

/*
	// SmeIterator implementation
	virtual bool next();
	virtual SmeProxy* getSmeProxy() const;
	virtual SmeInfo  getSmeInfo() const;
	virtual SmeIndex getSmeIndex() const; // ?????
*/
	// SmeRegistrar implementation
	virtual void registerSmeProxy(const SmeSystemId& systemId, SmeProxy* smeProxy);

	// SmeDispatcher implementation
	virtual SmeProxy* selectSmeProxy();
};

}; // namespace smeman
}; // namespace smsc
#endif



