#include "SmeManagerTestCases.hpp"
#include "SmeManagerUtil.hpp"
#include "test/sms/SmsUtil.hpp"
#include "smeman/smetypes.h"
#include "util/debug.h"
#include <map>
#include <sstream>

namespace smsc {
namespace test {
namespace smeman {

using smsc::util::Logger;
using smsc::test::sms::SmsUtil;
using smsc::test::sms::operator<<;
using namespace std;
using namespace smsc::test::util;

ostream& operator<< (ostream& os, const SmeInfo& sme)
{
	os << "{systemId = " << sme.systemId << "(" << sme.systemId.length() << ")";
	os << ", disabled = " << (sme.disabled ? "true" : "false");
	os << ", interfaceVersion = " << (int) sme.interfaceVersion;
	os << ", systemType = " << sme.systemType;
	os << ", hostname = " << sme.hostname;
	os << ", port = " << sme.port << "}";
}

SmeManagerTestCases::SmeManagerTestCases(SmeManager* _smeMan, SmeRegistry* _smeReg,
	CheckList* _chkList)
	: smeMan(_smeMan), smeReg(_smeReg), chkList(_chkList)
{
	//__require__(smeMan);
	__require__(smeReg);
	//__require__(chkList);
}

Category& SmeManagerTestCases::getLog()
{
	static Category& log = Logger::getCategory("SmeManagerTestCases");
	return log;
}

void SmeManagerTestCases::debugSme(const char* tc, const Address& smeAddr,
	const SmeInfo& sme)
{
	ostringstream os;
	os << "smeAddr = " << smeAddr << ", sme = " << sme;
	getLog().debug("[%d]\t%s(): %s", thr_self(), tc, os.str().c_str());
	//__trace2__("%s(): %s", tc, os.str().c_str());
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

#define __compare__(field, errCode) \
	if (sme1.field != sme2.field) { res.push_back(errCode); }

vector<int> SmeManagerTestCases::compareSmeInfo(const SmeInfo& sme1,
	const SmeInfo& sme2)
{
	vector<int> res;
	__compare__(typeOfNumber, 1);
	__compare__(numberingPlan, 2);
	__compare__(interfaceVersion, 3);
	__compare__(rangeOfAddress, 4);
	__compare__(systemType, 5);
	__compare__(password, 6);
	__compare__(hostname, 7);
	__compare__(port, 8);
	__compare__(systemId, 9);
	__compare__(SME_N, 10);
	__compare__(disabled, 11);
	return res;
}

void SmeManagerTestCases::addSme(const char* tc, int num,
	const Address* smeAddr, const SmeInfo* sme)
{
	if (smeReg->registerSme(*smeAddr, *sme))
	{
		debugSme(tc, *smeAddr, *sme);
		if (smeMan)
		{
			smeMan->addSme(*sme);
		}
	}
}

void SmeManagerTestCases::addCorrectSme(Address* smeAddr, SmeInfo* sme, int num)
{
	int numAddr = 3; int numSme = 11;
	TCSelector s(num, numAddr * numSme);
	__decl_tc__;
	for (; s.check(); s++)
	{
		try
		{
			SmsUtil::setupRandomCorrectAddress(smeAddr, 10, 20);
			switch (s.value1(numAddr))
			{
				case 1: //по умолчанию случайные ton и npi
					break;
				case 2: //ton = 1 и npi = 1 соответствует +7902986...
					smeAddr->setTypeOfNumber(1);
					smeAddr->setNumberingPlan(1);
					break;
				case 3: //ton = 2 и npi = 1 соответствует 8902986...
					smeAddr->setTypeOfNumber(2);
					smeAddr->setNumberingPlan(1);
					break;
				default:
					throw s;
			}
			setupRandomCorrectSmeInfo(sme);
			switch(s.value2(numAddr))
			{
				case 1: //ничего особенного
					__tc__("addCorrectSme");
					break;
				case 2: //typeOfNumber вне диапазона
					__tc__("addCorrectSme.invalidType");
					sme->typeOfNumber = rand2(0x7, 0xff);
					break;
				case 3: //numberingPlan вне диапазона
					__tc__("addCorrectSme.invalidPlan");
					sme->numberingPlan = rand2(0x13, 0xff);
					break;
				case 4: //interfaceVersion вне диапазона
					__tc__("addCorrectSme.invalidVersion");
					sme->interfaceVersion = rand2(0x35, 0xff);
					break;
				case 5: //пустой addressRange
					__tc__("addCorrectSme.invalidAddressRangeLength");
					sme->rangeOfAddress = "";
					break;
				case 6: //addressRange больше макс длины
					{
						__tc__("addCorrectSme.invalidAddressRangeLength");
						auto_ptr<char> _addressRange =
							rand_char(MAX_ADDRESS_RANGE_LENGTH + 1);
						sme->rangeOfAddress = _addressRange.get();
					}
					break;
				case 7: //пустой systemType
					__tc__("addCorrectSme.invalidSystemTypeLength");
					sme->systemType = "";
					break;
				case 8: //systemType больше макс длины
					{
						__tc__("addCorrectSme.invalidSystemTypeLength");
						auto_ptr<char> _systemType =
							rand_char(MAX_SYSTEM_TYPE_LENGTH + 1);
						sme->systemType = _systemType.get();
					}
					break;
				case 9: //пустой password
					__tc__("addCorrectSme.invalidPasswordLength");
					sme->password = "";
					break;
				case 10: //password больше макс длины
					{
						__tc__("addCorrectSme.invalidPasswordLength");
						auto_ptr<char> _password =
							rand_char(MAX_PASSWORD_LENGTH + 1);
						sme->password = _password.get();
					}
					break;
				case 11: //systemId больше макс длины
					{
						__tc__("addCorrectSme.invalidSystemIdLength");
						auto_ptr<char> _systemId =
							rand_char(MAX_SYSTEM_ID_LENGTH + 1);
						sme->systemId = _systemId.get();
					}
					break;
				default:
					__unreachable__("Invalid num");
			}
			addSme("addCorrectSme", s.value(), smeAddr, sme);
			__tc_ok__;
		}
		catch(...)
		{
			__tc_fail__(s.value());
			error();
		}
	}
}

void SmeManagerTestCases::addCorrectSmeWithEmptySystemId(Address* smeAddr,
	SmeInfo* sme)
{
	__decl_tc__;
	__tc__("addCorrectSme.systemIdEmpty");
	try
	{
		SmsUtil::setupRandomCorrectAddress(smeAddr, 10, 20);
		setupRandomCorrectSmeInfo(sme);
		sme->systemId = "";
		addSme("addCorrectSmeWithEmptySystemId", 1, smeAddr, sme);
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmeManagerTestCases::addIncorrectSme(const SmeInfo& existentSme)
{
	__require__(smeMan);
	__decl_tc__;
	__tc__("addIncorrectSme.invalidSystemId");
	try
	{
		SmeInfo info;
		setupRandomCorrectSmeInfo(&info);
		info.systemId = existentSme.systemId;
		//проверка на правильность прочих параметров не производится
		if (smeMan)
		{
			smeMan->addSme(info);
			__tc_fail__(101);
		}
	}
	catch(...)
	{
		__tc_ok__;
	}
}

void SmeManagerTestCases::deleteExistentSme(const SmeSystemId& systemId)
{
	__decl_tc__;
	__tc__("deleteSme.existentSme");
	try
	{
		smeReg->deleteSme(systemId);
		if (smeMan)
		{
			smeMan->deleteSme(systemId);
			__tc_ok__;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmeManagerTestCases::deleteNonExistentSme()
{
	__require__(smeMan);
	__decl_tc__;
	__tc__("deleteSme.nonExistentSme");
	try
	{
		auto_ptr<char> _systemId = rand_char(MAX_SYSTEM_ID_LENGTH);
		smeMan->deleteSme(_systemId.get());
		__tc_fail__(101);
	}
	catch(...)
	{
		__tc_ok__;
	}
}

/*
void SmeManagerTestCases::disableExistentSme(SmeInfo* sme)
{
	__decl_tc__;
	__tc__("disableExistentSme");
	try
	{
		smeMan->disableSme(sme->systemId);
		sme->disabled = true;
		__tc_ok__;
	}
	catch(...)
	{
		error();
		__tc_fail__(100);
	}
}

void SmeManagerTestCases::disableNonExistentSme()
{
	__decl_tc__;
	__tc__("disableNonExistentSme");
	try
	{
		auto_ptr<char> _systemId = rand_char(MAX_SYSTEM_ID_LENGTH);
		smeMan->disableSme(_systemId.get());
		__tc_fail__(101);
	}
	catch(...)
	{
		__tc_ok__;
	}
}

void SmeManagerTestCases::enableExistentSme(SmeInfo* sme)
{
	__decl_tc__;
	__tc__("enableExistentSme");
	try
	{
		smeMan->enableSme(sme->systemId);
		sme->disabled = false;
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmeManagerTestCases::enableNonExistentSme()
{
	__decl_tc__;
	__tc__("enableNonExistentSme");
	try
	{
		auto_ptr<char> _systemId = rand_char(MAX_SYSTEM_ID_LENGTH);
		smeMan->enableSme(_systemId.get());
		__tc_fail__(101);
	}
	catch(...)
	{
		__tc_ok__;
	}
}
*/

void SmeManagerTestCases::getExistentSme(const SmeInfo& sme, SmeProxy* proxy)
{
	__require__(smeMan);
	__decl_tc__;
	__tc__("getSme.existentSme");
	try
	{
		SmeIndex index = smeMan->lookup(sme.systemId);
		SmeInfo _sme = smeMan->getSmeInfo(index);
		proxy = smeMan->getSmeProxy(index);
		vector<int> tmp = compareSmeInfo(sme, _sme);
		if (tmp.size())
		{
			__tc_fail2__(tmp);
		}
		else
		{
			__tc_ok__;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

void SmeManagerTestCases::getNonExistentSme(const SmeSystemId& systemId, int num)
{
	__require__(smeMan);
	TCSelector s(num, 2);
	__decl_tc__;
	__tc__("getSme.nonExistentSme");
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
					__unreachable__("Invalid num");
			}
			__tc_fail__(101);
		}
		catch(...)
		{
			__tc_ok__;
		}
	}
}

void SmeManagerTestCases::iterateSme()
{
	__require__(smeMan);
	__decl_tc__;
	__tc__("iterateSme");
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
		bool flag = true;
		//итератор пропустил некоторые sme
		if (numSme != smeReg->size())
		{
			__tc_fail__(102);
			flag = false;
		}
		//перечислить отличия в sme
		for (MismatchMap::iterator it = mismatch.begin(); it != mismatch.end(); it++)
		{
			if (it->second)
			{
				__tc_fail__(it->first);
				flag = false;
			}
		}
		if (flag)
		{
			__tc_ok__;
		}
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
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
	const map<uint32_t, int>& statMap, void res)
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
		__tc_fail__(1);
		return;
	}
	float tmp = (float) (maxVal - minVal) / (maxVal + minVal);
	if (tmp > 0.1)
	{
		__tc_fail__(2);
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
		__tc_fail__(3);
	}
}

void SmeManagerTestCases::selectSme(const vector<SmeInfo*>& sme,
	const vector<SmeProxy*>& proxy, int num)
{
	TCSelector s(num, 2);
	void res = new TCResult(TC_SELECT_SME, s.getChoice());
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
					__unreachable__("Invalid num");
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
			__tc_fail__(100);
		}
	}
	return res;
}
*/

void SmeManagerTestCases::registerCorrectSmeProxy(const SmeSystemId& systemId,
	SmeProxy** proxy)
{
	__require__(smeMan);
	__decl_tc__;
	__tc__("registerSmeProxy");
	try
	{
		CorrectSmeProxy* tmp = new CorrectSmeProxy();
		smeMan->registerSmeProxy(systemId, tmp);
		smeReg->bindSme(systemId);
		SmeIndex index = smeMan->lookup(systemId);
		*proxy = smeMan->getSmeProxy(index);
		getLog().debug("[%d]\tregisterCorrectSmeProxy(): systemId = %s, proxyId = %u",
			thr_self(), systemId.c_str(), (*proxy)->getUniqueId());
		/*
		__trace2__("registerCorrectSmeProxy(): systemId = %s, proxyId = %u",
			systemId.c_str(), (*proxy)->getUniqueId());
		*/
		__tc_ok__;
	}
	catch(...)
	{
		__tc_fail__(100);
		error();
	}
}

}
}
}

