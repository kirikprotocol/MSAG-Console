#include "DlCommandListener.h"

#define BEGINMETHOD\
  try
#define  ENDMETHOD \
  catch(std::exception& e) \
  { \
    throw Exception("%s failed:\"%s\"",__func__,e.what());\
  } \
  catch(const char* e) \
  { \
    throw Exception("%s failed:\"%s\"",__func__,e);\
  } \
  catch(...) \
  { \
    throw Exception("%s failed: Unknown exception",__func__);\
  }

using namespace smsc::util;
using namespace smsc::core::buffers;

namespace smsc {
namespace cluster {

DlCommandListener::DlCommandListener(DistrListAdmin* dladmin_)
    : dladmin(dladmin_)
{
}

void DlCommandListener::handle(const Command& command)
{

	switch( command.getType() ){
	case DLADD_CMD:
		dlAdd(command);
		break;
    case DLDELETE_CMD:
        dlDelete(command);
		break;
    case DLALTER_CMD:
        dlAlter(command);
		break;
	}
}

void DlCommandListener::dlAdd(const Command& command)
{
    int maxElements;
    std::string dlname;
    std::string owner;
    File::offset_type offset1;
    File::offset_type offset2;

    const DlAddCommand* cmd = dynamic_cast<const DlAddCommand*>(&command);

    cmd->getArgs(maxElements, offset1, offset2, dlname, owner);

    /*printf("\n============== dlAdd ==============\n");
    printf("type: %d\n", cmd->getType());
    printf("maxElements: %d\n", maxElements);
    printf("dlname: %s\n", dlname.c_str());
    printf("owner: %s\n\n", owner.c_str());
    return;*/

    BEGINMETHOD
    {
        dladmin->addDistrList(dlname.c_str(),!owner.length(), owner.length() && owner.length() ? owner.c_str() : "0", maxElements);
    }
    ENDMETHOD
}

void DlCommandListener::dlDelete(const Command& command)
{

    std::string dlname;

    const DlDeleteCommand* cmd = dynamic_cast<const DlDeleteCommand*>(&command);

    cmd->getArgs(dlname);

    /*printf("\n============== dlDelete ==============\n");
    printf("dlname: %s\n", dlname.c_str());
    return;*/
    
    BEGINMETHOD
    {
        dladmin->deleteDistrList(dlname.c_str());
    }
    ENDMETHOD


}

void DlCommandListener::dlAlter(const Command& command)
{
    int maxElements;
    std::string dlname;

    const DlAlterCommand* cmd = dynamic_cast<const DlAlterCommand*>(&command);

    cmd->getArgs(maxElements, dlname);

    /*printf("\n============== dlAlter ==============\n");
    printf("dlname: %s\n", dlname.c_str());
    return;*/

    BEGINMETHOD
    {
        dladmin->changeDistrList(dlname.c_str(), maxElements);
    }
    ENDMETHOD
}


}
}
