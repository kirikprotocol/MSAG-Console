#include "MessageStoreTestCases.h"
#include "../util/CheckList.h"

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
	MessageStoreTestCases tc;
	CheckList& cl = CheckList::getCheckList(CheckList::UNIT_TEST);
	cl.startNewGroup("Message Store");
	
	//���������� SM
	cl.writeResult("���������� ����������� SM", tc.storeCorrectSM());
	cl.writeResult("���������� ������������� SM", tc.storeIncorrectSM());
	
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

	return 0;
}

