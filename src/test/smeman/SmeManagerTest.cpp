#include "SmeManagerTestCases.hpp"
#include "test/util/Util.cpp"

using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, SmeManagerTestCases
using namespace smsc::smeman;

int main(int argc, char* argv[])
{
	try
	{
		//Manager::init("config.xml");
		SmeManager smeMan;
		SmeRegistry smeReg;
		SmeManagerTestCases tc(&smeMan, &smeReg, NULL);
		Address addr, addr2;
		SmeInfo sme, sme2;
/*
addCorrectSme(4)->
enableExistentSme(1)->
enableExistentSme(1)->
getExistentSme(1)->
registerCorrectSmeProxy(1)->
deleteExistentSme(1){100}
*/

		tc.addCorrectSme(&addr, &sme, 4);
		SmeProxy* proxy;
		tc.registerCorrectSmeProxy(sme.systemId, &proxy);
		//tc.selectSme(v, 1);

/*
		tc.enableExistentSme(&sme);
		tc.disableExistentSme(&sme);
		tc.addCorrectSmeWithEmptySystemId(&sme2);
		tc.registerCorrectSmeProxy(sme.systemId, &proxyId);
		tc.addIncorrectSme(sme);
		tc.deleteExistentSme(sme.systemId);
		tc.getNonExistentSme(sme.systemId, 1);
		tc.getNonExistentSme("", 2);
		tc.iterateSme(v);
		SmeProxy* proxy;
		tc.getExistentSme(sme, proxy);
		uint32_t proxyId;
		tc.registerCorrectSmeProxy(sme.systemId, &proxyId);
		v.push_back(&sme);
		tc.selectSme(v, 2);
*/		
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs";
	}
	return 0;
}

