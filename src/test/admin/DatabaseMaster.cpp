#include "test/util/TextUtil.hpp"
#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "smpp/smpp.h"
#include <fstream>

using namespace smsc::sms;
using namespace smsc::smpp;
using namespace smsc::util::config;
using namespace smsc::store;
using namespace smsc::test::util;
using namespace std;

struct DatabaseMaster
{
	const int count;
	const int shift;
	
	DatabaseMaster(int _count, int _shift)
	: count(_count), shift(_shift) {}

	void genSms();
	const string str(int i);
};

const string DatabaseMaster::str(int i)
{
	char buf[4];
	sprintf(buf, "%02d", i);
	return buf;
}

void DatabaseMaster::genSms()
{
	Manager::init("config.xml");
	StoreManager::startup(Manager::getInstance());
	MessageStore* msgStore = StoreManager::getMessageStore();
	for (int i = 0; i < count; i++)
	{
		Descriptor dst;
		SMS sms;
		sms.setSubmitTime(time(NULL) + 10 * count);
		sms.setValidTime(LONG_MAX);
		sms.setNextTime(LONG_MAX);
		string addr;
		switch (i % 4)
		{
			case 0:
				addr = ".0.0.";
				break;
			case 1:
				addr = ".0.1.";
				break;
			case 2:
				addr = ".1.0.";
				break;
			case 3:
				addr = ".1.1.";
				break;
		}
		string srcAddr = addr + str(i);
		sms.setOriginatingAddress(srcAddr.c_str());
		string destAddr = addr + str((i + shift) % count);
		sms.setDestinationAddress(destAddr.c_str());
		string destAddr2 = addr + str((i + 2 * shift) % count);
		sms.setDealiasedDestinationAddress(destAddr2.c_str());
		sms.setArchivationRequested(true);
		ostringstream os;
		os << "àáâãäå¸æçèéêëìíîïðñòóôõö÷øùúûüýþÿ ";
		os << "ÀÁÂÃÄÅ¨ÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞß ";
		os << str(i);
		uint8_t dataCoding = i % 2 ? DataCoding::DEFAULT : DataCoding::UCS2;
		int msgLen;
		auto_ptr<char> msg = encode(os.str(), dataCoding, msgLen);
		sms.setIntProperty(Tag::SMPP_DATA_CODING, dataCoding);
		sms.setIntProperty(Tag::SMPP_SM_LENGTH, msgLen);
		sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, msg.get(), msgLen);
		//sms_msg
		SMSId smsId = msgStore->getNextId();
		msgStore->createSms(sms, smsId, CREATE_NEW);
		//sms_arc
		SMSId smsId2 = msgStore->getNextId();
		msgStore->createSms(sms, smsId2, CREATE_NEW);
		switch (i % 4)
		{
			case 0:
				msgStore->changeSmsStateToDelivered(smsId2, dst);
				break;
			case 1:
				msgStore->changeSmsStateToUndeliverable(smsId2, dst, 123);
				break;
			case 2:
				msgStore->changeSmsStateToExpired(smsId2);
				break;
			case 3:
				msgStore->changeSmsStateToDeleted(smsId2);
				break;
		}
	}
	StoreManager::shutdown();
}

int main(int argc, char* argv[])
{
	int count = 23;
	int shift = 1;
	if (argc == 2)
	{
		count = atoi(argv[1]);
	}
	else if (argc == 3)
	{
		count = atoi(argv[1]);
		shift = atoi(argv[2]);
	}
	DatabaseMaster gen(count, shift);
	gen.genSms();
	return 0;
}
