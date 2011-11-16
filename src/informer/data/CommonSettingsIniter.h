#ifndef _INFORMER_COMMONSETTINGSINITER_H
#define _INFORMER_COMMONSETTINGSINITER_H

namespace smsc {
namespace util {
namespace config {
class Config;
}
}
}

namespace eyeline {
namespace informer {

class CommonSettings;
class SnmpManager;
class DeadLockWatcher;
class UTF8;

struct CommonSettingsIniter
{
    void init( CommonSettings& cs,
               smsc::util::config::Config& cfg,
               SnmpManager*                snmp,       // not owned
               DeadLockWatcher*            dlwatcher,
               UTF8*                       utf8,
               bool                        archive );

private:
    void loadTimezones( CommonSettings& cs );
};

}
}

#endif
