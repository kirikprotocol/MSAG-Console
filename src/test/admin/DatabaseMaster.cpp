#include "test/util/TextUtil.hpp"
#include "util/config/Manager.h"
#include "store/StoreManager.h"
#include "smpp/smpp.h"
#include "util/config/ConfigView.h"
#include "db/DataSourceLoader.h"
#include "profiler/profiler.hpp"
#include <fstream>

using smsc::util::config::ConfigView;
using namespace smsc::sms;
using namespace smsc::smpp;
using namespace smsc::util::config;
using namespace smsc::store;
using namespace smsc::profiler;
using namespace smsc::db;
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
	DatabaseMaster()
	{
		Manager::init("config.xml");
	}

	void genProfiles(int count, int shift);
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

void DatabaseMaster::genProfiles(int count, int shift)
{
    ConfigView dsConfig(Manager::getInstance(), "StartupLoader");
    DataSourceLoader::loadup(&dsConfig);
    DataSource* dataSource = DataSourceFactory::getDataSource("OCI");
	__require__(dataSource);
    ConfigView config(Manager::getInstance(), "DataSource");
    dataSource->init(&config);
	Profile defProfile;
	SmeManager smeMan;
	Profiler profiler(defProfile, &smeMan, "profiler");
	profiler.loadFromDB(dataSource);
	for (int i = 0; i < count; i++)
	{
		string strAddr;
		int reportOptions;
		int codePage;
		switch (i % 4)
		{
			case 0:
				strAddr = ".0.0.";
				reportOptions = 1;
				codePage = 8;
				break;
			case 1:
				strAddr = ".0.1.";
				reportOptions = 1;
				codePage = 0;
				break;
			case 2:
				strAddr = ".1.0.";
				reportOptions = 0;
				codePage = 8;
				break;
			case 3:
				strAddr = ".1.1.";
				reportOptions = 0;
				codePage = 0;
				break;
		}
		strAddr += str(i);
		Profile profile;
		profile.codepage = codePage;
		profile.reportoptions = reportOptions;
		const Address addr(strAddr.c_str());
		profiler.update(addr, profile);
	}
	profiler.stop();
}

void DatabaseMaster::genSms()
{
	int count = 100;
	int shift = 1;
	StoreManager::startup(Manager::getInstance());
	StoreManager::stopArchiver();
	MessageStore* msgStore = StoreManager::getMessageStore();
	for (int i = 0; i < count; i++)
	{
		Descriptor dst;
		SMS sms;
		time_t submitTime = 1000000000 + (i % 2 ? 10 : -10) * i;
		sms.setSubmitTime(submitTime);
		tm t;
		char strTime[30];
		asctime_r(localtime_r(&submitTime, &t), strTime);
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
		sms.setIntProperty(Tag::SMPP_ESM_CLASS, 0x0);
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
		auto_ptr<char> msg = encode(os.str(), dataCoding, msgLen, true);
		sms.setIntProperty(Tag::SMPP_DATA_CODING, dataCoding);
		sms.setIntProperty(Tag::SMPP_SM_LENGTH, msgLen);
		sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, msg.get(), msgLen);
		//sms.setBinProperty(Tag::SMSC_RAW_SHORTMESSAGE, msg.get(), msgLen);
		//sms_msg
		SMSId smsId = msgStore->getNextId();
		msgStore->createSms(sms, smsId, CREATE_NEW);
		__trace2__("message created: smsId = %llu, submitTime = %s, state = ENROUTE",
			smsId, strTime);
		//sms_arc
		const char* state;
		SMSId smsId2 = msgStore->getNextId();
		msgStore->createSms(sms, smsId2, CREATE_NEW);
		switch (i % 4)
		{
			case 0:
				state = "DELIVERED";
				msgStore->changeSmsStateToDelivered(smsId2, dst);
				break;
			case 1:
				state = "UNDELIVERABLE";
				msgStore->changeSmsStateToUndeliverable(smsId2, dst, 123);
				break;
			case 2:
				state = "EXPIRED";
				msgStore->changeSmsStateToExpired(smsId2);
				break;
			case 3:
				state = "DELETED";
				msgStore->changeSmsStateToDeleted(smsId2);
				break;
		}
		__trace2__("message created: smsId = %llu, submitTime = %s, state = %s",
			smsId2, strTime, state);
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
	DatabaseMaster gen;
	gen.genSms();
	gen.genProfiles(23, 1);
	return 0;
}
