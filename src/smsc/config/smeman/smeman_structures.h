/*
  $Id$
*/
#ifndef SMSC_UTIL_CONFIG_SMEMAN_SMEMAN_STRUCTURES
#define SMSC_UTIL_CONFIG_SMEMAN_SMEMAN_STRUCTURES

#include <inttypes.h>
#include <string>

#include "util/cstrings.h"
#include "logger/Logger.h"

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

*/


namespace smsc {
namespace config {
namespace smeman {

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
  //bool forceDC;
  uint32_t timeout;
  uint32_t proclimit;
  uint32_t schedlimit;
  char* receiptSchemeName;
  bool disabled;
  ModeType mode;
  signed long providerId;
  uint32_t accessMask;
  uint32_t flags;
};


struct SmeRecord
{
  RecordType rectype;
  int priority;
  char* smeUid;

  SmppSmeRecord smppSme;

  SmeRecord()
  {
    ::memset(this, 0, sizeof(*this));
    rectype = SMPP_SME;
    smppSme.accessMask=1;
  }

  SmeRecord(const SmeRecord & copy)
  {
    ::memcpy(this, &copy, sizeof(copy));
    smeUid = cStringCopy(copy.smeUid);
    smppSme.systemType        = cStringCopy(copy.smppSme.systemType);
    smppSme.password          = cStringCopy(copy.smppSme.password);
    smppSme.addrRange         = cStringCopy(copy.smppSme.addrRange);
    smppSme.receiptSchemeName = cStringCopy(copy.smppSme.receiptSchemeName);
  }

  ~SmeRecord()
  {
    if (smeUid != 0) delete[] smeUid;
      if (smppSme.systemType        != 0)  delete[] smppSme.systemType;
      if (smppSme.password          != 0)  delete[] smppSme.password;
      if (smppSme.addrRange         != 0)  delete[] smppSme.addrRange;
      if (smppSme.receiptSchemeName != 0)  delete[] smppSme.receiptSchemeName;
    ::memset(this, 0, sizeof(*this));
  }
};


}
}
}

#endif // ifndef SMSC_UTIL_CONFIG_SMEMAN_SMEMAN_STRUCTURES
