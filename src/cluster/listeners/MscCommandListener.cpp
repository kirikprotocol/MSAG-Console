#include "MscCommandListener.h"
#include "mscman/MscManager.h"

namespace smsc {
namespace cluster {

using smsc::mscman::MscManager;

void MscCommandListener::handle(const Command& command)
{
	switch( command.getType() ){
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
	}
}

void MscCommandListener::mscRegistrate(const Command& command)
{
    char mscNum[22];

    const MscRegistrateCommand* cmd = dynamic_cast<const MscRegistrateCommand*>(&command);

    cmd->getArgs(mscNum);

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

}
}
