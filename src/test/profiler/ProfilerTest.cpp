#include "ProfilerTestCases.hpp"
#include "test/core/ProfileUtil.hpp"
#include "util/config/Manager.h"
#include "test/sms/SmsUtil.hpp"
#include "ProfilerCheckList.hpp"
#include "core/threads/ThreadPool.hpp"

using smsc::profiler::Profile;
using smsc::smeman::ProxyMonitor;
using smsc::test::sms::SmsUtil;
using smsc::test::core::ProfileUtil;
using smsc::core::threads::ThreadPool;
using smsc::core::threads::ThreadedTask;
using smsc::util::config::Manager;
using namespace smsc::test::profiler;
using namespace smsc::test::util;
using namespace std;

class ProfilerMonitor : public ThreadedTask, public ProxyMonitor
{
	ProfilerTestCases* tc;
public:
	ProfilerMonitor(ProfilerTestCases* _tc) : tc(_tc) {}
	virtual int Execute()
	{
		while (true)
		{
			Wait();
			if (isStopping)
			{
				break;
			}
			__trace2__("ProfilerMonitor notified");
			tc->onCommand();
		}
	}
	virtual const char* taskName()
	{
		return "ProfilerMonitor";
	}
};

void executeTest(ProfilerCheckList* chkList)
{
	Profile defProfile;
	ProfileUtil::setupRandomCorrectProfile(defProfile);

	ThreadPool threadPool;
	Profiler* profiler = new Profiler(defProfile);
	profiler->loadFromDB();

	ProfileRegistry profileReg(defProfile);
	ProfilerTestCases tc(profiler, &profileReg, chkList);
	
	ProfilerMonitor* monitor = new ProfilerMonitor(&tc);
	profiler->attachMonitor(monitor);
	threadPool.startTask(monitor);
	threadPool.startTask(profiler);

	Address addr;
	SmsUtil::setupRandomCorrectAddress(&addr);
	tc.putCommand(addr, 8);
	sleep(1);
	tc.lookup(addr);
	//tc.putCommand(addr, RAND_TC);
	//tc.createProfileMatch(addr, RAND_TC);
	//tc.createProfileNotMatch(addr, RAND_TC);
	//tc.updateProfile(addr);

	monitor->stop();
	profiler->stop();
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

