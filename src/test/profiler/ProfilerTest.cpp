#include "ProfilerTestCases.hpp"
#include "test/core/ProfileUtil.hpp"
#include "util/config/Manager.h"
#include "test/sms/SmsUtil.hpp"
#include "ProfilerCheckList.hpp"
#include "core/threads/ThreadPool.hpp"

using smsc::profiler::Profile;
using smsc::test::sms::SmsUtil;
using smsc::test::core::ProfileUtil;
using smsc::core::threads::ThreadPool;
using smsc::util::config::Manager;
using namespace smsc::test::profiler;
using namespace smsc::test::util;
using namespace std;

void executeTest(ProfilerCheckList* chkList)
{
	Profile defProfile;
	ProfileUtil::setupRandomCorrectProfile(defProfile);

	ThreadPool threadPool;
	Profiler* profiler = new Profiler(defProfile);
	profiler->loadFromDB();
	threadPool.startTask(profiler);

	ProfileRegistry profileReg(defProfile);
	ProfilerTestCases tc(profiler, &profileReg, chkList);

	Address addr;
	SmsUtil::setupRandomCorrectAddress(&addr);
	tc.putCommand(addr, 8);
	sleep(5);
	tc.lookup(addr);
	//tc.putCommand(addr, RAND_TC);
	//tc.createProfileMatch(addr, RAND_TC);
	//tc.createProfileNotMatch(addr, RAND_TC);
	//tc.updateProfile(addr);
}

int main(int argc, char* argv[])
{
	try
	{
		Manager::init("config.xml");
		ProfilerCheckList chkList;
		executeTest(&chkList);
		chkList.saveHtml();
	}
	catch (const char*)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

