#include "SbmCommandListener.h"

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

SbmCommandListener::SbmCommandListener(DistrListAdmin* dladmin_)
    : dladmin(dladmin_)
{
}

void SbmCommandListener::handle(const Command& command)
{
    
	switch( command.getType() ){
	case SBMADDSUBMITER_CMD:
		sbmAddSubmiter(command);
		break;
    case SBMDELETESUBMITER_CMD:
        sbmDeleteSubmiter(command);
		break;
	}
}

void SbmCommandListener::sbmAddSubmiter(const Command& command)
{
    std::string dlname;
    std::string address;
    File::offset_type offset;

    const SbmAddSubmiterCommand* cmd = dynamic_cast<const SbmAddSubmiterCommand*>(&command);

    cmd->getArgs(offset, dlname, address);

    BEGINMETHOD
    {
        dladmin->grantPosting(dlname.c_str(), address.c_str());
    }
    ENDMETHOD
}

void SbmCommandListener::sbmDeleteSubmiter(const Command& command)
{
    std::string dlname;
    std::string address;

    const SbmDeleteSubmiterCommand* cmd = dynamic_cast<const SbmDeleteSubmiterCommand*>(&command);

    cmd->getArgs(dlname, address);

    BEGINMETHOD
    {
        dladmin->revokePosting(dlname.c_str(), address.c_str());
    }
    ENDMETHOD
}

}
}
