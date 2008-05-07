#ifndef SMSC_MCI_SME_PROFILER
#define SMSC_MCI_SME_PROFILER

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <string>

#include <logger/Logger.h>
#include <db/DataSource.h>

namespace smsc {
namespace mcisme {

using namespace smsc::db;
using smsc::logger::Logger;

class AbonentProfile;
class AbonentProfiler
{
private:

  static DataSource*  ds;
  static Logger*  logger;

public:

  static uint8_t  defaultEventMask;
  static bool     bDefaultInform, bDefaultNotify, bDefaultWantNotifyMe;
  static void     init(DataSource* _ds, uint8_t defaultEventMask,
                       bool defaultInform, bool defaultNotify, bool defaultWantNotifyMe);
};

struct AbonentProfile
{
  uint8_t eventMask;
  bool    inform, notify, wantNotifyMe;
  int8_t informTemplateId, notifyTemplateId; // if -1 => default

  AbonentProfile()
    : eventMask(AbonentProfiler::defaultEventMask),
      inform(AbonentProfiler::bDefaultInform), notify(AbonentProfiler::bDefaultNotify),
      wantNotifyMe(AbonentProfiler::bDefaultWantNotifyMe),
      informTemplateId(-1), notifyTemplateId(-1) {}
};

}}

#endif // SMSC_MCI_SME_PROFILER
