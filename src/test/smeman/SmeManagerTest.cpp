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
		SmeManagerTestCases tc;
		SmeInfo sme;
		cout << *tc.addCorrectSme(&sme, 1) << endl;
		cout << *tc.deleteExistentSme(sme.systemId) << endl;
		cout << *tc.getNonExistentSme(sme.systemId, 1) << endl;
		cout << *tc.getNonExistentSme("", 2) << endl;
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}
	return 0;
}

