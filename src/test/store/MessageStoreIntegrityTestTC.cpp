#include "util/config/Manager.h"
#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>

using namespace std;
using namespace smsc::sms;
using namespace smsc::test::store;
using namespace smsc::test::util;
using namespace smsc::util::config;

TCResultFilter* newFilter()
{
    TCResultFilter* filter = new TCResultFilter();
	filter->registerTC(TC_STORE_CORRECT_SM,
		"���������� ����������� SM");
	filter->registerTC(TC_STORE_INCORRECT_SM,
		"���������� ������������� SM");
	filter->registerTC(TC_STORE_ASSERT_SM,
		"���������� ������������� SM, �������� �� assert");
	filter->registerTC(TC_SET_CORRECT_SM_STATUS,
		"���������� ��������� ������� SM");
	filter->registerTC(TC_SET_INCORRECT_SM_STATUS,
		"������������ ��������� ������� SM");
	filter->registerTC(TC_SET_NON_EXISTENT_SM_STATUS,
		"��������� ������� ��������������� SM");
	filter->registerTC(TC_UPDATE_CORRECT_EXISTENT_SM,
		"���������� ���������� ������������� SM");
	filter->registerTC(TC_UPDATE_INCORRECT_EXISTENT_SM,
		"���������� ������������� SM ������������� �������");
	filter->registerTC(TC_UPDATE_NON_EXISTENT_SM,
		"���������� ��������������� SM");
	filter->registerTC(TC_DELETE_EXISTENT_SM,
		"�������� ������������� SM");
	filter->registerTC(TC_DELETE_NON_EXISTENT_SM,
		"�������� ��������������� SM");
	filter->registerTC(TC_DELETE_EXISTENT_WAITING_SM_BY_NUMBER,
		"�������� ������������ SM ��������� �������� �� ������������ �����");
	filter->registerTC(TC_DELETE_NON_EXISTENT_WAITING_SM_BY_NUMBER,
		"�������� �������������� SM ��������� �������� �� ������������ �����");
	filter->registerTC(TC_LOAD_EXISTENT_SM,
		"������ ������������� SM");
	filter->registerTC(TC_LOAD_NON_EXISTENT_SM,
		"������ ��������������� SM");
	filter->registerTC(TC_LOAD_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER,
		"�������� ��������� ������ SM ��������� �������� �� ������������ �����");
	filter->registerTC(TC_LOAD_NON_EXISTENT_WAITING_SM_BY_DESTINATION_NUMBER,
		"�������� ������� ������ SM ��������� �������� �� ������������ �����");
	filter->registerTC(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER,
		"�������� ��������� ������ SM ������������ �� ������������ �����");
	filter->registerTC(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_DESTINATION_NUMBER,
		"�������� ������� ������ SM ������������ �� ������������ �����");
	filter->registerTC(TC_LOAD_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER,
		"�������� ��������� ������ SM ������������ � ������������� ������");
	filter->registerTC(TC_LOAD_NON_EXISTENT_SM_ARCHIEVE_BY_ORIGINATING_NUMBER,
		"�������� ������� ������ SM ������������ � ������������� ������");
	filter->registerTC(TC_GET_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS,
		"�������� �������� ���������� ������ ���������� ���������");
	filter->registerTC(TC_GET_NON_EXISTENT_SM_DELIVERY_FAILURE_STATISTICS,
		"�������� ������ ���������� ������ ���������� ���������");
	filter->registerTC(TC_CREATE_BILLING_RECORD,
		"�������� ������ ��� ���������� ������");
	return filter;
}

/**
 * ��������� ������������ ����������� ������ (Data and Database Integrity 
 * Testing) � ������� ��������� �� test cases � checklist.
 */
int main(int argc, char* argv[])
{
    Manager::init("config.xml");
	MessageStoreTestCases tc; //throws exception
    TCResultFilter* filter = newFilter();

	//������ SM ��� ���������� �������� �� �������� �������������� SM
	SMSId correctId;
	SMS correctSM;
	delete tc.storeCorrectSM(&correctId, &correctSM, RAND_TC);

	//������ SM � ����� �����
	cout << "���� 1: �������� & ������" << endl;
	for (int i = 0; i < 10; i++)
	{
		cout << "i = " << i << endl;
		SMSId id;
		SMS sms;
		TCResultStack stack;
		stack.push_back(tc.storeCorrectSM(&id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));
		filter->addResultStack(stack);
	}
	//������ ������ �� �����, � ����� ���� ������ ���� ������
	const int listSize = 10;
	for (int j = 0; j < 5; j++)
	{
		SMSId id[listSize];
		SMS sms[listSize];
		TCResultStack stack[listSize];
		cout << "���� 2: �������� SM" << endl;
		for (int i = 0; i < listSize; i++)
		{
			cout << "i = " << i << endl;
			filter->addResult(*tc.storeCorrectSM(&id[i], &sms[i], RAND_TC));
			stack[i].push_back(tc.storeCorrectSM(&id[i], &sms[i], RAND_TC));
		}
		cout << "���� 2: ������ SM" << endl;
		for (int i = 0; i < listSize; i++)
		{
			cout << "i = " << i << endl;
			filter->addResult(*tc.loadExistentSM(id[i], sms[i]));
			stack[i].push_back(tc.loadExistentSM(id[i], sms[i]));
			filter->addResultStack(stack[i]);
		}
	}

	//������
	cout << "������" << endl;
	filter->addResult(*tc.storeIncorrectSM(correctSM, ALL_TC));
	filter->addResult(*tc.loadNonExistentSM());
	filter->addResult(*tc.storeAssertSM(ALL_TC));
	
	//��������� checklist
	cout << "���������� checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	cl.writeResult(*filter);
	delete filter;

	/*
	//��������� ������� SM
	cl.writeResult("���������� ��������� ������� SM", tc.setCorrectSMStatus());
	cl.writeResult("������������ ��������� ������� SM", 
		tc.setIncorrectSMStatus());
	cl.writeResult("��������� ������� ��������������� SM", 
		tc.setNonExistentSMStatus());
	
	//���������� SM
	cl.writeResult("���������� ���������� ������������� SM", 
		tc.updateCorrectExistentSM());
	cl.writeResult("���������� ������������� SM ������������� �������", 
		tc.updateIncorrectExistentSM());
	cl.writeResult("���������� ��������������� SM", tc.updateNonExistentSM());
	
	//�������� SM
	cl.writeResult("�������� ������������� SM", tc.deleteExistentSM());
	cl.writeResult("�������� ��������������� SM", tc.deleteNonExistentSM());
	
	//�������� ������ ��� ���������� ������
	cl.writeResult("�������� ������ ��� ���������� ������", 
		tc.createBillingRecord());
	*/

	return 0;
}

