#include "MemCommandListener.h"

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

MemCommandListener::MemCommandListener(DistrListAdmin* dladmin_)
    : dladmin(dladmin_)
{
}

void MemCommandListener::handle(const Command& command)
{

	switch( command.getType() ){
	case MEMADDMEMBER_CMD:
		memAddMember(command);
		break;
    case MEMDELETEMEMBER_CMD:
        memDeleteMember(command);
		break;
	}
}

void MemCommandListener::memAddMember(const Command& command)
{
    std::string dlname;
    std::string address;
    File::offset_type offset;

    const MemAddMemberCommand* cmd = dynamic_cast<const MemAddMemberCommand*>(&command);

    cmd->getArgs(offset, dlname, address);

    BEGINMETHOD
    {
        dladmin->addMember(dlname.c_str(), address.c_str());
    }
    ENDMETHOD
}

void MemCommandListener::memDeleteMember(const Command& command)
{
    std::string dlname;
    std::string address;

    const MemDeleteMemberCommand* cmd = dynamic_cast<const MemDeleteMemberCommand*>(&command);

    cmd->getArgs(dlname, address);

    BEGINMETHOD
    {
        dladmin->deleteMember(dlname.c_str(), address.c_str());
    }
    ENDMETHOD
}

}
}
