#include "test/smpp/SmppUtil.hpp"
#include "smpp/smpp_time.h"

using namespace smsc::smpp;
using namespace smsc::test::smpp;

int main(int argc, char* argv[])
{
	time_t base = time(NULL);
	time_t lt = time(NULL) + 10000;
	SmppTime s;
	COStr str;
	str.copy(SmppUtil::time2string(lt, s, base, 1));
	printf("1: %s -> %ld, must be %ld, diff = %ld\n",
		(const char*) str, smppTime2CTime(str), lt, smppTime2CTime(str) - lt);
	str.copy(SmppUtil::time2string(lt, s, base, 2));
	printf("2: %s -> %ld, must be %ld, diff = %ld\n",
		(const char*) str, smppTime2CTime(str), lt, smppTime2CTime(str) - lt);
	str.copy(SmppUtil::time2string(lt, s, base, 3));
	printf("3: %s -> %ld, must be %ld, diff = %ld\n",
		(const char*) str, smppTime2CTime(str), lt, smppTime2CTime(str) - lt);
	return 0;
}
