#include "sms/sms.h"
#include "test/sms/SmsUtil.hpp"
#include "test/util/Util.hpp"
#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>

using namespace std;
using namespace smsc::sms;
using namespace smsc::test::sms;

int main(int argc, char* argv[])
{
	SMS sms, sms1;
	SmsUtil::setupRandomCorrectSms(&sms);
	sms1 = sms;
	vector<int> res1 = SmsUtil::compareMessages(sms1, sms);
	cout << "operator=()" << endl;
	copy(res1.begin(), res1.end(), ostream_iterator<int>(cout, ","));
	cout << "SMS(const SMS&)" << endl;
	SMS sms2(sms);
	vector<int> res2 = SmsUtil::compareMessages(sms2, sms);
	copy(res2.begin(), res2.end(), ostream_iterator<int>(cout, ","));

	return 0;
}
