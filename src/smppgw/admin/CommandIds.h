// 
// File:   CommandIds.h
// Author: igork
//
// Created on 27 Май 2004 г., 15:59
//

#ifndef _SMPPGW_ADMIN_CommandIds_H
#define	_SMPPGW_ADMIN_CommandIds_H

#include "admin/protocol/Command.h"

namespace smsc {
namespace smppgw {
namespace admin {
  
namespace CommandIds {
  enum IDS {
    unknown = smsc::admin::protocol::Command::undefined,
    apply,
    updateSmeInfo,
    addSme,
    statusSme,
    deleteSme,
    traceRoute,
    loadRoutes,
    regSmsc
  };
}

}
}
}


#endif	/* _SMPPGW_ADMIN_CommandIds_H */

