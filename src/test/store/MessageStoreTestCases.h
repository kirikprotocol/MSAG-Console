#ifndef TEST_STORE_MESSAGE_STORE_TEST_CASES
#define TEST_STORE_MESSAGE_STORE_TEST_CASES

namespace smsc  {
namespace test  {
namespace store {

/**
 * ���� ����� �������� ��� test cases ����������� ��� ������������ ����������
 * Message Store.
 * 
 * @author bryz
 */
class MessageStoreTestCases
{
public:
	/**
	 * ���������� ����������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool storeCorrectSM(int num = -1);

	/**
	 * ���������� ������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	bool storeIncorrectSM();

	/**
	 * ���������� ��������� ������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ��������� �������� ������ SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool setCorrectSMStatus();

	/**
	 * ������������ ��������� ������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� �������� ��������� ������������ ������ SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	bool setIncorrectSMStatus();

	/**
	 * ��������� ������� ��������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	bool setNonExistentSMStatus();

	/**
	 * ���������� ���������� ������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool updateCorrectExistentSM();

	/**
	 * ���������� ������������� SM ������������� �������.
	 * ����������� ������ ������ ���������� � ���.
	 */
	bool updateIncorrectExistentSM();
	
	/**
	 * ���������� ��������������� SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	bool updateNonExistentSM();

	/**
	 * �������� ������������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ������� ���.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool deleteExistingSM();
	
	/**
	 * �������� ��������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool deleteNonExistingSM();
	
	/**
	 * ������ ������������� SM.
	 * ��������� � ���� ���������� SMS ���������, ����� ��������� ���.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool loadExistingSM();

	/**
	 * ������ ��������������� SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	bool loadNonExistingSM();

	/**
	 * �������� ������ ��� ���������� ������.
	 */
	bool createBillingRecord();
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_TEST_CASES */
