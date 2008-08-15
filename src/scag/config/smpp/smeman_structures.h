/*
  $Id$
*/
/*

<smerecord type="smpp" uid="terminal345">
  <param name="typeOfNumber" value="0"/>
  <param name="numberingPlan" value="0"/>
  <param name="interfaceVersion" value="0x34"/>
  <param name="systemType" value="????"/>
  <param name="password" value="drowssap"/>
  <param name="addrRange" value="902*"/>
  <param name="smeN" value="0"/>
</smerecord>

SmeMan* smeman;
.... getting manager
SmeManConfig config;
config.load("smeman.xml");
SmeManConfig::RecordIterator iter = config.getRecordIterator();
while ( iter.hasRecord() )
{
  SmeRecord record;
  iter.fetchNext(&record);
  smeman.registerSme(&record);
}
......

config.clear();
SmeMan::RecordIterator iter = smeMan.getRecordIterator();
while ( iter.hasRecord() )
{
  SmeRecord record;
  iter.fetchNext(&record);
  config.putRecord(&record);
}
success = config.store("smeman.xml.new");
if ( success )
{
  unlink("smeman.xml");
  rename("smeman.xml.new","smeman.xml");
}

*/
#ifndef SCAG_CONFIG_SMPPMAN_STRUCTURES
#define SCAG_CONFIG_SMPPMAN_STRUCTURES

#include <inttypes.h>
#include <util/cstrings.h>
#include <logger/Logger.h>
#include <string>

namespace scag {
namespace config {

using smsc::util::cStringCopy;
using smsc::logger::Logger;

typedef enum {SMPP_SME,SS7_SME} RecordType;
typedef enum {MODE_TX, MODE_RX, MODE_TRX} ModeType;

struct SmppSmeRecord
{
  uint8_t typeOfNumber;
  uint8_t numberingPlan;
  uint8_t interfaceVersion;
  char* systemType;
  char* password;
  char* addrRange;
  uint32_t smeN;
  bool wantAlias;
  bool forceDC;
  uint32_t timeout;
  uint32_t proclimit;
  uint32_t schedlimit;
  char* receiptSchemeName;
  bool disabled;
  ModeType mode;
  signed long providerId;
};

struct Ss7SmeRecord
{
  // ... nothing, now
};

struct SmeRecord
{
  RecordType rectype;
  int priority;
  char* smeUid;
  union
  {
    SmppSmeRecord smppSme;
    Ss7SmeRecord ss7Sme;
  }recdata;

  SmeRecord()
  {
    std::memset(this, 0, sizeof(*this));
    rectype = SMPP_SME;
  }

  SmeRecord(const SmeRecord & copy)
  {
    std::memcpy(this, &copy, sizeof(copy));
    smeUid = cStringCopy(copy.smeUid);
    if (rectype == SMPP_SME)
    {
      recdata.smppSme.systemType        = cStringCopy(copy.recdata.smppSme.systemType);
      recdata.smppSme.password          = cStringCopy(copy.recdata.smppSme.password);
      recdata.smppSme.addrRange         = cStringCopy(copy.recdata.smppSme.addrRange);
      recdata.smppSme.receiptSchemeName = cStringCopy(copy.recdata.smppSme.receiptSchemeName);
    } else {
      // !!! not yet implemented
    }
  }

  ~SmeRecord()
  {
    if (smeUid != 0) delete[] smeUid;
    if (rectype == SMPP_SME)
    {
      if (recdata.smppSme.systemType        != 0)  delete[] recdata.smppSme.systemType;
      if (recdata.smppSme.password          != 0)  delete[] recdata.smppSme.password;
      if (recdata.smppSme.addrRange         != 0)  delete[] recdata.smppSme.addrRange;
      if (recdata.smppSme.receiptSchemeName != 0)  delete[] recdata.smppSme.receiptSchemeName;
    } else {
      // !!! not yet implemented
    }
    std::memset(this, 0, sizeof(*this));
  }
};


}
}

namespace scag2 {
namespace config {
using scag::config::SmppSmeRecord;
using scag::config::SmeRecord;
}
}

#endif // ifndef SMSC_UTIL_CONFIG_SMEMAN_SMEMAN_STRUCTURES
