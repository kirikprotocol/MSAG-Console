#include "AliasManagerTestCases.hpp"
#include "util/Logger.h"
#include "test/sms/SmsUtil.hpp"
#include "test/util/TCResultFilter.hpp"
#include "test/util/CheckList.hpp"

using log4cpp::Category;
using smsc::util::Logger;
using smsc::alias::AliasInfo;
using smsc::test::sms::SmsUtil;
using namespace smsc::test::alias; //constants, AliasManagerTestCases
using namespace smsc::test::util; //TCResultFilter, CheckList

static Category& log = Logger::getCategory("AliasManagerFunctionalTest");

class AliasManagerFunctionalTest
{
	AliasManagerTestCases tc;
	vector<Address*> addr;
	AliasRegistry aliasReg;
	vector<TCResultStack*> stack;

public:
	~AliasManagerFunctionalTest();
	void executeTest(TCResultFilter* filter, int numAddr);
	void printAliases();

private:
	TestAliasData prepareForNewAlias(const Address& alias, const Address& addr);
	void executeTestCases(const Address& alias, const Address& addr);
	void printAlias(const TestAliasData* aliasData);
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
}

void AliasManagerFunctionalTest::printAlias(const TestAliasData* aliasData)
{
	log.debugStream() << *aliasData;
}

void AliasManagerFunctionalTest::printAliases()
{
	/*
	RouteRegistry::RouteIterator* it = routeReg.iterator();
	for (; it->hasNext(); (*it)++)
	{
		printRoute(**it);
	}
	delete it;
	*/
}

void AliasManagerFunctionalTest::executeTestCases(
	const Address& alias, const Address& addr)
{
	log.debug("*** start ***");

	//�������� ������ ����� ��� ���� alias, addr
	stack.push_back(new TCResultStack());

	//����������� ������ � ��������������� addr -> alias � alias -> addr, 1/8
	//����������� ������ � ��������������� ������ alias -> addr, 1/8
	//����������� ������ � ��������������� ������ addr -> alias, 1/8
	//����������� ������ � ������������� ������ ��� alias -> addr ���
	//	������ ��� addr -> alias, 1/8
	//����� ������ �� ������, 2/8
	//����� ������ �� ������, 2/8
	for (TCSelector s(RAND_SET_TC, 8); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				{
					TestAliasData aliasData(alias, addr);
					TCResult* res = tc.addCorrectAliasMatch(&aliasData, RAND_TC);
					stack.back()->push_back(res);
					aliasReg.putAlias(aliasData);
				}
				break;
			case 2:
				{
					TestAliasData aliasData(alias, addr);
					TCResult* res = tc.addCorrectAliasNotMatchAddress(
						&aliasData, RAND_TC);
					stack.back()->push_back(res);
					aliasReg.putAlias(aliasData);
				}
				break;
			case 3:
				{
					TestAliasData aliasData(alias, addr);
					TCResult* res = tc.addCorrectAliasNotMatchAlias(
						&aliasData, RAND_TC);
					stack.back()->push_back(res);
					aliasReg.putAlias(aliasData);
				}
				break;
			case 4:
				{
					TestAliasData aliasData(alias, addr);
					TCResult* res = tc.addCorrectAliasException(&aliasData, RAND_TC);
					stack.back()->push_back(res);
					aliasReg.putAlias(aliasData);
				}
				break;
			case 5:
			case 6:
				{
					TCResult* res = tc.findAliasByAddress(aliasReg, addr);
					stack.back()->push_back(res);
				}
				break;
			case 7:
			case 8:
				{
					TCResult* res = tc.findAddressByAlias(aliasReg, alias);
					stack.back()->push_back(res);
				}
				break;
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
		for (int j = i; j < numAddr; j++)
		{
			Address& origAlias = *addr[i];
			Address& origAddr = *addr[j];
			executeTestCases(origAlias, origAddr);
		}
	}
	
	//����� ������ � ������ ��� ������ ���� �������
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = i; j < numAddr; j++)
		{
			Address& origAlias = *addr[i];
			Address& origAddr = *addr[j];
			filter->addResult(tc.findAliasByAddress(aliasReg, origAddr));
			filter->addResult(tc.findAddressByAlias(aliasReg, origAlias));
		}
	}

	//������������ �� ������ ������������������ �������
    filter->addResult(tc.iterateAliases(aliasReg));

	//��������� ������� �������
    filter->addResult(tc.deleteAliases());
	aliasReg.clear();

	//����� ������ � ������ ��� ������ ���� �������
	for (int i = 0; i < numAddr; i++)
	{
		for (int j = i; j < numAddr; j++)
		{
			Address& origAlias = *addr[i];
			Address& origAddr = *addr[j];
			filter->addResult(tc.findAliasByAddress(aliasReg, origAddr));
			filter->addResult(tc.findAddressByAlias(aliasReg, origAlias));
		}
	}

	//������������ �� ������ ������������������ �������
    filter->addResult(tc.iterateAliases(aliasReg));

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
	for (int i = 0; i < stack.size(); i++)
	{
		delete stack[i];
	}
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "���������� checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Alias Manager", "smsc::alias");
	//������������������ ���� �����
	cl.writeResult("����������� ������ � ��������������� addr -> alias � alias -> addr",
		filter->getResults(TC_ADD_CORRECT_ALIAS_MATCH));
	cl.writeResult("����������� ������ � ��������������� ������ alias -> addr",
		filter->getResults(TC_ADD_CORRECT_ALIAS_NOT_MATCH_ADDRESS));
	cl.writeResult("����������� ������ � ��������������� ������ addr -> alias",
		filter->getResults(TC_ADD_CORRECT_ALIAS_NOT_MATCH_ALIAS));
	cl.writeResult("����������� ������ � ������������� ������ ��� alias -> addr ��� ������ ��� addr -> alias",
		filter->getResults(TC_ADD_CORRECT_ALIAS_EXCEPTION));
	cl.writeResult("����������� ������ � ������������� �����������",
		filter->getResults(TC_ADD_INCORRECT_ALIAS));
	cl.writeResult("��������� ������� �������",
		filter->getResults(TC_DELETE_ALIASES));
	cl.writeResult("����� ������ �� ������",
		filter->getResults(TC_FIND_ALIAS_BY_ADDRESS));
	cl.writeResult("����� ������ �� ������",
		filter->getResults(TC_FIND_ADDRESS_BY_ALIAS));
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
			test.printAliases();
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

