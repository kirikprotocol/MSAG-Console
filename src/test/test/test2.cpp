#include "test/sms/SmsUtil.hpp"

using namespace smsc::sms;
using namespace smsc::test::sms;

int main(int argc, char* argv[])
{
	SMS sms1, sms2;
	SmsUtil::setupRandomCorrectSms(&sms1);
	uint8_t* buf = sms1.getMessageBody().getBuffer();
	int len = sms1.getMessageBody().getBufferLength();
	sms2.getMessageBody().setBuffer(buf, len);
	return 0;
}
