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
namespace smppgw {
namespace admin {

using namespace smsc::util::xml;

Response * CommandStatusSme::CreateResponse(smsc::smppgw::Smsc * SmscApp)
{
  Variant result(smsc::admin::service::StringListType);
    for (std::auto_ptr<SmeIterator> i(SmscApp->getSmeAdmin()->iterator()); i.get() != NULL;)
    {
        std::string status;
        status += i->getSmeInfo().systemId;
        status += ",";
        if (i->getSmeInfo().internal)
        {
            status += "internal";
        }
        else
        {
            if (!i->isSmeConnected())
            {
                status += "disconnected";
            }
            else
            {
                SmeProxy * smeProxy = i->getSmeProxy();
                try {
                    std::string tmpStr;
                    switch (smeProxy->getBindMode())
                    {
                    case smeTX:
                        tmpStr += "tx,";
                        break;
                    case smeRX:
                        tmpStr += "rx,";
                        break;
                    case smeTRX:
                        tmpStr += "trx,";
                        break;
                    default:
                        tmpStr += "unknown,";
                    }
                    char inIP[128], outIP[128];
                    if (smeProxy->getPeers(inIP,outIP))
                    {
                        tmpStr += inIP;
                        tmpStr += ",";
                        tmpStr += outIP;
                    }
                    else
                    {
                        tmpStr += "unknown,unknown";
                    }
                    status += tmpStr;
                } catch (...) {
                    status += "unknown,unknown,unknown";
                }
            }
        }
        result.appendValueToStringList(status.c_str());
        if (!i->next())
            break;
    }
    return new Response(Response::Ok, result);
}


}
}
}
