#include "ProfilerTestCases.hpp"
#include "test/sms/SmsUtil.hpp"
#include "test/core/ProfileUtil.hpp"
#include "util/Logger.h"
#include "util/config/Manager.h"
#include "ProfilerCheckList.hpp"
#include "core/threads/ThreadPool.hpp"
#include <sstream>

using log4cpp::Category;
using smsc::util::Logger;
using smsc::profiler::Profile;
using smsc::smeman::SmscCommand;
using smsc::core::threads::ThreadPool;
using smsc::test::sms::SmsUtil;
using namespace smsc::test::core; //ProfileRegistry, ProfileUtil
using namespace smsc::test::profiler; //ProfilerTestCases, ProfilerCheckList
using namespace smsc::test::util; //TCSelector, Deletor
using namespace std;

static Category& log = Logger::getCategory("ProfilerFunctionalTest");

class ProfilerFunctionalTest
{
	Profile defProfile;
	Profiler* profiler;
	ThreadPool threadPool;
	ProfileRegistry* profileReg;
	CheckList* chkList;
	ProfilerTestCases* tc;
	vector<Address> addr1;
	vector<Address> addr2;

public:
	ProfilerFunctionalTest(CheckList* chkList);
	~ProfilerFunctionalTest();
	void executeTest(int numAddr);

private:
	void reinit();
	void executeTestCases(const Address& address);
};

ProfilerFunctionalTest::ProfilerFunctionalTest(CheckList* _chkList)
	: profiler(NULL), tc(NULL), chkList(_chkList)
{
	ProfileUtil::setupRandomCorrectProfile(defProfile);
	profileReg = new ProfileRegistry(defProfile);
	reinit();
}

ProfilerFunctionalTest::~ProfilerFunctionalTest()
{
	//delete profiler;
	delete profileReg;
	delete tc;
}

void ProfilerFunctionalTest::executeTestCases(const Address& address)
{
	//�������� ������ ������� ���������������� �������� ������, 1/5
	//�������� ������ ������� ������������������ �������� ������, 1/5
	//���������� ��� ������������� �������, 1/5
	//����� ������� ��� ��������� ������, 2/5
	Address addr;
	bool created = false;
	for (TCSelector s(RAND_SET_TC, 3); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				addr = address;
				tc->createProfileMatch(addr, RAND_TC);
				created = true;
				break;
			case 2:
				addr = address;
				tc->createProfileNotMatch(addr, RAND_TC);
				created = true;
				break;
			/*
			case 3:
				if (created)
				{
					//update ������ ��� ������� ������, ��� ����� ������
					tc->updateProfile(addr);
				}
				break;
			*/
			default:
				tc->lookup(address);
		}
	}
}

void ProfilerFunctionalTest::reinit()
{
	//����������� profiler � ���������, ��� ��� ��������� ����������� �
	//�������� �� ��. ��� profileReg �������� ��� �� �����
	if (profiler)
	{
		profiler->stop();
		//profiler->putCommand(SmscCommand()); //����� ��������� stop()
		//delete profiler;
	}
	profiler = new Profiler(defProfile);
	profiler->loadFromDB();
	threadPool.startTask(profiler);
	if (tc)
	{
		delete tc;
	}
	tc = new ProfilerTestCases(profiler, profileReg, chkList);
}

void ProfilerFunctionalTest::executeTest(int numAddr)
{
	//���������� ������ �������
	addr1.resize(numAddr);
	addr2.resize(numAddr);
	for (int i = 0; i < numAddr; i++)
	{
		SmsUtil::setupRandomCorrectAddress(&addr1[i]);
		SmsUtil::setupRandomCorrectAddress(&addr2[i]);
	}
	//�� ������ ����� ����������� ���������� ���������� ��������
	for (int i = 0; i < numAddr; i++)
	{
		executeTestCases(addr1[i]);
		tc->putCommand(addr2[i], RAND_TC);
		tc->lookup(addr2[i]);
	}
	reinit();
	//�� ������ ����� ����������� ���������� ���������� ��������
	for (int i = 0; i < numAddr; i++)
	{
		executeTestCases(addr1[i]);
		tc->putCommand(addr2[i], RAND_TC);
		tc->lookup(addr2[i]);
	}
	reinit();
	//����� ��������
	for (int i = 0; i < numAddr; i++)
	{
		tc->lookup(addr1[i]);
		tc->lookup(addr2[i]);
	}
	//������� ������
	addr1.clear();
	addr2.clear();
}

/**
 * ��������� ������������ Profile �
 * ������� ��������� �� ���� ������ � checklist.
 */
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: ProfilerFunctionalTest <numCycles> <numAddr>" << endl;
		exit(0);
	}

	const int numCycles = atoi(argv[1]);
	const int numAddr = atoi(argv[2]);
	smsc::util::config::Manager::init("config.xml");
	try
	{
		//Manager::init("config.xml");
		ProfilerCheckList chkList;
		for (int i = 0; i < numCycles; i++)
		{
			ProfilerFunctionalTest test(&chkList);
			test.executeTest(numAddr);
		}
		__trace__("Before save checklist");
		chkList.saveHtml();
	}
	catch (const char*)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

