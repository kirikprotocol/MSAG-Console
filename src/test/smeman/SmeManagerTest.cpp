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
		SmeManagerTestCases tc(&smeMan);
		SmeInfo sme, sme2;
		vector<SmeInfo*> v;
/*
addCorrectSme(4)->
enableExistentSme(1)->
enableExistentSme(1)->
getExistentSme(1)->
registerCorrectSmeProxy(1)->
deleteExistentSme(1){100}
*/

		cout << *tc.addCorrectSme(&sme, 4) << endl;
		SmeProxy* proxy;
		cout << *tc.registerCorrectSmeProxy(sme.systemId, &proxy) << endl;
		v.push_back(&sme);
		//cout << *tc.selectSme(v, 1) << endl;

/*
		cout << *tc.enableExistentSme(&sme) << endl;
		cout << *tc.disableExistentSme(&sme) << endl;
		cout << *tc.addCorrectSmeWithEmptySystemId(&sme2) << endl;
		cout << *tc.registerCorrectSmeProxy(sme.systemId, &proxyId) << endl;
		cout << *tc.addIncorrectSme(sme) << endl;
		cout << *tc.deleteExistentSme(sme.systemId) << endl;
		cout << *tc.getNonExistentSme(sme.systemId, 1) << endl;
		cout << *tc.getNonExistentSme("", 2) << endl;
		cout << *tc.iterateSme(v) << endl;
		SmeProxy* proxy;
		cout << *tc.getExistentSme(sme, proxy) << endl;
		uint32_t proxyId;
		cout << *tc.registerCorrectSmeProxy(sme.systemId, &proxyId) << endl;
		v.push_back(&sme);
		cout << *tc.selectSme(v, 2) << endl;
*/		
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}
	return 0;
}

