#include "sms/sms.h"
#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/Util.hpp"
#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;
using namespace smsc::sms;
using namespace smsc::test::sms;
using namespace smsc::test::smpp;

int main(int argc, char* argv[])
{
	/*
	char str1[30], str2[30], str3[30];
	time_t lt = time(NULL);
	cout << "time = " << lt << endl;
	cout << "str1 = " << SmppUtil::time2string(lt, str1, lt, 1) << endl;
	cout << "str2 = " << SmppUtil::time2string(lt, str2, lt, 2) << endl;
	cout << "str3 = " << SmppUtil::time2string(lt, str3, lt, 3) << endl;
	cout << "inverse2 = " << (SmppUtil::string2time(str2, lt) - lt) << endl;
	cout << "inverse3 = " << (SmppUtil::string2time(str3, lt) - lt) << endl;
	*/
	time_t lt = time(NULL);
	cout << "000000000028000R -> " << SmppUtil::string2time("000000000028000R", lt) << endl;
	cout << "020809140850000+ -> " << SmppUtil::string2time("020809140850000+", lt) << endl;
	cout << "020809210917028+ -> " << SmppUtil::string2time("020809210917028+", lt) << endl;
	
	return 0;
}
