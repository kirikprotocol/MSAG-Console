// 
// File:   CommandActions.h
// Author: loomox
//
// Created on 31 марта 2005 
//

#ifndef _SMPPGW_ADMIN_CommandActions_H
#define	_SMPPGW_ADMIN_CommandActions_H


#include "admin/protocol/Command.h"

namespace smsc {
namespace smppgw {
namespace admin {
namespace Actions {

struct CommandActions
{
  bool restart;
  bool reloadconfig;
  CommandActions();
};


}
}
}
}


#endif	/* _SMPPGW_ADMIN_CommandActions_H */

