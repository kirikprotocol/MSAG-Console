#ifndef TEST_UTIL
#define TEST_UTIL

namespace smsc {
namespace test {
namespace util {

	/**
	 * ������������ ��� ������ �������� � test case
	 * 
	 * @param value
	 * @param maxValue
	 * @return 
	 * <ul>
	 * <li>��������� ����� �� 0 �� maxValue, ���� value < 0
	 * <li>value, ���� 0 < value < maxValue
	 * <li>������� �� ������� value �� maxValue, ���� value > maxValue
	 * </ul>
	 */
	int selectTestProc(int value, int maxValue);
}
}
}

#endif /* TEST_UTIL */
