// 
// File:   CommandStatusSme.cpp
// Author: Vitaly
//
// Created on 05.04.05
//

#include "CommandStatusSme.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"
#include "admin/service/Variant.h"
#include "admin/service/Type.h"

namespace smsc {
namespace scag {
namespace admin {

using namespace smsc::util::xml;

Response * CommandStatusSme::CreateResponse(smsc::scag::Smsc * SmscApp)
{
  Variant result(smsc::admin::service::StringListType);
  ((SmeManager*)(SmscApp->getSmeAdmin()))->statusSme(result);

    return new Response(Response::Ok, result);
}


}
}
}
