#include "AliasManagerTestCases.hpp"
#include "util/Logger.h"
#include "test/sms/SmsUtil.hpp"
#include "test/util/TCResultFilter.hpp"
#include "test/util/CheckList.hpp"

using log4cpp::Category;
using smsc::util::Logger;
using smsc::alias::AliasInfo;
using smsc::test::sms::SmsUtil;
using smsc::test::core::AliasRegistry;
using smsc::test::core::operator<<;
using namespace smsc::test::alias; //constants, AliasManagerTestCases
using namespace smsc::test::util; //TCResultFilter, CheckList

static Category& log = Logger::getCategory("AliasManagerFunctionalTest");

class AliasManagerFunctionalTest
{
	AliasManager* aliasMan;
	AliasRegistry* aliasReg;
	AliasManagerTestCases tc;
	vector<Address*> addr;
	//AliasRegistry aliasReg;
	vector<TCResultStack*> stack;

public:
	AliasManagerFunctionalTest()
		: aliasMan(new AliasManager()), aliasReg(new AliasRegistry()),
		tc(aliasMan, aliasReg) {}
	~AliasManagerFunctionalTest();
	void executeTest(TCResultFilter* filter, int numAddr);
	void printAliases();

private:
	void executeTestCases(const Address& alias, const Address& addr);
	void printAlias(const AliasInfo& aliasData);
};

AliasManagerFunctionalTest::~AliasManagerFunctionalTest()
{
	for (int i  = 0; i < addr.size(); i++)
	{
		delete addr[i];
	}
	for (int i = 0; i < stack.size(); i++)
	{
		delete stack[i];
	}
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

	//�������� ������ ����� ��� ���� alias, addr
	stack.push_back(new TCResultStack());

	//����������� ������ � ��������������� addr->alias � alias->addr, 1/3
	//����������� ������ � ��������������� ������ alias->addr, 1/3
	//����������� ������ � ��������������� ������ addr->alias, 1/3
	for (TCSelector s(RAND_SET_TC, 3); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				{
					AliasInfo aliasInfo;
					aliasInfo.alias = alias;
					aliasInfo.addr = addr;
					TCResult* res = tc.addCorrectAliasMatch(&aliasInfo, RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			case 2:
				{
					AliasInfo aliasInfo;
					aliasInfo.alias = alias;
					aliasInfo.addr = addr;
					TCResult* res = tc.addCorrectAliasNotMatchAddress(
						&aliasInfo, RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			case 3:
				{
					AliasInfo aliasInfo;
					aliasInfo.alias = alias;
					aliasInfo.addr = addr;
					TCResult* res = tc.addCorrectAliasNotMatchAlias(
						&aliasInfo, RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			/*
			case 4:
				{
					AliasInfo aliasInfo;
					aliasInfo.alias = alias;
					aliasInfo.addr = addr;
					TCResult* res = tc.addCorrectAliasException(&aliasInfo, RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			case 5:
			case 6:
				{
					TCResult* res = tc.findAliasByAddress(addr);
					stack.back()->push_back(res);
				}
				break;
			case 7:
			case 8:
				{
					TCResult* res = tc.findAddressByAlias(alias);
					stack.back()->push_back(res);
				}
				break;
			*/
		}
	}
}

void AliasManagerFunctionalTest::executeTest(
	TCResultFilter* filter, int numAddr)
{
	//���������� ������ �������
	for (int i = 0; i < numAddr; i++)
	{
		addr.push_back(new Address());
		SmsUtil::setupRandomCorrectAddress(addr.back());
	}

	//�� ������ ���� ������� ����������� ���������� ���������� �������
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
	
	//����� ������ � ������
	for (int i = 0; i < numAddr; i++)
	{
		TCResult* res1 = tc.findAliasByAddress(*addr[i]);
		TCResult* res2 = tc.findAddressByAlias(*addr[i]);
		TCResult* res3 = tc.checkInverseTransformation(*addr[i]);
		filter->addResult(res1);
		filter->addResult(res2);
		filter->addResult(res3);
		delete res1;
		delete res2;
		delete res3;
	}

	//������������ �� ������ ������������������ �������
    //filter->addResult(tc.iterateAliases());

	//��������� ������� �������
    filter->addResult(tc.deleteAliases());

	//����� ������ � ������ ��� ������ ���� �������
	for (int i = 0; i < numAddr; i++)
	{
		TCResult* res1 = tc.findAliasByAddress(*addr[i]);
		TCResult* res2 = tc.findAddressByAlias(*addr[i]);
		TCResult* res3 = tc.checkInverseTransformation(*addr[i]);
		filter->addResult(res1);
		filter->addResult(res2);
		filter->addResult(res3);
		delete res1;
		delete res2;
		delete res3;
	}

	//������������ �� ������ ������������������ �������
    //filter->addResult(tc.iterateAliases());

	//��������� �����������
	for (int i = 0; i < stack.size(); i++)
	{
		filter->addResultStack(*stack[i]);
	}

	//������� ������
	for (int i = 0; i < addr.size(); i++)
	{
		delete addr[i];
	}
	addr.clear();
	for (int i = 0; i < stack.size(); i++)
	{
		delete stack[i];
	}
	stack.clear();
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "���������� checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Alias Manager", "smsc::alias");
	//������������������ ���� �����
	cl.writeResult("����������� ������ � ��������������� addr->alias � alias->addr",
		filter->getResults(TC_ADD_CORRECT_ALIAS_MATCH));
	cl.writeResult("����������� ������ � ��������������� ������ alias->addr",
		filter->getResults(TC_ADD_CORRECT_ALIAS_NOT_MATCH_ADDRESS));
	cl.writeResult("����������� ������ � ��������������� ������ addr->alias",
		filter->getResults(TC_ADD_CORRECT_ALIAS_NOT_MATCH_ALIAS));
	/*
	cl.writeResult("����������� ������ � ������������� ������ ��� alias->addr ��� ������ ��� addr->alias",
		filter->getResults(TC_ADD_CORRECT_ALIAS_EXCEPTION));
	cl.writeResult("����������� ������ � ������������� �����������",
		filter->getResults(TC_ADD_INCORRECT_ALIAS));
	*/
	cl.writeResult("��������� ������� �������",
		filter->getResults(TC_DELETE_ALIASES));
	cl.writeResult("����� ������ �� ������",
		filter->getResults(TC_FIND_ALIAS_BY_ADDRESS));
	cl.writeResult("����� ������ �� ������",
		filter->getResults(TC_FIND_ADDRESS_BY_ALIAS));
	cl.writeResult("�������������� addr->alias->addr � alias->addr->alias �����������",
		filter->getResults(TC_CHECK_INVERSE_TRANSFORMATION));
	cl.writeResult("������������ �� ������ ������������������ �������",
		filter->getResults(TC_ITERATE_ALIASES));
}

/**
 * ��������� ������������ AliasManager �
 * ������� ��������� �� ���� ������ � checklist.
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
		TCResultFilter* filter = new TCResultFilter();
		for (int i = 0; i < numCycles; i++)
		{
			AliasManagerFunctionalTest test;
			test.executeTest(filter, numAddr);
		}
		saveCheckList(filter);
		delete filter;
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

