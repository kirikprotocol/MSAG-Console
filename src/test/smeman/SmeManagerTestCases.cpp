#include "SmeManagerTestCases.hpp"
#include "SmeManagerUtil.hpp"
#include "smeman/smetypes.h"
#include "util/debug.h"
#include <map>
#include <sstream>

namespace smsc {
namespace test {
namespace smeman {

using smsc::util::Logger;
using smsc::test::sms::SmsUtil;
using namespace std;
using namespace smsc::test::util;

ostream& operator<< (ostream& os, const SmeInfo& sme)
{
	os << "systemId = " << sme.systemId << "(" << sme.systemId.length() << ")";
	os << ", disabled = " << (sme.disabled ? "true" : "false");
	os << ", interfaceVersion = " << (int) sme.interfaceVersion;
	os << ", systemType = " << sme.systemType;
	os << ", hostname = " << sme.hostname;
	os << ", port = " << sme.port;
}

SmeManagerTestCases::SmeManagerTestCases(SmeManager* _smeMan, SmeRegistry* _smeReg)
	: smeMan(_smeMan), smeReg(_smeReg)
{
	//__require__(smeMan);
	__require__(smeReg);
}

Category& SmeManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmeManagerTestCases");
	return log;
}

void SmeManagerTestCases::debugSme(const char* tc, SmeInfo& sme)
{
	ostringstream os;
	os << sme << endl;
	getLog().debug("[%d]\t%s(): %s", thr_self(), tc, os.str().c_str());
	__trace2__("%s(): %s", tc, os.str().c_str());
}

void SmeManagerTestCases::setupRandomCorrectSmeInfo(SmeInfo* sme)
{
	//пункт 4.1
	sme->typeOfNumber = 0x0;
	sme->numberingPlan = 0x0;
	sme->interfaceVersion = 0x34;
	//addressRange
	auto_ptr<char> _addressRange = rand_char(rand1(MAX_ADDRESS_RANGE_LENGTH));
	sme->rangeOfAddress = _addressRange.get();
	//systemType
	auto_ptr<char> _systemType = rand_char(MAX_SYSTEM_TYPE_LENGTH);
	sme->systemType = _systemType.get();
	//password
	auto_ptr<char> _password = rand_char(rand1(MAX_PASSWORD_LENGTH));
	sme->password = _password.get();
	//hostname & port
	sme->hostname = "localhost";
	sme->port = rand1(65535);
	//systemId
	auto_ptr<char> _systemId = rand_char(MAX_SYSTEM_ID_LENGTH);
	sme->systemId = _systemId.get();
	sme->SME_N = rand0(INT_MAX); //реально не используется
	sme->disabled = !rand0(3); //реально не используется
}

vector<int> SmeManagerTestCases::compareSmeInfo(const SmeInfo& sme1,
	const SmeInfo& sme2)
{
	vector<int> res;
	if (sme1.typeOfNumber != sme2.typeOfNumber)
	{
		res.push_back(1);
	}
	if (sme1.numberingPlan != sme2.numberingPlan)
	{
		res.push_back(2);
	}
	if (sme1.interfaceVersion != sme2.interfaceVersion)
	{
		res.push_back(3);
	}
	if (sme1.rangeOfAddress != sme2.rangeOfAddress)
	{
		res.push_back(4);
	}
	if (sme1.systemType != sme2.systemType)
	{
		res.push_back(5);
	}
	if (sme1.password != sme2.password)
	{
		res.push_back(6);
	}
	if (sme1.hostname != sme2.hostname)
	{
		res.push_back(7);
	}
	if (sme1.port != sme2.port)
	{
		res.push_back(8);
	}
	if (sme1.systemId != sme2.systemId)
	{
		res.push_back(9);
	}
	if (sme1.SME_N != sme2.SME_N)
	{
		res.push_back(10);
	}
	if (sme1.disabled != sme2.disabled)
	{
		res.push_back(11);
	}
	return res;
}

TCResult* SmeManagerTestCases::addCorrectSme(Address* smeAddr, SmeInfo* sme, int num)
{
	TCSelector s(num, 11);
	TCResult* res = new TCResult(TC_ADD_CORRECT_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			SmsUtil::setupRandomCorrectAddress(smeAddr);
			setupRandomCorrectSmeInfo(sme);
			switch(s.value())
			{
				case 1: //ничего особенного
					break;
				case 2: //typeOfNumber вне диапазона
					sme->typeOfNumber = rand2(0x7, 0xff);
					break;
				case 3: //numberingPlan вне диапазона
					sme->numberingPlan = rand2(0x13, 0xff);
					break;
				case 4: //interfaceVersion вне диапазона
					sme->interfaceVersion = rand2(0x35, 0xff);
					break;
				case 5: //пустой addressRange
					sme->rangeOfAddress = "";
					break;
				case 6: //addressRange больше макс длины
					{
						auto_ptr<char> _addressRange =
							rand_char(MAX_ADDRESS_RANGE_LENGTH + 1);
						sme->rangeOfAddress = _addressRange.get();
					}
					break;
				case 7: //пустой systemType
					sme->systemType = "";
					break;
				case 8: //systemType больше макс длины
					{
						auto_ptr<char> _systemType =
							rand_char(MAX_SYSTEM_TYPE_LENGTH + 1);
						sme->systemType = _systemType.get();
					}
					break;
				case 9: //пустой password
					sme->password = "";
					break;
				case 10: //password больше макс длины
					{
						auto_ptr<char> _password =
							rand_char(MAX_PASSWORD_LENGTH + 1);
						sme->password = _password.get();
					}
					break;
				case 11: //systemId больше макс длины
					{
						auto_ptr<char> _systemId =
							rand_char(MAX_SYSTEM_ID_LENGTH + 1);
						sme->systemId = _systemId.get();
					}
					break;
				default:
					throw s;
			}
			smeReg->registerSme(*smeAddr, *sme);
			debugSme("addCorrectSme", *sme);
			if (smeMan)
			{
				smeMan->addSme(*sme);
			}
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

TCResult* SmeManagerTestCases::addCorrectSmeWithEmptySystemId(Address* smeAddr,
	SmeInfo* sme)
{
	TCResult* res = new TCResult(TC_ADD_CORRECT_SME, 1001);
	try
	{
		SmsUtil::setupRandomCorrectAddress(smeAddr);
		setupRandomCorrectSmeInfo(sme);
		sme->systemId = "";
		smeReg->registerSme(*smeAddr, *sme);
		debugSme("addCorrectSmeWithEmptySystemId", *sme);
		if (smeMan)
		{
			smeMan->addSme(*sme);
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

TCResult* SmeManagerTestCases::addIncorrectSme(const SmeInfo& existentSme)
{
	__require__(smeMan);
	TCResult* res = new TCResult(TC_ADD_INCORRECT_SME);
	try
	{
		SmeInfo info;
		setupRandomCorrectSmeInfo(&info);
		info.systemId = existentSme.systemId;
		//проверка на правильность прочих параметров не производится
		smeMan->addSme(info);
		res->addFailure(101);
	}
	catch(...)
	{
		//ok
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::deleteExistentSme(const SmeSystemId& systemId)
{
	TCResult* res = new TCResult(TC_DELETE_EXISTENT_SME);
	try
	{
		smeReg->deleteSme(systemId);
		if (smeMan)
		{
			smeMan->deleteSme(systemId);
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

TCResult* SmeManagerTestCases::deleteNonExistentSme()
{
	__require__(smeMan);
	TCResult* res = new TCResult(TC_DELETE_NON_EXISTENT_SME);
	try
	{
		auto_ptr<char> _systemId = rand_char(MAX_SYSTEM_ID_LENGTH);
		smeMan->deleteSme(_systemId.get());
		res->addFailure(101);
	}
	catch(...)
	{
		//ok
	}
	debug(res);
	return res;
}

/*
TCResult* SmeManagerTestCases::disableExistentSme(SmeInfo* sme)
{
	TCResult* res = new TCResult(TC_DISABLE_EXISTENT_SME);
	try
	{
		smeMan->disableSme(sme->systemId);
		sme->disabled = true;
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
	catch(...)
	{
		//ok
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::enableExistentSme(SmeInfo* sme)
{
	TCResult* res = new TCResult(TC_ENABLE_EXISTENT_SME);
	try
	{
		smeMan->enableSme(sme->systemId);
		sme->disabled = false;
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
	catch(...)
	{
		//ok
	}
	debug(res);
	return res;
}
*/

TCResult* SmeManagerTestCases::getExistentSme(const SmeInfo& sme, SmeProxy* proxy)
{
	__require__(smeMan);
	TCResult* res = new TCResult(TC_GET_EXISTENT_SME);
	try
	{
		SmeIndex index = smeMan->lookup(sme.systemId);
		SmeInfo _sme = smeMan->getSmeInfo(index);
		vector<int> tmp = compareSmeInfo(sme, _sme);
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
	__require__(smeMan);
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
		catch(...)
		{
			//ok
		}
	}
	debug(res);
	return res;
}

TCResult* SmeManagerTestCases::iterateSme()
{
	__require__(smeMan);
	TCResult* res = new TCResult(TC_ITERATE_SME);
	try
	{
		int numSme = 0;
		typedef map<int, int> MismatchMap;
		MismatchMap mismatch;
		SmeIterator* iter = smeMan->iterator();
		while (iter->next())
		{
			numSme++;
			//SmeProxy* proxy = iter->getSmeProxy();
			//SmeIndex index = iter->getSmeIndex();
			SmeInfo smeInfo1 = iter->getSmeInfo();
			//debugSme(info);
			bool found = false;
			const SmeInfo* smeInfo2 = smeReg->getSme(smeInfo1.systemId);
			if (smeInfo2)
			{
				vector<int> tmp = compareSmeInfo(smeInfo1, *smeInfo2);
				for (int i = 0; i < tmp.size(); i++)
				{
					mismatch[tmp[i]]++;
				}
			}
			//итератор вернул лишние sme
			else
			{
				mismatch[101]++;
			}
		}
		delete iter;
		//итератор пропустил некоторые sme
		if (numSme != smeReg->size())
		{
			res->addFailure(102);
		}
		//перечислить отличия в sme
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

/*
void SmeManagerTestCases::setupRandomSmeProxy(const vector<SmeProxy*>& proxy)
{
	int num = rand0(proxy.size());
	for (int i = 0; i < num; i++)
	{
		int idx = rand0(proxy.size() - 1);
		if (!proxy[idx]->hasInput())
		{
			proxy[idx]->putCommand(SmscCommand());
		}
	}
}

void SmeManagerTestCases::checkSelectSmeStat(const vector<SmeInfo*>& sme,
	const map<uint32_t, int>& statMap, TCResult* res)
{
	int minVal = INT_MAX;
	int maxVal = 0;
	for (map<uint32_t, int>::const_iterator it = statMap.begin();
		 it != statMap.end(); it++)
	{
		if (minVal > it->second)
		{
			minVal = it->second;
			continue;
		}
		if (maxVal < it->second)
		{
			maxVal = it->second;
			continue;
		}
	}
	if (minVal > maxVal)
	{
		res->addFailure(1);
		return;
	}
	float tmp = (float) (maxVal - minVal) / (maxVal + minVal);
	if (tmp > 0.1)
	{
		res->addFailure(2);
	}
	int enabledSmeCount = 0;
	for (int i = 0; i < sme.size(); i++)
	{
		if (!sme[i]->disabled)
		{
			enabledSmeCount++;
		}
	}
	if (enabledSmeCount != statMap.size())
	{
		res->addFailure(3);
	}
}

TCResult* SmeManagerTestCases::selectSme(const vector<SmeInfo*>& sme,
	const vector<SmeProxy*>& proxy, int num)
{
	TCSelector s(num, 2);
	TCResult* res = new TCResult(TC_SELECT_SME, s.getChoice());
	for (; s.check(); s++)
	{
		try
		{
			unsigned long timeout;
			switch (s.value())
			{
				case 1:
					timeout = 0;
					break;
				case 2:
					timeout = 100 * rand1(5);
					break;
				default:
					throw s;
			}
			map<uint32_t, int> statMap;
			for (int i = 0; i < 100; i++)
			{
				SmeProxy* _proxy = smeMan->selectSmeProxy(timeout);
				if (_proxy)
				{
					_proxy->getCommand();
					statMap[_proxy->getUniqueId()]++;
				}
				setupRandomSmeProxy(proxy);
			}
			checkSelectSmeStat(sme, statMap, res);
		}
		catch (...)
		{
			error();
			res->addFailure(100);
		}
	}
	return res;
}
*/

TCResult* SmeManagerTestCases::registerCorrectSmeProxy(const SmeSystemId& systemId,
	SmeProxy** proxy)
{
	__require__(smeMan);
	TCResult* res = new TCResult(TC_REGISTER_CORRECT_SME_PROXY);
	try
	{
		CorrectSmeProxy* tmp = new CorrectSmeProxy();
		smeMan->registerSmeProxy(systemId, tmp);
		smeReg->bindSme(systemId);
		SmeIndex index = smeMan->lookup(systemId);
		*proxy = smeMan->getSmeProxy(index);
		getLog().debug("[%d]\tregisterCorrectSmeProxy(): systemId = %s, proxyId = %u",
			thr_self(), systemId.c_str(), (*proxy)->getUniqueId());
		__trace2__("registerCorrectSmeProxy(): systemId = %s, proxyId = %u",
			systemId.c_str(), (*proxy)->getUniqueId());
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

