#include "ProfilerTestCases.hpp"
#include "util/config/Manager.h"
#include "test/sms/SmsUtil.hpp"
#include "ProfilerCheckList.hpp"
#include "core/threads/ThreadPool.hpp"

using smsc::profiler::Profile;
using smsc::test::sms::SmsUtil;
using smsc::core::threads::ThreadPool;
using namespace smsc::test::profiler;
using namespace smsc::test::util;
using namespace std;

void executeTest(ProfilerCheckList* chkList)
{
	Profile defProfile;
	ProfilerTestCases::setupRandomCorrectProfile(defProfile);

	ThreadPool threadPool;
	Profiler* profiler = new Profiler(defProfile);
	profiler->loadFromDB();
	threadPool.startTask(profiler);

	ProfileRegistry profileReg(defProfile);
	ProfilerTestCases tc(profiler, &profileReg, chkList);

	Address addr;
	SmsUtil::setupRandomCorrectAddress(&addr);
	tc.createProfileNotMatch(addr, 4);
	tc.updateProfile(addr);
	tc.lookup(addr);
	//tc.putCommand(addr, RAND_TC);
	//tc.createProfileMatch(addr, RAND_TC);
	//tc.createProfileNotMatch(addr, RAND_TC);
	//tc.updateProfile(addr);
}

int main(int argc, char* argv[])
{
	smsc::util::config::Manager::init("config.xml");
	try
	{
		//Manager::init("config.xml");
		ProfilerCheckList chkList;
		executeTest(&chkList);
		//chkList.saveHtml();
	}
	catch (const char*)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

