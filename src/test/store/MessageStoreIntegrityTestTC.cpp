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


void executeIntegrityTest(TCResultFilter* filter, int listSize)
{
	cout << ".";
	MessageStoreTestCases tc; //throws exception
	SMSId id[listSize];
	SMS sms[listSize];
	TCResultStack stack[listSize];

	//�������� SM, 100%
	//cout << "�������� SM" << endl;
	for (int i = 0; i < listSize; i++)
	{
		TCResult* res = tc.storeCorrectSM(&id[i], &sms[i], RAND_TC);
		filter->addResult(res);
		stack[i].push_back(res);
		//delete res;
	}
	
	//���������� ����������� SM � ����������� �������� �� ��� ������������ SM, 1/15
	//���������� �������������� SM � ����������, 1/15
	//���������� �������������� SM � �������, 1/15
	//���������� ������������� SM, 1/15
	//��������� ����������� �������, 1/15
	//������������ ��������� ������� SM, 1/15
	//��������� SM, 1/15
	//������������ ��������� SM, 1/15
	//������ SM, 7/15
	//cout << "�������������� ��� ������������� SM" << endl;
	for (TCSelector s(RAND_SET_TC, 15); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < listSize; i++)
				{
					SMSId newId;
					SMS newSMS;
					//�������
					TCResult* res1 = tc.storeCorrectSM(&newId, &newSMS,
						id[i], sms[i], RAND_TC);
					filter->addResult(res1);
					stack[i].push_back(res1);
					//delete res1;
					//���������
					TCResult* res2 = tc.loadExistentSM(id[i], sms[i]);
					filter->addResult(res2);
					stack[i].push_back(res2);
					//delete res2;
					//�������
					TCResult* res3 = tc.deleteExistentSM(newId);
					filter->addResult(res3);
					stack[i].push_back(res3);
					//delete res3;
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.storeReplaceSM(id[i], &sms[i]);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
			case 3:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.storeRejectDuplicateSM(sms[i]);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
			case 4:
				{
					TCResult* res = tc.storeIncorrectSM(RAND_TC);
					filter->addResult(res);
					//stack[i].push_back(res);
					//delete res;
				}
				break;
			case 5:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.setCorrectSMStatus(id[i], &sms[i], RAND_TC);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
			case 6:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.setIncorrectSMStatus(id[i]);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
			case 7:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.replaceCorrectSM(id[i], &sms[i], RAND_TC);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
			case 8:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.replaceIncorrectSM(id[i], sms[i], RAND_TC);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
			default: //case = 9..15
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.loadExistentSM(id[i], sms[i]);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
		}
	}

	//�������� SM, 100%
	//cout << "�������� SM" << endl;
	for (int i = 0; i < listSize; i++)
	{
		TCResult* res = tc.deleteExistentSM(id[i]);
		filter->addResult(res);
		stack[i].push_back(res);
		//delete res;
	}

	//��������� ������� �������������� SM
	//��������� �������������� SM
	//������ �������������� SM
	//�������� �������������� SM
	//cout << "�������������� ��� ���������� SM" << endl;
	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.setNonExistentSMStatus(id[i], RAND_TC);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.replaceNonExistentSM(id[i], RAND_TC);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
			case 3:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.loadNonExistentSM(id[i], RAND_TC);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
			case 4:
				for (int i = 0; i < listSize; i++)
				{
					TCResult* res = tc.deleteNonExistentSM(id[i], RAND_TC);
					filter->addResult(res);
					stack[i].push_back(res);
					//delete res;
				}
				break;
		}
	}

	//������
	//cout << "������" << endl;
#ifdef ASSERT_THROW_IF_FAIL
	filter->addResult(tc.storeAssertSM(ALL_TC));
#endif

	//��������� �����������
	//cout << "��������� �����������" << endl;
	for (int i = 0; i < listSize; i++)
	{
		filter->addResultStack(stack[i]);
	}
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "���������� checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	//�������������������� ���� �����
	cl.writeResult("���������� ����������� SM",
		filter->getResults(TC_STORE_CORRECT_SM));
	cl.writeResult("���������� SM � ���������� ��� �������������",
		filter->getResults(TC_STORE_REPLACE_SM));
	cl.writeResult("���������� �������������� SM",
		filter->getResults(TC_STORE_REJECT_DUPLICATE_SM));
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
	
	//���� ��� ���������������������� ���� �����
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
 * Testing) � ������� ��������� �� ���� ������ � checklist.
 */
int main(int argc, char* argv[])
{
	try
	{
		Manager::init("config.xml");
		StoreManager::startup(Manager::getInstance());
		TCResultFilter* filter = new TCResultFilter();
		for (int i = 0; i < 100; i++)
		{
			executeIntegrityTest(filter, 1);
		}
		for (int i = 0; i < 20; i++)
		{
			executeIntegrityTest(filter, 5);
		}
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

