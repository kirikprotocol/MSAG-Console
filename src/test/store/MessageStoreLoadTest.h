#ifndef TEST_STORE_MESSAGE_STORE_LOAD_TEST
#define TEST_STORE_MESSAGE_STORE_LOAD_TEST

namespace smsc  {
namespace test  {
namespace store {

/**
 * ���� ����� �������� ������������ test case ��� ������������ ���������� 
 * Message Store ��� ���������� � ������� ���������.
 * @author bryz
 */
class MessageStoreLoadTest
{
public:
	/**
	 * ��������� ���� �� ����������. ���� ��������� � ����������� ����� test 
	 * cases:
	 * <ul>
	 * <li>���������� SMS ��������� (20 ����������, 1 ������������) � ����.
	 * <li>��������� ������� SMS ��������� (20 ����������, 1 ������������).
	 * <li>�������� SMS ��������� �� ���� (1 ������������).
	 * </ul>
	 * 
	 * @param numThreads ���������� ������������� ���������. � ������ ��������
	 * ��������� ���������� (��� ����) ����������� � ����.
	 */
	void startTest(int numThreads);

	/**
	 * ���������� ���������� �����.
	 * 
	 * @return ���������� SMS ��������� ����������� � ���� � �������.
	 */
	int stopTest();

	/**
	 * �������� ������� ���������� �� �����.
	 * 
	 * @return ���������� SMS ��������� ����������� � ���� � �������.
	 */
	int getStat();

	/**
	 * ����������� ������� �� stdout:
	 * <ul>
	 * <li>start <numThreads> - ������ �����
	 * <li>stop - ������� ����� � ����� �����������
	 * <li>stat - ����� ������� �����������
	 * </ul>
	 */
	void runInteractiveMode();
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_LOAD_TEST */
