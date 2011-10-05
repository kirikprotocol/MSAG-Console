#include "PrcCommandListener.h"

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

using smsc::distrlist::Principal;
using namespace smsc::util;
using namespace smsc::core::buffers;

namespace smsc {
namespace cluster {

PrcCommandListener::PrcCommandListener(DistrListAdmin* dladmin_)
    : dladmin(dladmin_)
{
}

void PrcCommandListener::handle(const Command& command)
{

	switch( command.getType() ){
	  case PRCADDPRINCIPAL_CMD:
	    prcAddPrincipal(command);
	    break;
	  case PRCDELETEPRINCIPAL_CMD:
	    prcDeletePrincipal(command);
	    break;
	  case PRCALTERPRINCIPAL_CMD:
	    prcAlterPrincipal(command);
	    break;
    default:break;
	}
}

void PrcCommandListener::prcAddPrincipal(const Command& command)
{
    int maxLists;
    int maxElements;
    std::string address;
    File::offset_type offset;

    const PrcAddPrincipalCommand* cmd = dynamic_cast<const PrcAddPrincipalCommand*>(&command);

    cmd->getArgs(maxLists, maxElements, offset, address);

    BEGINMETHOD
    {
        dladmin->addPrincipal(Principal(address.c_str() ,maxLists, maxElements));
    }
    ENDMETHOD
}

void PrcCommandListener::prcDeletePrincipal(const Command& command)
{
    std::string address;

    const PrcDeletePrincipalCommand* cmd = dynamic_cast<const PrcDeletePrincipalCommand*>(&command);

    cmd->getArgs(address);

    BEGINMETHOD
    {
        dladmin->deletePrincipal(address.c_str());
    }
    ENDMETHOD
}

void PrcCommandListener::prcAlterPrincipal(const Command& command)
{

    int maxLists;
    int maxElements;
    std::string addresses;

    const PrcAlterPrincipalCommand* cmd = dynamic_cast<const PrcAlterPrincipalCommand*>(&command);

    cmd->getArgs(maxLists, maxElements, addresses);

    BEGINMETHOD
    {
        dladmin->changePrincipal(Principal(addresses.c_str(), maxLists, maxElements));
    }
    ENDMETHOD
}

}
}
