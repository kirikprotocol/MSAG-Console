#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>

using namespace std;
using namespace smsc::sms;
using namespace smsc::store;
using namespace smsc::test::store;
using namespace smsc::test::util;
using namespace smsc::util::config;

TCResultFilter* executeIntegrityTest()
{
	MessageStoreTestCases tc; //throws exception
    TCResultFilter* filter = new TCResultFilter();

	//������ SM, ����� ����� �����, �������, ����� ����� � ������
	cout << "Step 1" << endl;
	for (int i = 0; i < 100; i++)
	{
		SMSId id;
		SMS sms;
		TCResultStack stack;
		stack.push_back(tc.storeCorrectSM(&id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));

		stack.push_back(tc.replaceCorrectSM(id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));

        stack.push_back(tc.setCorrectSMStatus(id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));

		stack.push_back(tc.storeDuplicateSM(sms, RAND_TC));
		stack.push_back(tc.storeIncorrectSM(RAND_TC));
		stack.push_back(tc.replaceIncorrectSM(id, sms, RAND_TC));
		stack.push_back(tc.setIncorrectSMStatus(id));
		stack.push_back(tc.loadExistentSM(id, sms));

		stack.push_back(tc.deleteExistentSM(id));

		stack.push_back(tc.setNonExistentSMStatus(id, RAND_TC));
		stack.push_back(tc.deleteNonExistentSM(id, RAND_TC));
		stack.push_back(tc.replaceNonExistentSM(id, RAND_TC));
		stack.push_back(tc.loadNonExistentSM(id, RAND_TC));
		filter->addResultStack(stack);
	}
	
	//������� ������ ������, ����� �����, �������, ����� � ������
	const int listSize = 20;
	for (int j = 0; j < 3; j++)
	{
		cout << "Step " << (j + 2) << endl;
		SMSId id[listSize];
		SMS sms[listSize];
		TCResultStack stack[listSize];
		//�������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.storeCorrectSM(&id[i], &sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//������ SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadExistentSM(id[i], sms[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//������������ ��������� ������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.setIncorrectSMStatus(id[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//������������ ��������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.replaceIncorrectSM(id[i], sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//���������� �������������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.storeDuplicateSM(sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//������ SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadExistentSM(id[i], sms[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//��������� ����������� �������
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.setCorrectSMStatus(id[i], &sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//������ SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadExistentSM(id[i], sms[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//��������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.replaceCorrectSM(id[i], &sms[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//������ SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadExistentSM(id[i], sms[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//�������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.deleteExistentSM(id[i]);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//������ �������������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.loadNonExistentSM(id[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//��������� �������������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.replaceNonExistentSM(id[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//�������� �������������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.deleteNonExistentSM(id[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//��������� ������� �������������� SM
		for (int i = 0; i < listSize; i++)
		{
			TCResult* res = tc.setNonExistentSMStatus(id[i], RAND_TC);
			filter->addResult(res);
			stack[i].push_back(res);
			//delete res;
		}
		//��������� �����������
		for (int i = 0; i < listSize; i++)
		{
			filter->addResultStack(stack[i]);
		}
	}

	//������
	cout << "Step 3" << endl;
	filter->addResult(tc.storeIncorrectSM(ALL_TC));
#ifdef ASSERT_THROW_IF_FAIL
	filter->addResult(tc.storeAssertSM(ALL_TC));
#endif
	return filter;
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "���������� checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	//�������������������� test cases
	cl.writeResult("���������� ����������� SM",
		filter->getResults(TC_STORE_CORRECT_SM));
	cl.writeResult("���������� �������������� SM",
		filter->getResults(TC_STORE_DUPLICATE_SM));
	cl.writeResult("���������� ������������� SM",
		filter->getResults(TC_STORE_INCORRECT_SM));
	cl.writeResult("���������� ������������� SM, �������� �� assert",
		filter->getResults(TC_STORE_ASSERT_SM));
	cl.writeResult("������ ������������� SM",
		filter->getResults(TC_LOAD_EXISTENT_SM));
	cl.writeResult("������ ��������������� SM",
		filter->getResults(TC_LOAD_NON_EXISTENT_SM));
	cl.writeResult("�������� ������������� SM",
		filter->getResults(TC_DELETE_EXISTENT_SM));
	cl.writeResult("�������� ��������������� SM",
		filter->getResults(TC_DELETE_NON_EXISTENT_SM));
	cl.writeResult("���������� ������ ������������� SM",
		filter->getResults(TC_REPLACE_CORRECT_SM));
	cl.writeResult("������ ������������� SM ������������� �������",
		filter->getResults(TC_REPLACE_INCORRECT_SM));
	cl.writeResult("������ ��������������� SM",
		filter->getResults(TC_REPLACE_NON_EXISTENT_SM));
	
	//���� ��� ���������������������� test cases
	cl.writeResult("���������� ��������� ������� SM",
		filter->getResults(TC_SET_CORRECT_SM_STATUS));
	cl.writeResult("������������ ��������� ������� SM",
		filter->getResults(TC_SET_INCORRECT_SM_STATUS));
	cl.writeResult("��������� ������� ��������������� SM",
		filter->getResults(TC_SET_NON_EXISTENT_SM_STATUS));
	cl.writeResult("�������� ������������ SM ��������� �������� �� ������������ �����",
		filter->getResults(TC_DELETE_EXISTENT_WAITING_SM_BY_NUMBER));
	cl.writeResult("�������� �������������� SM ��������� �������� �� ������������ �����",
		filter->getResults(TC_DELETE_NON_EXISTENT_WAITING_SM_BY_NUMBER));
	cl.writeResult("�������� ��������� ������ SM ��������� �������� �� ������������ �����",
		filter->getResults(TC_LOAD_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER));
	cl.writeResult("�������� ������� ������ SM ��������� �������� �� ������������ �����",
		filter->getResults(TC_LOAD_NON_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER));
	cl.writeResult("�������� ��������� ������ SM ������������ �� ������������ �����",
		filter->getResults(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER));
	cl.writeResult("�������� ������� ������ SM ������������ �� ������������ �����",
		filter->getResults(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER));
	cl.writeResult("�������� ��������� ������ SM ������������ � ������������� ������",
		filter->getResults(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER));
	cl.writeResult("�������� ������� ������ SM ������������ � ������������� ������",
		filter->getResults(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER));
	cl.writeResult("�������� �������� ���������� ������ ���������� ���������",
		filter->getResults(TC_GET_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS));
	cl.writeResult("�������� ������ ���������� ������ ���������� ���������",
		filter->getResults(TC_GET_NON_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS));
	cl.writeResult("�������� ������ ��� ���������� ������",
		filter->getResults(TC_CREATE_BILLING_RECORD));
}

/**
 * ��������� ������������ ����������� ������ (Data and Database Integrity 
 * Testing) � ������� ��������� �� test cases � checklist.
 */
int main(int argc, char* argv[])
{
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		TCResultFilter* filter = executeIntegrityTest();
		saveCheckList(filter);
		delete filter;
		StoreManager::shutdown();
	}
	catch (...)
	{
		cout << "Failed to execute test. See the logs" << endl;
	}

	return 0;
}

