#ifndef TEST_STORE_MESSAGE_STORE_INTEGRITY_TEST
#define TEST_STORE_MESSAGE_STORE_INTEGRITY_TEST

namespace smsc  {
namespace test  {
namespace store {

/**
 * ���� ����� �������� ��� test cases ����������� ��� ������������ ����������
 * Message Store:
 * <ul>
 * <li>���������� SMS ��������� � ��.
 * <li>���������� ��� ������������ � �� SMS ���������.
 * <li>�������� SMS ��������� �� ��.
 * <li>(???)�������� ������� ��� �������� � ������� �� CBOSS.
 * </ul>
 * 
 * @author bryz
 */
class MessageStoreIntegrityTest
{
public:
	/**
	 * ��������� ��� test cases.
	 */
	void executeAllTestCases();

	/**
	 * ��������� ���������� SMS ��������� � ����.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	void storeCorrectSM();

	/**
	 * ��������� ������������ SMS ��������� � ����.
	 * ����������� ������ ������ ���������� � ���.
	 */
	void storeIncorrectSM();

	/**
	 * ��������� � ���� ���������� SMS ���������, ����� ��������� �������� ������ SM.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	void setCorrectSMStatus();

	/**
	 * ��������� � ���� ���������� SMS ���������, ����� �������� ��������� ������������ ������ SM.
	 * ����������� ������ ������ ���������� � ���.
	 */
	void setIncorrectSMStatus();

	/**
	 * ��������� � ���� ���������� SMS ���������, ����� ������� ���.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	void deleteExistingSM();
	
	/**
	 * �������� ������� �������������� � ���� SMS ���������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	void deleteNonExistingSM();
	
	/**
	 * ��������� � ���� ���������� SMS ���������, ����� ��������� ���.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	void loadExistingSM();

	/**
	 * �������� ��������� �������������� � ���� SMS ���������.
	 * ������ debug ���������� ������ ���������� � ���.
	 */
	void loadNonExistingSM();

private:
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_INTEGRITY_TEST */
