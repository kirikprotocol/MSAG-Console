#include <cstdlib>
#include <ctime>
#include "Util.h"

namespace smsc {
namespace test {
namespace util {

//srand((unsigned) time(NULL));

int selectTestProc(int value, int maxValue)
{
	if (value < 0)
	{
		int num = (int) (maxValue * rand() / (RAND_MAX + 1.0));
		return num;
	}
	if (value > maxValue)
	{
		return value % maxValue;
	}
	return value;
}

}
}
}
