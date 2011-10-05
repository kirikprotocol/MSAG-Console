#include "ProfileCommandListener.h"
#include "cluster/Commands.h"
#include "sms/sms.h"
#include "util/Exception.hpp"
#include "profiler/profiler-types.hpp"

namespace smsc {
namespace cluster {

using smsc::sms::Address;
using smsc::util::Exception;

using smsc::profiler::ProfilerInterface;
using smsc::profiler::Profile;

ProfileCommandListener::ProfileCommandListener(smsc::profiler::Profiler * profiler_)
    : profiler(profiler_),
    logger(smsc::logger::Logger::getInstance("smsc.cluster.ProfileCommandListener"))
{
}

void ProfileCommandListener::handle(const Command& command)
{
  switch( command.getType() ){
    case PROFILEUPDATE_CMD:
      profileUpdate(command);
      break;
    case PROFILEDELETE_CMD:
      profileDelete(command);
      break;
    default:break;
  }
}

bool isMask(const Address & address)
{
  for (unsigned i=0; i<address.length; i++)
  {
    if (address.value[i] == '?')
      return true;
  }
  return false;
}

void ProfileCommandListener::profileUpdate(const Command& command)
{
    uint8_t plan;
    uint8_t type;
    char address[21];
    smsc::profiler::Profile profile;

    const ProfileUpdateCommand* cmd = dynamic_cast<const ProfileUpdateCommand*>(&command);

    cmd->getArgs(profile, plan, type, address);


    try {

      Address addr((uint8_t)strlen(address), type, plan, address);

#ifdef SMSC_DEBUG
      char addr_str[smsc::sms::MAX_ADDRESS_VALUE_LENGTH+9];
      addr.toString(addr_str, sizeof(addr_str)/sizeof(addr_str[0]));
      smsc_log_debug(logger, "Address: \"%s\", codepage:%u, report options:%u, locale:%s, hide:%u, hideModif:%u", addr_str, profile.codepage, profile.reportoptions, profile.locale.c_str(), profile.hide, profile.hideModifiable);
#endif

      ProfilerInterface *proInterface = profiler;

      if (isMask(addr))
        proInterface->updatemask(addr, profile);
      else
        proInterface->update(addr, profile);

    }
    catch (...)
    {
        throw Exception("Exception of profileUpdate");
    }
}

void ProfileCommandListener::profileDelete(const Command& command)
{
    uint8_t plan, type;
    char address[21];

    const ProfileDeleteCommand* cmd = dynamic_cast<const ProfileDeleteCommand*>(&command);

    cmd->getArgs(plan, type, address);

    try {

        ProfilerInterface *proInterface = profiler;

        Address addr((uint8_t)strlen(address), plan, type, address);
        proInterface->remove(addr);
    }catch (...)
    {
        throw Exception("Exception of profileDelete");
    }
}

}
}
