// 
// File:   CommandActions.h
// Author: loomox
//
// Created on 31 марта 2005 
//

#ifndef _SCAG_ADMIN_CommandActions_H
#define	_SCAG_ADMIN_CommandActions_H


#include "admin/protocol/Command.h"

namespace scag {
namespace admin {
namespace Actions {
/*
struct CommandActions
{
  bool restart;
  bool reloadroutes;
  CommandActions() : restart(false), reloadroutes(false) {};
};  */

}
}
}

namespace scag2 {
namespace admin {
namespace Actions = scag::admin::Actions;
}
}

#endif	/* _SCAG_ADMIN_CommandActions_H */

