#include <util/config/smeman/SmeManConfig.h>
#include <util/cstrings.h>
#include <iostream>

using smsc::util::config::smeman::SmeManConfig;
using smsc::util::config::smeman::SmeRecord;
using smsc::util::config::smeman::SMPP_SME;
using smsc::util::config::smeman::SS7_SME;
using smsc::util::cStringCopy;

std::ostream & operator <<(std::ostream &out, SmeRecord &record)
{
	out << "  -----------------------------------------------" << std::endl;
	out << "  UID: \"" << record.smeUid << '"' << std::endl;
	if (record.rectype == SMPP_SME)
	{
		out << "  Type: SMPP" << std::endl;
		out << "    typeOfNumber:     " << record.recdata.smppSme.typeOfNumber << std::endl;
		out << "    numberingPlan:    " << record.recdata.smppSme.numberingPlan << std::endl;
		out << "    interfaceVersion: " << record.recdata.smppSme.interfaceVersion << std::endl;
		out << "    systemType:       \"" << record.recdata.smppSme.systemType << '"' << std::endl;
		out << "    password:         \"" << record.recdata.smppSme.password << '"' << std::endl;
		out << "    addrRange:        \"" << record.recdata.smppSme.addrRange << '"' << std::endl;
		out << "    smeN:             " << record.recdata.smppSme.smeN << std::endl;
	} else {
		out << "  Type: SS7" << std::endl;
	}
}

std::ostream & operator <<(std::ostream &out, SmeManConfig &config)
{
	out << "*************************************************" << std::endl;
	for (SmeManConfig::RecordIterator i = config.getRecordIterator(); i.hasRecord();)
	{
		SmeRecord* record;
		i.fetchNext(record);
		out << (*record);
	}
	out << "*************************************************" << std::endl;
}

int main(int argc, char ** argv)
{
	SmeManConfig config;
	if (config.load("SmeRecords.xml") != SmeManConfig::success)
	{
		std::cout << "ERROR: Cannot load config" << std::endl;
		return -1;
	}
	std::cout << config;
	{
		SmeRecord* record = new SmeRecord();
		record->rectype = SMPP_SME;
		record->smeUid = cStringCopy("test sme uid");
		record->recdata.smppSme.typeOfNumber = 1;
		record->recdata.smppSme.numberingPlan = 2;
		record->recdata.smppSme.interfaceVersion = 3;
		record->recdata.smppSme.systemType = cStringCopy("test system type");
		record->recdata.smppSme.password = cStringCopy("test password");
		record->recdata.smppSme.addrRange = cStringCopy("test addr range");
		record->recdata.smppSme.smeN = 4;
		if (config.putRecord(record) != SmeManConfig::success) {
			std::cout << "ERROR: Cannot put record" << std::endl;
			return -1;
		}
	}
	std::cout << config;
	if (config.store("SmeRecords.new.xml") != SmeManConfig::success) {
		std::cout << "ERROR: Cannot store config" << std::endl;
		return -1;
	}
}
