#include "route_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>

namespace smsc {
namespace router {
using namespace smsc::smeman;
using namespace smsc::sms;
using namespace std;

int TestUnit1()
{
	class FakeProxy : public SmeProxy {
		//....
public:
		virtual void close() {}
		virtual void putCommand(const SmscCommand &command) {}
		virtual SmeProxyPriority getPriority() const { return SmeProxyPriorityDefault;}
		virtual SmscCommand getCommand() { SmscCommand c; return c;}
		virtual SmeProxyState getState() const {SmeProxyState s; return s;}
		virtual void init() {}
		virtual bool hasInput() const {return false;}
		virtual void attachMonitor(ProxyMonitor *monitor) {}
		virtual bool attached() {return false;}
		virtual uint32_t getNextSequenceNumber() {return 1;}
	};
	class FakeSmeTable : public SmeTable
	{
	public:
		virtual SmeIndex lookup(const SmeSystemId &systemId) const {
			return (SmeIndex)1;
		}
		virtual SmeProxy* getSmeProxy(SmeIndex index) const {
			return new FakeProxy;
		}
		virtual SmeInfo getSmeInfo(SmeIndex index) const {
			SmeInfo info;
			return info;
		}
	};
	RouteManager router;
	FakeSmeTable table;
	router.assign(&table);
	for ( int i=0; i < 10; ++i )
	{
		RouteInfo info;
		char buff[30]; 
		sprintf(buff,"sme%d",i);
		info.smeSystemId = string(buff);
		sprintf(buff,"902%d*",i);
		info.source = Address(strlen(buff),0,0,buff);
		sprintf(buff,"903%d*",i);
		info.dest = Address(strlen(buff),0,0,buff);
		router.addRoute(info);
	}
	for ( int i=0; i<10; ++i )
	{
		RouteInfo info;
		char buff[30]; 
		sprintf(buff,"902%d1",i);
		Address source = Address(strlen(buff),0,0,buff);
		sprintf(buff,"903%d2",i);
		Address dest = Address(strlen(buff),0,0,buff);
		SmeProxy* proxy;
		fprintf(stderr,"try lookup 902%d1->903%d2\n",i,i);
		if ( !router.lookup(source,dest,proxy,0) )
		{
			fprintf(stderr,"can't lookup 902%d1->903%d2\n",i,i);
		}
		else
		{
			fprintf(stderr,"route 902%d1->903%d2 proxy %p\n",i,i,proxy);
		}
	}
	for ( int i=0; i<10; ++i )
	{
		RouteInfo info;
		char buff[30]; 
		sprintf(buff,"902%d1",i);
		Address source = Address(strlen(buff),0,0,buff);
		sprintf(buff,"904%d2",i);
		Address dest = Address(strlen(buff),0,0,buff);
		SmeProxy* proxy;
		fprintf(stderr,"try lookup 902%d1->904%d2\n",i,i);
		if ( !router.lookup(source,dest,proxy,0) )
		{
			fprintf(stderr,"can't lookup 902%d1->904%d2\n",i,i);
		}
		else
		{
			fprintf(stderr,"route 902%d1->904%d2 proxy %p\n",i,i,proxy);
		}
	}
}

};
};

#if !defined ( TESTUNIT_WITHOUT_MAIN )
int main(void)
{
	return smsc::router::TestUnit1();
}
#endif
