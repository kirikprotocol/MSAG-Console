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
};

}
}
}

#endif /* TEST_STORE_MESSAGE_STORE_LOAD_TEST */
