#include "test/sms/SmsUtil.hpp"

using namespace smsc::sms;
using namespace smsc::test::sms;

int main(int argc, char* argv[])
{
	SMS sms;
	SmsUtil::setupRandomCorrectSms(&sms);
	sms.messageBody.getBuffer();
	return 0;
}
