/*
  $Id$
*/


#include <inttypes.h>
#include <vector>
#include "logger/Logger.h"

#ifndef SMSC_UTIL_CONFIG_ALIAS_ALIASCONFIG
#define SMSC_UTIL_CONFIG_ALIAS_ALIASCONFIG

namespace smsc {
namespace util {
namespace config {
namespace alias {

struct AliasRecord
{
  char* addrValue;
  int addrTni;
  int addrNpi;
  char* aliasValue;
  int aliasTni;
  int aliasNpi;
  bool hide;
  
  AliasRecord()
  {
    addrValue = aliasValue = NULL;
    addrTni = addrNpi = aliasTni = aliasNpi = 0;
    hide = false;
  }
  
  ~AliasRecord()
  {
    if (addrValue != NULL)
      delete addrValue;
    if (aliasValue != NULL)
      delete aliasValue;
  }
};

class AliasConfig
{
protected:
public:
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
  status reload();

  status store(const char * const filename);

  RecordIterator getRecordIterator() const;
protected:
  smsc::logger::Logger *logger;
  SRVector records;
  std::auto_ptr<char> config_filename;
private:
};

}
}
}
}

#endif //ifndef SMSC_UTIL_CONFIG_ALIAS_ALIASCONFIG
