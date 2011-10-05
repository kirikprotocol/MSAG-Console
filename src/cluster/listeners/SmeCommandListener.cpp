#include "SmeCommandListener.h"

namespace smsc {
namespace cluster {

SmeCommandListener::SmeCommandListener(smsc::smeman::SmeManager *smeman_)
    : smeman(smeman_)
{
}

void SmeCommandListener::handle(const Command& command)
{

  switch( command.getType() ){
    case SMEADD_CMD:
      smeAdd(command);
      break;
    case SMEREMOVE_CMD:
      smeRemove(command);
      break;
    case SMEUPDATE_CMD:
      smeUpdate(command);
      break;
    default:break;
  }
}

void SmeCommandListener::smeAdd(const Command& command)
{
    smsc::smeman::SmeInfo si;

    const SmeAddCommand* cmd = dynamic_cast<const SmeAddCommand*>(&command);

    cmd->getArgs(si);

    smeman->addSme(si);
}

void SmeCommandListener::smeRemove(const Command& command)
{
    char smeId[SMEID_LENGTH];

    const SmeRemoveCommand* cmd = dynamic_cast<const SmeRemoveCommand*>(&command);

    cmd->getArgs(smeId);

    smeman->deleteSme(smeId);
}

void SmeCommandListener::smeUpdate(const Command& command)
{
    smsc::smeman::SmeInfo si;

    const SmeUpdateCommand* cmd = dynamic_cast<const SmeUpdateCommand*>(&command);

    cmd->getArgs(si);

    smeman->updateSmeInfo(si.systemId, si);
}

}
}
