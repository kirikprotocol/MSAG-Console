#include "SmeManagerTestCases.hpp"
#include "test/util/Util.cpp"
#include "test/util/TCResultFilter.hpp"
#include "test/util/CheckList.hpp"
#include "util/Logger.h"

using namespace smsc::test::util;
using namespace smsc::test::smeman; //constants, SmeManagerTestCases
using log4cpp::Category;
using smsc::util::Logger;
using smsc::smeman::SmeInfo;
using smsc::smeman::SmeProxy;
using smsc::test::util::CheckList;
static Category& log = Logger::getCategory("SmeManagerFunctionalTest");

void executeFunctionalTest(TCResultFilter* filter, int listSize)
{
	SmeManagerTestCases tc;
	vector<SmeInfo*> sme;
	vector<TCResultStack*> stack;

	log.debug("*** start ***");

	//����������� sme � ����������� �����������
	for (int i = 0; i < listSize; i++)
	{
		sme.push_back(new SmeInfo());
		stack.push_back(new TCResultStack());
		TCResult* res = tc.addCorrectSme(sme.back(), RAND_TC);
		stack.back()->push_back(res);
	}

	//����������� sme � ������������� �����������, 1/5
	//Disable ������������������� sme, 1/5
	//Enable ������������������� sme, 1/5
	//��������� ������������������� sme, 2/5
	for (TCSelector s(RAND_SET_TC, 5); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.addIncorrectSme(*sme[i]);
					stack.back()->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < sme.size(); i++)
				{
					TCResult* res = tc.disableExistentSme(sme[i]);
					stack[i]->push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < sme.size(); i++)
				{
					TCResult* res = tc.enableExistentSme(sme[i]);
					stack[i]->push_back(res);
				}
				break;
			default: //case = 4..5
				for (int i = 0; i < sme.size(); i++)
				{
					SmeProxy* proxy;
					TCResult* res = tc.getExistentSme(*sme[i], proxy);
					stack[i]->push_back(res);
				}
		}
	}

	//�������� ������������������� sme
	for (int i = 0; i < sme.size(); i++)
	{
		TCResult* res = tc.deleteExistentSme(sme[i]->systemId);
		stack[i]->push_back(res);
	}

	//��������� ���������������������/��������������� sme
	for (int i = 0; i < sme.size(); i++)
	{
		TCResult* res = tc.getNonExistentSme(sme[i]->systemId, RAND_TC);
		stack[i]->push_back(res);
	}

	//�������� ���������������������/��������������� sme
	//Disable ���������������������/��������������� sme
	//Enable ���������������������/��������������� sme
	TCResult* res1 = tc.deleteNonExistentSme();
	TCResult* res2 = tc.disableNonExistentSme();
	TCResult* res3 = tc.enableNonExistentSme();
	stack[0]->push_back(res1);
	stack[0]->push_back(res2);
	stack[0]->push_back(res3);

	//��������� �����������
	for (int i = 0; i < stack.size(); i++)
	{
		filter->addResultStack(*stack[i]);
	}

	//������� ������
	for (int i = 0; i < sme.size(); i++)
	{
		delete sme[i];
		delete stack[i];
	}
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "���������� checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("SME Manager", "smsc::smeman");
	//������������������ ���� �����
	cl.writeResult("����������� sme � ����������� �����������",
		filter->getResults(TC_ADD_CORRECT_SME));
	cl.writeResult("����������� sme � ������������� �����������",
		filter->getResults(TC_ADD_INCORRECT_SME));
	cl.writeResult("�������� ������������������� sme",
		filter->getResults(TC_DELETE_EXISTENT_SME));
	cl.writeResult("�������� ���������������������/��������������� sme",
		filter->getResults(TC_DELETE_NON_EXISTENT_SME));
	cl.writeResult("Disable ������������������� sme",
		filter->getResults(TC_DISABLE_EXISTENT_SME));
	cl.writeResult("Disable ���������������������/��������������� sme",
		filter->getResults(TC_DISABLE_NON_EXISTENT_SME));
	cl.writeResult("Enable ������������������� sme",
		filter->getResults(TC_ENABLE_EXISTENT_SME));
	cl.writeResult("Enable ���������������������/��������������� sme",
		filter->getResults(TC_ENABLE_NON_EXISTENT_SME));
	cl.writeResult("��������� ������������������� sme",
		filter->getResults(TC_GET_EXISTENT_SME));
	cl.writeResult("��������� ���������������������/��������������� sme",
		filter->getResults(TC_GET_NON_EXISTENT_SME));
}

/**
 * ��������� ������������ SmeManager �
 * ������� ��������� �� ���� ������ � checklist.
 */
int main(int argc, char* argv[])
{
	try
	{
		//Manager::init("config.xml");
		TCResultFilter* filter = new TCResultFilter();
		for (int i = 0; i < 100; i++)
		{
			executeFunctionalTest(filter, 1);
		}
		for (int i = 0; i < 100; i++)
		{
			executeFunctionalTest(filter, 5);
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

