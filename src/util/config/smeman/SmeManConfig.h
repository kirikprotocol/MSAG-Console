#ifndef SMSC_UTIL_CONFIG_SMEMAN_SMEMANCONFIG
#define SMSC_UTIL_CONFIG_SMEMAN_SMEMANCONFIG

#include "smeman_structures.h"

#include <util/xml/DOMTreeReader.h>

namespace smsc {
namespace util {
namespace config {
namespace smeman {

using smsc::util::xml::DOMTreeReader;

class SmeManConfig
{
protected:
	typedef std::vector<SmeRecord*> SRVector;
public:
	enum status {success, fail};

	class RecordIterator
	{
	public:
		bool hasRecord();
		status fetchNext(SmeRecord *&record);
		RecordIterator(SRVector const &records_vector);
	protected:
		SRVector::const_iterator iter;
		SRVector::const_iterator end;
	};

	SmeManConfig();
	virtual ~SmeManConfig();

	void clear();
	status putRecord(SmeRecord *record);

	status load(const char * const filename);
	status reload();

	status store(const char * const filename) const;
	status store(std::ostream &out) const;

	RecordIterator getRecordIterator() const;

	std::string getText() const;
protected:
	DOMTreeReader reader;
	log4cpp::Category &logger;
	SRVector records;
	std::auto_ptr<char> configFileName;

private:
};

}
}
}
}

#endif //ifndef SMSC_UTIL_CONFIG_SMEMAN_SMEMANCONFIG

