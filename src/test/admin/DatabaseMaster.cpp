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
	const string getTonNpi(int i);
};

const string DatabaseMaster::str(int i)
{
	char buf[4];
	sprintf(buf, "%02d", i);
	return buf;
}

const string DatabaseMaster::getTonNpi(int i)
{
	switch (i % 4)
	{
		case 0:
			return ".0.0.";
		case 1:
			return ".0.1.";
		case 2:
			return ".1.0.";
		case 3:
			return ".1.1.";
	}
}

void DatabaseMaster::genSms()
{
	Manager::init("config.xml");
	StoreManager::startup(Manager::getInstance());
	StoreManager::stopArchiver();
	MessageStore* msgStore = StoreManager::getMessageStore();
	for (int i = 0; i < count; i++)
	{
		Descriptor dst;
		SMS sms;
		sms.setSubmitTime(time(NULL) + (i % 2 ? 10 : -10) * i);
		sms.setValidTime(LONG_MAX);
		sms.setNextTime(LONG_MAX);
		string srcAddr = getTonNpi(i) + str(i);
		sms.setOriginatingAddress(srcAddr.c_str());
		int i2 = (i + shift) % count;
		string destAddr = getTonNpi(i2) + str(i2);
		sms.setDestinationAddress(destAddr.c_str());
		int i3 = (i + 2 * shift) % count;
		string destAddr2 = getTonNpi(i3) + str(i3);
		sms.setDealiasedDestinationAddress(destAddr2.c_str());
		sms.setSourceSmeId(str(i % 2).c_str());
		sms.setDestinationSmeId(str(i % 3).c_str());
		sms.setRouteId(str(i % 4).c_str());
		sms.setArchivationRequested(true);
		ostringstream os;
		uint8_t dataCoding;
		switch (i % 5)
		{
			case 0: //default (latin1)
				dataCoding = DataCoding::DEFAULT;
				os << str(i) << " (default):" << latinChars << digitChars << symbolChars;
				break;
			case 1: //ucs2
				dataCoding = DataCoding::UCS2;
				os << str(i) << " (ucs2):" << rusChars << digitChars;
				break;
			case 2: //ucs2 (другие символы)
				dataCoding = DataCoding::UCS2;
				os << str(i) << " (ucs2):" << latinChars << symbolChars;
				break;
			case 3: //smsc7bit
				dataCoding = DataCoding::SMSC7BIT;
				os << str(i) << " (smsc7bit):" << latinChars << digitChars << symbolChars;
				break;
			case 4: //binary
				dataCoding = DataCoding::BINARY;
				os << str(i) << " (binary):" << latinChars << digitChars;
				break;
		}
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
	StoreManager::startArchiver(); //перенести финализированные записи в архив
	while (StoreManager::isArchivationInProgress())
	{
		sleep(1);
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
