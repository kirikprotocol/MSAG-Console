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

/**
 * select * from sms_msg where oa like '.0.____' and dda like '.0.____' and
 *   src_sme_id like '_0' and dst_sme_id like '_8' and
 *   route_id like '_8' and id like '000000000000001_'
 * oa = .0.???? (50 записей)
 * da = .0.???? (25 записей)
 * src_sme_id = ?0 (13 записей)
 * dst_sme_id = ?8 (6 записей)
 * route_id = ?8 (3 записи)
 * submit_time = [09.09.2001 1:40:00, 09.09.2001 1:50:00] (2 записи)
 * id = 000000000000001? (1 запись)
 */
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
		sms.setSubmitTime(1000000000 + (i % 2 ? 10 : -10) * i);
		sms.setValidTime(LONG_MAX);
		sms.setNextTime(LONG_MAX);
		//oa
		string srcAddr = getTonNpi(i) + str(i);
		sms.setOriginatingAddress(srcAddr.c_str());
		//dda
		int i2 = (i + shift) % count;
		string destAddr = getTonNpi(i2) + str(i2);
		sms.setDealiasedDestinationAddress(destAddr.c_str());
		//da
		int i3 = (i + 2 * shift) % count;
		string destAddr2 = getTonNpi(i3) + str(i3);
		sms.setDestinationAddress(destAddr2.c_str());
		//ids
		sms.setSourceSmeId(str(i % 8).c_str());
		sms.setDestinationSmeId(str(i % 16).c_str());
		sms.setRouteId(str(i % 32).c_str());
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
	DatabaseMaster gen(100, 1);
	gen.genSms();
	return 0;
}
