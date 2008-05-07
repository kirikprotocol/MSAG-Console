
#include "Profiler.h"

namespace smsc {
namespace mcisme {

DataSource* AbonentProfiler::ds     = 0;
Logger*     AbonentProfiler::logger = 0;
uint8_t     AbonentProfiler::defaultEventMask = 0xFF;
bool        AbonentProfiler::bDefaultInform = true;
bool        AbonentProfiler::bDefaultNotify = false;
bool        AbonentProfiler::bDefaultWantNotifyMe=false;

/* ----------------------- Main logic implementation (static functions) --------------------------------- */

void AbonentProfiler::init(DataSource* _ds, uint8_t _defaultEventMask,
                           bool defaultInform, bool defaultNotify, bool defaultWantNotifyMe)
{
  AbonentProfiler::logger = Logger::getInstance("smsc.mcisme.AbonentProfiler");
  AbonentProfiler::defaultEventMask = _defaultEventMask;
  AbonentProfiler::bDefaultInform = defaultInform;
  AbonentProfiler::bDefaultNotify = defaultNotify;
  AbonentProfiler::bDefaultWantNotifyMe = defaultWantNotifyMe;
  AbonentProfiler::ds = _ds;
}

}}
