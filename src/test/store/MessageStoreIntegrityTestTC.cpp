#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
#include "test/util/TCResultFilter.hpp"
#include <iostream>

using namespace std;
using namespace smsc::sms;
using namespace smsc::test::store;
using namespace smsc::test::util;

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
    TCResultFilter filter;

	//����������� �������� test cases
cout << "Register test cases" << endl;
	filter.registerTC(TC_STORE_CORRECT_SM, "���������� ����������� SM");
	filter.registerTC(TC_STORE_INCORRECT_SM, "���������� ������������� SM");
	filter.registerTC(TC_LOAD_EXISTENT_SM, "������ ������������� SM");
	filter.registerTC(TC_LOAD_NONEXISTENT_SM, "������ ��������������� SM");
	
	//���������� ������
	SMSId id;
	SMS sms;
	for (int i = 0; i < 10; i++)
	{
cout << "i = " << i << endl;
		TCResultStack stack;
		stack.push_back(tc.storeCorrectSM(&id, &sms, RAND_TC));
		stack.push_back(tc.loadExistentSM(id, sms));
		filter.addResultStack(stack);
	}
	filter.addResult(*tc.storeIncorrectSM(sms, RAND_TC));
	filter.addResult(*tc.loadNonExistentSM());
	
	//��������� checklist
cout << "Save checklist" << endl;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store");
	cl.writeResult(filter);


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

