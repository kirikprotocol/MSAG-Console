#include "test/smpp/SmppUtil.hpp"
#include "smpp/smpp_time.h"

using namespace smsc::test::smpp;

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("conv <smpp_time>");
	}
	SmppTime s;
	SmppUtil::time2string(time(NULL) /*+ 100000*/, s, time(NULL), atoi(argv[1]));
    smsc::smpp::COStr str;
	str.copy(s);

	printf("timezone = %d\n", (int) timezone);
	printf("Current time = %ld\n", time(NULL));
	printf("'%s' -> %ld, should be %ld\n",
		s, smsc::smpp::smppTime2CTime(str), SmppUtil::string2time(s, time(NULL)));
	return 0;
}
