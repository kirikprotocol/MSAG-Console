#ifndef SCAG_CONFIG_SMEMANCONFIG
#define SCAG_CONFIG_SMEMANCONFIG

#include "smeman_structures.h"
#include <vector>

namespace scag {
namespace config {


class SmppManConfig
{
public:
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

  SmppManConfig();
  virtual ~SmppManConfig();

  void clear();
  status putRecord(SmeRecord *record);

  status load(const char * const filename);
  status reload();

  status store(const char * const filename);
  status store(std::ostream &out);

  RecordIterator getRecordIterator() const;

  std::string getText();
protected:
  smsc::logger::Logger *logger;
  SRVector records;
  std::auto_ptr<char> configFileName;

private:
};

}
}

#endif //ifndef SMSC_UTIL_CONFIG_SMEMAN_SMEMANCONFIG
