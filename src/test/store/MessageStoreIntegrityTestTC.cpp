#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>

using namespace std;
using namespace smsc::sms;
using namespace smsc::test::store;
using namespace smsc::test::util;

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
 *
 * @param argc
 * @param argv
 * @return 
 */
int main(int argc, char* argv[])
{
	MessageStoreTestCases tc; //throws StoreException
    TCResultFilter* filter = newFilter();

	//���������� ������
	SMSId id;
	SMS sms;
	for (int i = 0; i < 10; i++)
	{
cout << "i = " << i << endl;
		TCResultStack stack;
		stack.push_back(tc.storeCorrectSM(&id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));
		filter->addResultStack(stack);
	}
	filter->addResult(*tc.storeIncorrectSM(sms, ALL_TC));
	filter->addResult(*tc.loadNonExistentSM());
	filter->addResult(*tc.storeAssertSM(ALL_TC));
	
	//��������� checklist
cout << "Save checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store", "smsc::store");
	cl.writeResult(*filter);


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

