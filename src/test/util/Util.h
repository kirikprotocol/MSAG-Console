#ifndef TEST_UTIL
#define TEST_UTIL

namespace smsc {
namespace test {
namespace util {

	/**
	 * Используется для выбора варианта в test case
	 * 
	 * @param value
	 * @param maxValue
	 * @return 
	 * <ul>
	 * <li>случайное число от 0 до maxValue, если value < 0
	 * <li>value, если 0 < value < maxValue
	 * <li>остаток от деления value на maxValue, если value > maxValue
	 * </ul>
	 */
	int selectTestProc(int value, int maxValue);
}
}
}

#endif /* TEST_UTIL */
