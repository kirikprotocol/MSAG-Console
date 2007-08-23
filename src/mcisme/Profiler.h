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
  static bool     bDefaultInform, bDefaultNotify;
  static void     init(DataSource* _ds, uint8_t defaultEventMask=0xFF,
                       bool defaultInform=true, bool defaultNotify=false);

  static bool delProfile(const char* abonent, Connection* connection=0);
  static void setProfile(const char* abonent, const AbonentProfile& profile, Connection* connection=0);
  static AbonentProfile getProfile(const char* abonent, Connection* connection=0);
};

struct AbonentProfile
{
  uint8_t eventMask;
  bool    inform, notify;
  int8_t informTemplateId, notifyTemplateId; // if -1 => default
        
  AbonentProfile() 
    : eventMask(AbonentProfiler::defaultEventMask), 
      inform(AbonentProfiler::bDefaultInform), notify(AbonentProfiler::bDefaultNotify),
      informTemplateId(-1), notifyTemplateId(-1) {};
  AbonentProfile(const AbonentProfile& pro) 
    : eventMask(pro.eventMask), inform(pro.inform), notify(pro.notify), 
      informTemplateId(pro.informTemplateId), notifyTemplateId(pro.notifyTemplateId) {};
  AbonentProfile& operator=(const AbonentProfile& pro) {
    eventMask = pro.eventMask;
    inform = pro.inform; notify = pro.notify;
    informTemplateId = pro.informTemplateId; notifyTemplateId = pro.notifyTemplateId;
    return (*this);
  };
};

}}

#endif // SMSC_MCI_SME_PROFILER
