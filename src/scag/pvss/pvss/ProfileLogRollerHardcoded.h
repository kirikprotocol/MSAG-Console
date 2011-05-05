#ifndef PVSS_PROFILELOGROLLERHARDCODED_H
#define PVSS_PROFILELOGROLLERHARDCODED_H

#include "scag/pvss/profile/ProfileLog.h"

namespace scag2 {
namespace pvss {

// profile log roller for pvss.
// configuration is kept is section 'PVSS.backup'
// Example:
// <section name="PVSS">
//    <section name="backup">
//      <param name="backupPrefix" type="string">storage/pvss/input/a.log</param>
//      <param name="finalSuffix" type="string">.pvss</param>
//      <param name="rollingInterval" type="int">300</param>
//      <param name="configReloadInterval" type="int">60</param>
//    </section>
// </section>

class ProfileLogRollerHardcoded : public ProfileLogRoller
{
public:
    ProfileLogRollerHardcoded( bool backupMode );
    virtual ~ProfileLogRollerHardcoded() {
        stop();
        WaitFor();
    }
    virtual bool readConfiguration();

private:
    time_t                              lastConfigTime_;
    time_t                              oldmtime_; // previous mtime of config
    bool                                backupMode_;
};

}
}

#endif
