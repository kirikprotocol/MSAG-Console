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

inline void prepareForNewSme(vector<SmeInfo*>& sme, vector<TCResultStack*>& stack)
{
	sme.push_back(new SmeInfo());
	stack.push_back(new TCResultStack());
}

void executeFunctionalTest(TCResultFilter* filter, int listSize)
{
	SmeManagerTestCases tc(new SmeManager());
	vector<SmeInfo*> sme;
	vector<TCResultStack*> stack;

	log.debug("*** start ***");

	//����������� sme � ����������� �����������
	for (int i = 0; i < listSize; i++)
	{
		prepareForNewSme(sme, stack);
		TCResult* res = tc.addCorrectSme(sme.back(), RAND_TC);
		stack.back()->push_back(res);
	}

	//����������� sme � ������������� �����������, 1/6
	//����������� sme � ������ systemId, 1/6
	//Disable ������������������� sme, 1/6
	//Enable ������������������� sme, 1/6
	//��������� ������������������� sme, 2/6
	bool emptySystemId = false; //����� ������� ������ ������������ sme
	for (TCSelector s(RAND_SET_TC, 6); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.addIncorrectSme(*sme[i]);
					stack.back()->push_back(res);
				}
				break;
			case 3:
			case 4:
				if (!emptySystemId)
				{
					emptySystemId = true;
					prepareForNewSme(sme, stack);
					TCResult* res = tc.addCorrectSmeWithEmptySystemId(sme.back());
					stack.back()->push_back(res);
				}
				break;
			/*
			case 3:
				for (int i = 0; i < sme.size(); i++)
				{
					TCResult* res = tc.disableExistentSme(sme[i]);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				for (int i = 0; i < sme.size(); i++)
				{
					TCResult* res = tc.enableExistentSme(sme[i]);
					stack[i]->push_back(res);
				}
				break;
			*/
			default: //case = 5..6
				for (int i = 0; i < sme.size(); i++)
				{
					SmeProxy* proxy;
					TCResult* res = tc.getExistentSme(*sme[i], proxy);
					stack[i]->push_back(res);
				}
		}
	}

	//������� sme ���������� ����������
	for (int i = 0; i < sme.size(); i++)
	{
		SmeProxy* proxy;
		TCResult* res = tc.registerCorrectSmeProxy(sme[i]->systemId, &proxy);
		stack[i]->push_back(res);
	}
	//filter->addResult(tc.selectSme(sme, RAND_TC));

	//������������ �� ������ ������������������ sme
	filter->addResult(tc.iterateSme(sme));

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
	stack[0]->push_back(tc.deleteNonExistentSme());
	//stack[0]->push_back(tc.disableNonExistentSme());
	//stack[0]->push_back(tc.enableNonExistentSme());

	//������������ �� ������ ������������������ sme
	filter->addResult(tc.iterateSme(vector<SmeInfo*>()));

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
	/*
	cl.writeResult("Disable ������������������� sme",
		filter->getResults(TC_DISABLE_EXISTENT_SME));
	cl.writeResult("Disable ���������������������/��������������� sme",
		filter->getResults(TC_DISABLE_NON_EXISTENT_SME));
	cl.writeResult("Enable ������������������� sme",
		filter->getResults(TC_ENABLE_EXISTENT_SME));
	cl.writeResult("Enable ���������������������/��������������� sme",
		filter->getResults(TC_ENABLE_NON_EXISTENT_SME));
	*/
	cl.writeResult("��������� ������������������� sme",
		filter->getResults(TC_GET_EXISTENT_SME));
	cl.writeResult("��������� ���������������������/��������������� sme",
		filter->getResults(TC_GET_NON_EXISTENT_SME));
	cl.writeResult("������������ �� ������ ������������������ sme",
		filter->getResults(TC_ITERATE_SME));
	cl.writeResult("������� sme ���������� ����������",
		filter->getResults(TC_SELECT_SME));
}

/**
 * ��������� ������������ SmeManager �
 * ������� ��������� �� ���� ������ � checklist.
 */
int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		cout << "Usage: RouteManagerFunctionalTest <numCycles> <numSme>" << endl;
		exit(0);
	}

	const int numCycles = atoi(argv[1]);
	const int numSme = atoi(argv[2]);
	try
	{
		//Manager::init("config.xml");
		TCResultFilter* filter = new TCResultFilter();
		for (int i = 0; i < numCycles; i++)
		{
			executeFunctionalTest(filter, numSme);
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

