#include "MscCommandListener.h"
#include "mscman/MscManager.h"

namespace smsc {
namespace cluster {

using smsc::mscman::MscManager;
using namespace smsc::util;
using namespace smsc::core::buffers;

void MscCommandListener::handle(const Command& command)
{
  switch( command.getType() )
  {
    case MSCREGISTRATE_CMD:
      mscRegistrate(command);
      break;
    case MSCUNREGISTER_CMD:
      mscUnregistrate(command);
      break;
    case MSCBLOCK_CMD:
      mscBlock(command);
      break;
    case MSCCLEAR_CMD:
      mscClear(command);
      break;
    case MSCREPORT_CMD:
      mscReport(command);
      break;
  }
}

void MscCommandListener::mscRegistrate(const Command& command)
{
    char mscNum[22];
    File::offset_type offset;

    const MscRegistrateCommand* cmd = dynamic_cast<const MscRegistrateCommand*>(&command);

    cmd->getArgs(mscNum,offset);

    MscManager::getMscAdmin().registrate(mscNum);
}

void MscCommandListener::mscUnregistrate(const Command& command)
{
    char mscNum[22];

    const MscUnregisterCommand* cmd = dynamic_cast<const MscUnregisterCommand*>(&command);

    cmd->getArgs(mscNum);

    MscManager::getMscAdmin().unregister(mscNum);
}

void MscCommandListener::mscBlock(const Command& command)
{
    char mscNum[22];

    const MscBlockCommand* cmd = dynamic_cast<const MscBlockCommand*>(&command);

    cmd->getArgs(mscNum);

    MscManager::getMscAdmin().block(mscNum);
}

void MscCommandListener::mscClear(const Command& command)
{
    char mscNum[22];

    const MscClearCommand* cmd = dynamic_cast<const MscClearCommand*>(&command);

    cmd->getArgs(mscNum);

    MscManager::getMscAdmin().clear(mscNum);
}

void MscCommandListener::mscReport(const Command& command)
{
    char mscNum[22];
    bool status;
    File::offset_type offset;

    const MscReportCommand* cmd = dynamic_cast<const MscReportCommand*>(&command);

    cmd->getArgs(mscNum, status, offset);

    MscManager::getMscStatus().report(mscNum, status);
}

}
}
