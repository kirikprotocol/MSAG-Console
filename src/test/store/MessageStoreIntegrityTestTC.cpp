#include "MessageStoreTestCases.hpp"
#include "test/util/CheckList.hpp"
#include "test/util/Util.hpp"
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
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store");
	
	//���������� SM
	SMS sms;
	cl.writeResult("���������� ����������� SM",
		tc.storeCorrectSM(&sms, ALL_TC)); //RAND_TC
	cl.writeResult("���������� ������������� SM",
		tc.storeIncorrectSM(sms, ALL_TC));
	
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
	cl.writeResult("�������� ������������� SM", tc.deleteExistingSM());
	cl.writeResult("�������� ��������������� SM", tc.deleteNonExistingSM());
	
	//������ SM
	cl.writeResult("������ ������������� SM", tc.loadExistingSM());
	cl.writeResult("������ ��������������� SM", tc.loadNonExistingSM());
	
	//�������� ������ ��� ���������� ������
	cl.writeResult("�������� ������ ��� ���������� ������", 
		tc.createBillingRecord());
	*/

	return 0;
}

