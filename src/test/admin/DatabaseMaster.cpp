#include "test/sms/SmsUtil.hpp"
#include "test/smpp/SmppUtil.hpp"
#include "test/util/TextUtil.hpp"
#include "util/config/Manager.h"
#include "store/StoreManager.h"
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
using namespace smsc::test::sms;
using namespace smsc::test::smpp;
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
 * submit_time = [09.09.2001 8:40:00, 09.09.2001 8:50:00] (2 записи)
 * id = 000000000000001? (1 запись)
 */
struct DatabaseMaster
{
	DatabaseMaster()
	{
		Manager::init("config.xml");
	}

	void genProfiles(int count);
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

void DatabaseMaster::genProfiles(int count)
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
		Profile profile;
		profile.codepage = i & 0x2 ? 8 : 0;
		profile.reportoptions = i & 0x1;
		char tmp[8];
		sprintf(tmp, "%02d", i);
		profiler.update(Address(tmp), profile);
		sprintf(tmp, "+%02d", i);
		profiler.update(Address(tmp), profile);
		sprintf(tmp, "%02d?", i);
		profiler.update(Address(tmp), profile);
		sprintf(tmp, "+%02d?", i);
		profiler.update(Address(tmp), profile);
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
		ostringstream os;
		uint8_t dataCoding;
		uint8_t esmClass;
		switch (i % 7)
		{
			case 0: //default (latin1)
				dataCoding = DataCoding::DEFAULT;
				esmClass = 0x0;
				os << str(i) << " (default):" << latinChars << digitChars << symbolChars;
				break;
			case 1: //default & udhi (latin1)
				dataCoding = DataCoding::DEFAULT;
				esmClass = ESM_CLASS_UDHI_INDICATOR;
				os << str(i) << " (default&udhi):" << latinChars << digitChars << symbolChars;
				break;
			case 2: //ucs2
				dataCoding = DataCoding::UCS2;
				esmClass = 0x0;
				os << str(i) << " (ucs2):" << rusChars << latinChars << digitChars << symbolChars;
				break;
			case 3: //ucs2 & udhi
				dataCoding = DataCoding::UCS2;
				esmClass = ESM_CLASS_UDHI_INDICATOR;
				os << str(i) << " (ucs2&udhi):" << rusChars << latinChars << digitChars << symbolChars;
				break;
			case 4: //smsc7bit
				dataCoding = DataCoding::SMSC7BIT;
				esmClass = 0x0;
				os << str(i) << " (smsc7bit):" << latinChars << digitChars << symbolChars;
				break;
			case 5: //smsc7bit & udhi
				dataCoding = DataCoding::SMSC7BIT;
				esmClass = ESM_CLASS_UDHI_INDICATOR;
				os << str(i) << " (smsc7bit&udhi):" << latinChars << digitChars << symbolChars;
				break;
			case 6: //binary
				dataCoding = DataCoding::BINARY;
				esmClass = rand0(1) ? 0x0 : ESM_CLASS_UDHI_INDICATOR;
				os << str(i) << " (binary):" << latinChars << digitChars;
				break;
		}
		sms.setIntProperty(Tag::SMPP_ESM_CLASS, esmClass);
		sms.setIntProperty(Tag::SMPP_DATA_CODING, dataCoding);
		int msgLen;
		auto_ptr<char> msg = encode(os.str(), dataCoding, msgLen, true);
		if (esmClass & ESM_CLASS_UDHI_INDICATOR &&
			dataCoding != DataCoding::BINARY)
		{
			int udhiLen = rand0(5);
			auto_ptr<uint8_t> udhi = rand_uint8_t(udhiLen);
			int bufLen = msgLen + udhiLen + 1;
			char buf[bufLen];
			*buf = (unsigned char) udhiLen;
			memcpy(buf + 1, udhi.get(), udhiLen);
			memcpy(buf + udhiLen + 1, msg.get(), msgLen);
			if (bufLen > MAX_SMPP_SM_LENGTH)
			{
				sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, buf, bufLen);
			}
			else
			{
				sms.setIntProperty(Tag::SMPP_SM_LENGTH, bufLen);
				sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, buf, bufLen);
			}
		}
		else
		{
			if (msgLen > MAX_SMPP_SM_LENGTH)
			{
				sms.setBinProperty(Tag::SMPP_MESSAGE_PAYLOAD, msg.get(), msgLen);
			}
			else
			{
				sms.setIntProperty(Tag::SMPP_SM_LENGTH, msgLen);
				sms.setBinProperty(Tag::SMPP_SHORT_MESSAGE, msg.get(), msgLen);
			}
		}
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
	gen.genProfiles(3);
	return 0;
}
