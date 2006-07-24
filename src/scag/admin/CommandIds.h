// 
// File:   CommandIds.h
// Author: igork
//
// Created on 27 ��� 2004 �., 15:59
//

#ifndef _SCAG_ADMIN_CommandIds_H
#define _SCAG_ADMIN_CommandIds_H

#include "admin/protocol/Command.h"

namespace scag {
namespace admin {
  
namespace CommandIds {
  enum IDS {
    unknown = smsc::admin::protocol::Command::undefined,
    updateSmeInfo,
    addSme,
    statusSme,
    deleteSme,
    applyConfig,
    applySmppRoutes,
    loadSmppTraceRoutes,
    traceSmppRoute,
    addSmsc,
    deleteSmsc,
    updateSmsc,
    updateRule,
    removeRule,
    addRule,
    reloadOperators,
    reloadServices,
    reloadTariffMatrix,
    applyHttpRoutes,
    loadHttpTraceRoutes,
    traceHttpRoute,
    getLogCategories    
  };
}

}
}


#endif  /* _SCAG_ADMIN_CommandIds_H */

