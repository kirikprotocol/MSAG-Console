#include "SmeManagerTestCases.hpp"
#include "SmeManagerUtil.hpp"
#include "test/core/CoreTestManager.hpp"
#include "smeman/smetypes.h"
#include <map>
#include <sstream>

namespace smsc {
namespace test {
namespace smeman {

using namespace std;
using namespace smsc::smeman; //SmeIndex, SmeError, SmeIterator
using namespace smsc::test::util;
using smsc::util::Logger;
using test::core::CoreTestManager;

ostream& operator<< (ostream& os, const SmeInfo& sme)
{
	os << "systemId = " << sme.systemId << "(" << sme.systemId.length() << ")";
	os << ", disabled = " << (sme.disabled ? "true" : "false");
	os << ", interfaceVersion = " << sme.interfaceVersion;
	os << ", systemType = " << sme.systemType;
	os << ", hostname = " << sme.hostname;
	os << ", port = " << sme.port;
}

SmeManagerTestCases::SmeManagerTestCases()
	: smeMan(CoreTestManager::getSmeManager()) {}

Category& SmeManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmeManagerTestCases");
	return log;
}

void SmeManagerTestCases::debugSme(SmeInfo& sme)
{
	ostringstream os;
	os << sme << endl;
	getLog().debug("[%d]\t%s", thr_self(), os.str().c_str());
}

void SmeManagerTestCases::setupRandomCorrectSmeInfo(SmeInfo* info)
{
	//����� 4.1
	info->typeOfNumber = 0x0;
	info->numberingPlan = 0x0;
	info->interfaceVersion = 0x34;
	//addressRange
	auto_ptr<char> _addressRange = rand_char(rand1(MAX_ADDRESS_RANGE_LENGTH));
	info->rangeOfAddress = _addressRange.get();
	//systemType
	auto_ptr<char> _systemType = rand_char(rand1(MAX_SYSTEM_TYPE_LENGTH));
	info->systemType = _systemType.get();
	//password
	auto_ptr<char> _password = rand_char(rand1(MAX_PASSWORD_LENGTH));
	info->password = _password.get();
	//hostname & port
	info->hostname = "localhost";
	info->port = rand1(65535);
	//systemId
	auto_ptr<char> _systemId = rand_char(rand1(MAX_SYSTEM_ID_LENGTH));
	info->systemId = _systemId.get();
	info->SME_N = rand0(65535);
	info->disabled = rand0(1);
}

vector<int> SmeManagerTestCases::compareSmeInfo(const SmeInfo& info1,
	const SmeInfo& info2)
{
	vector<int> res;
	if (info1.typeOfNumber != info2.typeOfNumber)
	{
		res.push_back(1);
	}
	if (info1.numberingPlan != info2.numberingPlan)
	{
		res.push_back(2);
	}
	if (info1.interfaceVersion != info2.interfaceVersion)
	{
		res.push_back(3);
	}
	if (info1.rangeOfAddress != info2.rangeOfAddress)
	{
		res.push_back(4);
	}
	if (info1.systemType != info2.systemType)
	{
		res.push_back(5);
	}
	if (info1.password != info2.password)
	{
		res.push_back(6);
	}
	if (info1.hostname != info2.hostname)
	{
		res.push_back(7);
	}
	if (info1.port != info2.port)
	{
		res.push_back(8);
	}
	if (info1.systemId != info2.systemId)
	{
		res.push_back(9);
	}
	if (info1.SME_N != info2.SME_N)
	{
		res.push_back(10);
	}
	if (info1.disabled != info2.disabled)
	{
		res.push_back(11);
	}
	return res;
}

TCResult* SmeManagerTestCases::addCorrectSme(SmeInfo* info, int num)
{
	TCSelector s(num, 10);
	TCResult* res = new TCResult(TC_ADD_CORRECT_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			setupRandomCorrectSmeInfo(info);
			switch(s.value())
			{
				case 1: //typeOfNumber ��� ���������
					info->typeOfNumber = rand2(0x7, 0xff);
					break;
				case 2: //numberingPlan ��� ���������
					info->numberingPlan = rand2(0x13, 0xff);
					break;
				case 3: //interfaceVersion ��� ���������
					info->interfaceVersion = rand2(0x35, 0xff);
					break;
				case 4: //������ addressRange
					info->rangeOfAddress = "";
					break;
				case 5: //addressRange ������ ���� �����
					{
						auto_ptr<char> _addressRange =
							rand_char(MAX_ADDRESS_RANGE_LENGTH + 1);
						info->rangeOfAddress = _addressRange.get();
					}
					break;
				case 6: //������ systemType
					info->systemType = "";
					break;
				case 7: //systemType ������ ���� �����
					{
						auto_ptr<char> _systemType =
							rand_char(MAX_SYSTEM_TYPE_LENGTH + 1);
						info->systemType = _systemType.get();
					}
					break;
				case 8: //������ password
					info->password = "";
					break;
				case 9: //password ������ ���� �����
					{
						auto_ptr<char> _password =
							rand_char(MAX_PASSWORD_LENGTH + 1);
						info->password = _password.get();
					}
					break;
				case 10: //systemId ������ ���� �����
					{
						auto_ptr<char> _systemId =
							rand_char(MAX_SYSTEM_ID_LENGTH + 1);
						info->systemId = _systemId.get();
					}
					break;
				default:
					throw s;
			}
			smeMan->addSme(*info);
		}
		catch(...)
		{
			error();
			res->addFailure(s.value());
		}
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::addCorrectSmeWithEmptySystemId(SmeInfo* info)
{
	TCResult* res = new TCResult(TC_ADD_CORRECT_SME, 1001);
	try
	{
		setupRandomCorrectSmeInfo(info);
		info->systemId = "";
		smeMan->addSme(*info);
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::addIncorrectSme(const SmeInfo& existentSme)
{
	TCResult* res = new TCResult(TC_ADD_INCORRECT_SME);
	try
	{
		SmeInfo info;
		setupRandomCorrectSmeInfo(&info);
		info.systemId = existentSme.systemId;
		//�������� �� ������������ ������ ���������� �� ������������
		smeMan->addSme(info);
		res->addFailure(101);
	}
	catch(SmeError&)
	{
		//ok
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::deleteExistentSme(const SmeSystemId& systemId)
{
	TCResult* res = new TCResult(TC_DELETE_EXISTENT_SME);
	try
	{
getLog().debug("deleteExistentSme(): systemId = %s", systemId.c_str());
		smeMan->deleteSme(systemId);
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::deleteNonExistentSme()
{
	TCResult* res = new TCResult(TC_DELETE_NON_EXISTENT_SME);
	try
	{
		auto_ptr<char> _systemId = rand_char(MAX_SYSTEM_ID_LENGTH);
		smeMan->deleteSme(_systemId.get());
		res->addFailure(101);
	}
	catch(SmeError&)
	{
		//ok
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::disableExistentSme(SmeInfo* info)
{
	TCResult* res = new TCResult(TC_DISABLE_EXISTENT_SME);
	try
	{
		smeMan->disableSme(info->systemId);
		info->disabled = true;
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::disableNonExistentSme()
{
	TCResult* res = new TCResult(TC_DISABLE_NON_EXISTENT_SME);
	try
	{
		auto_ptr<char> _systemId = rand_char(MAX_SYSTEM_ID_LENGTH);
		smeMan->disableSme(_systemId.get());
		res->addFailure(101);
	}
	catch(SmeError&)
	{
		//ok
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::enableExistentSme(SmeInfo* info)
{
	TCResult* res = new TCResult(TC_ENABLE_EXISTENT_SME);
	try
	{
		smeMan->enableSme(info->systemId);
		info->disabled = false;
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::enableNonExistentSme()
{
	TCResult* res = new TCResult(TC_ENABLE_NON_EXISTENT_SME);
	try
	{
		auto_ptr<char> _systemId = rand_char(MAX_SYSTEM_ID_LENGTH);
		smeMan->enableSme(_systemId.get());
		res->addFailure(101);
	}
	catch(SmeError&)
	{
		//ok
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::getExistentSme(const SmeInfo& info, SmeProxy* proxy)
{
	TCResult* res = new TCResult(TC_GET_EXISTENT_SME);
	try
	{
		SmeIndex index = smeMan->lookup(info.systemId);
		SmeInfo _info = smeMan->getSmeInfo(index);
		vector<int> tmp = compareSmeInfo(info, _info);
		for (int i = 0; i < tmp.size(); i++)
		{
			res->addFailure(tmp[i]);
		}
		proxy = smeMan->getSmeProxy(index);
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::getNonExistentSme(const SmeSystemId& systemId, int num)
{
	TCSelector s(num, 2);
	TCResult* res = new TCResult(TC_GET_NON_EXISTENT_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			switch(s.value())
			{
				case 1:
					smeMan->lookup(systemId);
					break;
				case 2:
					{
						auto_ptr<char> _systemId = rand_char(MAX_SYSTEM_ID_LENGTH);
						smeMan->lookup(_systemId.get());
					}
					break;
				default:
					throw s;
			}
			res->addFailure(101);
		}
		catch(SmeError&)
		{
			//ok
		}
		catch(...)
		{
			error();
			res->addFailure(100);
		}
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::iterateSme(const vector<SmeInfo*> sme)
{
	TCResult* res = new TCResult(TC_ITERATE_SME);
	try
	{
		SmeIterator* iter = smeMan->iterator();
		int foundSme = 0;
		int extraSme = 0;
		typedef map<int, int> MismatchMap;
		MismatchMap mismatch;
		while (iter->next())
		{
			//SmeProxy* proxy = iter->getSmeProxy();
			//SmeIndex index = iter->getSmeIndex();
			SmeInfo info = iter->getSmeInfo();
debugSme(info);
			bool found = false;
			for (int i = 0; i < sme.size(); i++)
			{
				if (sme[i]->systemId == info.systemId)
				{
					found = true; foundSme++;
					vector<int> tmp = compareSmeInfo(*sme[i], info);
					for (int j = 0; j < tmp.size(); j++)
					{
						mismatch[tmp[j]]++;
					}
					break;
				}
			}
			if (!found)
			{
				extraSme++;
			}
		}
		delete iter;
		//�������� ������ ������ sme
		if (extraSme)
		{
			res->addFailure(101);
		}
		//�������� ��������� ��������� sme
		if (foundSme != sme.size())
		{
			res->addFailure(102);
		}
		//����������� ������� � sme
		for (MismatchMap::iterator it = mismatch.begin(); it != mismatch.end(); it++)
		{
			if (it->second)
			{
				res->addFailure(it->first);
			}
		}
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::registerCorrectSmeProxy(const SmeSystemId& systemId)
{
	TCResult* res = new TCResult(TC_REGISTER_CORRECT_SME_PROXY);
	try
	{
		TestSmeProxy* proxy = new TestSmeProxy(systemId);
		smeMan->registerSmeProxy(systemId, proxy);
	}
	catch(...)
	{
		error();
		res->addFailure(100);
	}
	debug(res);
	return res;
}

}
}
}

