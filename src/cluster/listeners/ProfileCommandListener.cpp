#include "ProfileCommandListener.h"
#include "cluster/Commands.h"
#include "sms/sms.h"
#include "util/Exception.hpp"

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
    int codePage;
    int reportOption;
    int hideOption;

    bool hideModifaible;
    bool divertModifaible;
    bool udhContact;
    bool translit;

    bool divertActive;
    bool divertActiveAbsent;
    bool divertActiveBlocked;
    bool divertActiveBarred;
    bool divertActiveCapacity;

    std::string local;
    std::string divert;

    const ProfileUpdateCommand* cmd = dynamic_cast<const ProfileUpdateCommand*>(&command);

    cmd->getArgs(plan, type, address, codePage, reportOption, hideOption, hideModifaible, divertModifaible,
                        udhContact, translit, divertActive, divertActiveAbsent, divertActiveBlocked,
                        divertActiveBarred, divertActiveCapacity, local, divert);


    try {
    
      Address addr(strlen(address), plan, type, address);
      Profile profile;

      profile.codepage = codePage;
      profile.reportoptions = reportOption;
      profile.hide = hideOption;
      profile.locale = local;
      profile.hideModifiable = hideModifaible;
      profile.divert = divert;
      
      profile.divertActive         =  divertActive;
      profile.divertActiveAbsent   =  divertActiveAbsent;
      profile.divertActiveBlocked  =  divertActiveBlocked;
      profile.divertActiveBarred   =  divertActiveBarred;
      profile.divertActiveCapacity =  divertActiveCapacity;

      profile.divertModifiable = divertModifaible;
      profile.udhconcat = udhContact;
      profile.translit = translit;

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

        Address addr(strlen(address), plan, type, address);
        proInterface->remove(addr);
    }catch (...)
    {
        throw Exception("Exception of profileDelete");
    }
}

}
}
