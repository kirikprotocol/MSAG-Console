#include "AliasManagerTestCases.hpp"
#include "util/Logger.h"
#include "test/sms/SmsUtil.hpp"
#include "AliasManagerCheckList.hpp"
#include <sstream>

using log4cpp::Category;
using smsc::util::Logger;
using smsc::alias::AliasInfo;
using smsc::test::sms::SmsUtil;
using smsc::test::core::AliasRegistry;
using smsc::test::core::operator<<;
using namespace smsc::test::alias; //constants, AliasManagerTestCases, AliasManagerCheckList
using namespace smsc::test::util; //TCSelector, Deletor
using namespace std;

static Category& log = Logger::getCategory("AliasManagerFunctionalTest");

class AliasManagerFunctionalTest
{
	AliasManager* aliasMan;
	AliasRegistry* aliasReg;
	AliasManagerTestCases tc;
	vector<Address*> addr;
	//AliasRegistry aliasReg;

public:
	AliasManagerFunctionalTest(CheckList* chkList)
		: aliasMan(new AliasManager()), aliasReg(new AliasRegistry()),
		tc(aliasMan, aliasReg, chkList) {}
	~AliasManagerFunctionalTest();
	void executeTest(int numAddr);
	void printAliases();

private:
	void executeTestCases(const Address& alias, const Address& addr);
	void printAlias(const AliasInfo& aliasData);
};

AliasManagerFunctionalTest::~AliasManagerFunctionalTest()
{
	for_each(addr.begin(), addr.end(), Deletor<Address>());
	delete aliasMan;
	delete aliasReg;
}

void AliasManagerFunctionalTest::printAlias(const AliasInfo& alias)
{
	ostringstream os;
	os << alias;
	log.debug("[%d]\t%s", thr_self(), os.str().c_str());
}

void AliasManagerFunctionalTest::printAliases()
{
	log.debug("*** Begin of alias registry ***");
	AliasRegistry::AliasIterator* it = aliasReg->iterator();
	while (const AliasHolder* aliasHolder = it->next())
	{
		printAlias(aliasHolder->aliasInfo);
	}
	delete it;
	log.debug("*** End of alias registry ***");
}

void AliasManagerFunctionalTest::executeTestCases(
	const Address& alias, const Address& addr)
{
	log.debug("*** start ***");

	//–егистраци€ алиаса с преобразованием addr->alias и alias->addr, 1/3
	//–егистраци€ алиаса с преобразованием только alias->addr, 1/3
	//–егистраци€ алиаса с преобразованием только addr->alias, 1/3
	for (TCSelector s(RAND_SET_TC, 3); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				{
					AliasInfo aliasInfo;
					aliasInfo.alias = alias;
					aliasInfo.addr = addr;
					tc.addCorrectAliasMatch(&aliasInfo, RAND_TC);
				}
				break;
			case 2:
				{
					AliasInfo aliasInfo;
					aliasInfo.alias = alias;
					aliasInfo.addr = addr;
					tc.addCorrectAliasNotMatchAddress(&aliasInfo, RAND_TC);
				}
				break;
			case 3:
				{
					AliasInfo aliasInfo;
					aliasInfo.alias = alias;
					aliasInfo.addr = addr;
					tc.addCorrectAliasNotMatchAlias(&aliasInfo, RAND_TC);
				}
				break;
			/*
			case 4:
				{
					AliasInfo aliasInfo;
					aliasInfo.alias = alias;
					aliasInfo.addr = addr;
					tc.addCorrectAliasException(&aliasInfo, RAND_TC);
				}
				break;
			case 5:
			case 6:
				{
					tc.findAliasByAddress(addr);
				}
				break;
			case 7:
			case 8:
				{
					tc.findAddressByAlias(alias);
				}
				break;
			*/
		}
	}
}

void AliasManagerFunctionalTest::executeTest(int numAddr)
{
	//ѕодготовка списка адресов
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		SmsUtil::setupRandomCorrectAddress(addr.back());
	}

	//Ќа каждую пару адресов регистраци€ случайного количества алиасов
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = 0; j < numAddr; j++)
		{
			Address& origAlias = *addr[i];
			Address& origAddr = *addr[j];
			executeTestCases(origAlias, origAddr);
		}
	}
	tc.commit();
	printAliases();
	
	//ѕоиск алиаса и адреса
	for (int i = 0; i < numAddr; i++)
	{
		tc.findAliasByAddress(*addr[i]);
		tc.findAddressByAlias(*addr[i]);
		tc.checkInverseTransformation(*addr[i]);
	}

	//»терирование по списку зарегистрированных алиасов
    //tc.iterateAliases();

	//ќбнуление таблицы алиасов
    tc.deleteAliases();

	//ѕоиск алиаса и адреса дл€ каждой пары адресов
	for (int i = 0; i < numAddr; i++)
	{
		tc.findAliasByAddress(*addr[i]);
		tc.findAddressByAlias(*addr[i]);
		tc.checkInverseTransformation(*addr[i]);
	}

	//»терирование по списку зарегистрированных алиасов
    //tc.iterateAliases();

	//очистка пам€ти
	for_each(addr.begin(), addr.end(), Deletor<Address>());
	addr.clear();
}

/**
 * ¬ыполн€ет тестирование AliasManager и
 * выводит результат по тест кейсам в checklist.
 */
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: AliasManagerFunctionalTest <numCycles> <numAddr>" << endl;
		exit(0);
	}

	const int numCycles = atoi(argv[1]);
	const int numAddr = atoi(argv[2]);
	try
	{
		//Manager::init("config.xml");
		AliasManagerCheckList chkList;
		for (int i = 0; i < numCycles; i++)
		{
			AliasManagerFunctionalTest test(&chkList);
			test.executeTest(numAddr);
		}
		chkList.saveHtml();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

