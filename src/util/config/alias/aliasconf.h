/*
	$Id$
*/


#include <inttypes.h>

#ifndef SMSC_UTIL_CONFIG_ALIAS_ALIASCONFIG
#define SMSC_UTIL_CONFIG_ALIAS_ALIASCONFIG

#include <util/xml/DOMTreeReader.h>

namespace smsc {
namespace util {
namespace config {
namespace alias {

using smsc::util::xml::DOMTreeReader;

struct AliasRecord
{
	char* addrValue;
	int addrTni;
	int addrNpi;
	char* aliasValue;
	int aliasTni;
	int aliasNpi;
	bool hide;
};

class AliasConfig
{
protected:
  typedef std::vector<AliasRecord*> SRVector;
public:
  enum status {success, fail};

  class RecordIterator
  {
  public:
    bool hasRecord();
    status fetchNext(AliasRecord *&record);
    RecordIterator(SRVector const &records_vector);
  protected:
    SRVector::const_iterator iter;
    SRVector::const_iterator end;
  };

  AliasConfig();
	~AliasConfig()
	{
		for (SRVector::iterator it = records.begin(); 
				 it != records.end() ; 
				 ++it)
		{
			if ( *it ) delete *it;
		}
	}

  void clear();
  status putRecord(AliasRecord *record);

  status load(const char * const filename);

  status store(const char * const filename) const;

  RecordIterator getRecordIterator() const;
protected:
  DOMTreeReader reader;
  log4cpp::Category &logger;
  SRVector records;
private:
};

}
}
}
}

#endif //ifndef SMSC_UTIL_CONFIG_ALIAS_ALIASCONFIG

