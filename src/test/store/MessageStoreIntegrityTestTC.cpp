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

#define PREPARE_FOR_NEW_SMS \
	id.push_back(new SMSId()); \
	sms.push_back(new SMS()); \
	stack.push_back(new TCResultStack());
	
void executeIntegrityTest(TCResultFilter* filter, int listSize)
{
	cout << ".";
	MessageStoreTestCases tc; //throws exception
	vector<SMSId*> id;
	vector<SMS*> sms;
	vector<TCResultStack*> stack;

	//���������� ����������� sms, 1/1
	for (int i = 0; i < listSize; i++)
	{
		PREPARE_FOR_NEW_SMS
		TCResult* res = tc.storeCorrectSms(id.back(), sms.back(), RAND_TC);
		stack.back()->push_back(res);
	}
	
	//���������� ����������� sms � ����������� �������� �� ��� ������������ sms, 1/15
	//���������� �������������� sms � �������, 1/15
	//���������� ����������� sms � ���������� ��� �������������� 1/15
	//���������� ������������� sms, 1/15
	//���������� ������� sms � ��������� ENROUTE, 1/15
	//���������� ���������� ������������� sms, 1/15
    //������������ ���������� ������������� sms, 1/15
	//������ ������������� sms, 8/15
	for (TCSelector s(RAND_SET_TC, 15); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < listSize; i++)
				{
					PREPARE_FOR_NEW_SMS
					TCResult* res = tc.storeCorrectSms(id.back(), sms.back(),
						*id[i], *sms[i], RAND_TC);
					stack.back()->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.storeRejectDuplicateSms(*sms[i]);
					stack[i]->push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.storeReplaceCorrectSms(*id[i], sms[i]);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				{
					TCResult* res = tc.storeIncorrectSms(RAND_TC);
					filter->addResult(res);
					//stack[i]->push_back(res);
				}
				break;
			case 5:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.changeExistentSmsStateEnrouteToEnroute(
						*id[i], sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 6:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceCorrectSms(*id[i], sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 7:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			default: //case = 8..15
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadExistentSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
		}
	}

	//������� sms �� ENROUTE � ��������� ���������, 1/1
	for (int i = 0; i < id.size(); i++)
	{
		TCResult* res = tc.changeExistentSmsStateEnrouteToFinal(*id[i], 
			sms[i], RAND_TC);
		stack[i]->push_back(res);
	}
	
	//������� sms � ��������� ��������� � ����� ������ ���������, 1/4
	//���������� sms � ���������� ������������� sms ��������� ���������, 1/4
	//������ ������������� sms, 2/4
	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < listSize; i++)
				{
					PREPARE_FOR_NEW_SMS
					TCResult* res = tc.storeReplaceSmsInFinalState(id.back(),
						sms.back(), *id[i], *sms[i]);
					stack[i]->push_back(res);
				}
				break;
			default: //3..4
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadExistentSms(*id[i], *sms[i]);
					stack[i]->push_back(res);
				}
		}
	}

	//�������� ������������� sms, 1/1
	for (int i = 0; i < id.size(); i++)
	{
		TCResult* res = tc.deleteExistentSms(*id[i]);
		stack[i]->push_back(res);
	}

	//������� ��������������� sms � ����� ���������, 1/4
	//������������ ���������� ��������������� sms, 1/4
	//������ ��������������� sms, 1/4
	//�������� ��������������� sms, 1/4
	for (TCSelector s(RAND_SET_TC, 4); s.check(); s++)
	{
		switch (s.value())
		{
			case 1:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.changeFinalSmsStateToAny(*id[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 2:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.replaceIncorrectSms(*id[i], *sms[i], RAND_TC);
					stack[i]->push_back(res);
				}
				break;
			case 3:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.loadNonExistentSms(*id[i]);
					stack[i]->push_back(res);
				}
				break;
			case 4:
				for (int i = 0; i < id.size(); i++)
				{
					TCResult* res = tc.deleteNonExistentSms(*id[i]);
					stack[i]->push_back(res);
				}
				break;
		}
	}

//���������� ������������� sms � ��������� �� assert
#ifdef ASSERT_THROW_IF_FAIL
	filter->addResult(tc.storeAssertSM(ALL_TC));
#endif

	//��������� �����������
	for (int i = 0; i < listSize; i++)
	{
		filter->addResultStack(*stack[i]);
	}

	//������� ������
	for (int i = 0; i < id.size(); i++)
	{
		delete id[i];
		delete sms[i];
		delete stack[i];
	}
}

void saveCheckList(TCResultFilter* filter)
{
	cout << "���������� checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	//������������������ ���� �����
	cl.writeResult("���������� ����������� sms",
		filter->getResults(TC_STORE_CORRECT_SMS));
	cl.writeResult("���������� �������������� sms � �������",
		filter->getResults(TC_STORE_REJECT_DUPLICATE_SMS));
	cl.writeResult("���������� ����������� sms � ���������� ��� �������������",
		filter->getResults(TC_STORE_REPLACE_CORRECT_SMS));
	cl.writeResult("���������� sms � ���������� ������������� sms ��������� ���������",
		filter->getResults(TC_STORE_REPLACE_SMS_IN_FINAL_STATE));
	cl.writeResult("���������� ������������� sms",
		filter->getResults(TC_STORE_INCORRECT_SMS));
	cl.writeResult("���������� ������������� sms � ��������� �� assert",
		filter->getResults(TC_STORE_ASSERT_SMS));
	cl.writeResult("���������� ������� sms � ��������� ENROUTE",
		filter->getResults(TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_ENROUTE));
	cl.writeResult("������� sms �� ENROUTE � ��������� ���������",
		filter->getResults(TC_CHANGE_EXISTENT_SMS_STATE_ENROUTE_TO_FINAL));
	cl.writeResult("������� ��������������� sms ��� sms � ��������� ��������� � ����� ������ ���������",
		filter->getResults(TC_CHANGE_FINAL_SMS_STATE_TO_ANY));
	cl.writeResult("���������� ���������� ������������� sms",
		filter->getResults(TC_REPLACE_CORRECT_SMS));
	cl.writeResult("������������ ���������� ������������� ��� ���������� ��������������� sms",
		filter->getResults(TC_REPLACE_INCORRECT_SMS));
	cl.writeResult("������ ������������� sms",
		filter->getResults(TC_LOAD_EXISTENT_SMS));
	cl.writeResult("������ ��������������� sms",
		filter->getResults(TC_LOAD_NON_EXISTENT_SMS));
	cl.writeResult("�������� ������������� sms",
		filter->getResults(TC_DELETE_EXISTENT_SMS));
	cl.writeResult("�������� ��������������� sms",
		filter->getResults(TC_DELETE_NON_EXISTENT_SMS));
	
	//���� ��� ���������������������� ���� �����
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
		for (int i = 0; i < 50; i++)
		{
			executeIntegrityTest(filter, 1);
		}
		for (int i = 0; i < 50; i++)
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

