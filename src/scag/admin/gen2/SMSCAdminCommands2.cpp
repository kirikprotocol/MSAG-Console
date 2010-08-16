#include "SMSCAdminCommands2.h"
#include "util/xml/utilFunctions.h"


namespace scag2 {
namespace admin {

using namespace smsc::util;
using namespace smsc::util::xml;

//================================================================
//================ Smsc commands =================================

void Abstract_CommandSmsc::init()
{

    smsc_log_info(logger, "Abstract_CommandSmsc got parameters:");

    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(systemId,      "systemId")
    END_SCAN_PARAMS
}

//================================================================

void CommandDeleteSmsc::init()
{
    smsc_log_info(logger, "CommandDeleteSme got parameters:");

    BEGIN_SCAN_PARAMS
    GETSTRPARAM_(systemId, "systemId")
    END_SCAN_PARAMS

    if (systemId == "")
    {
        smsc_log_error(logger, "Unknown 'systemId' parameter");
        throw AdminException("Unknown 'systemId' parameter");
    }
}

Response * CommandDeleteSmsc::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandDeleteSmsc is processing");
    if(!ScagApp) throw Exception("Scag undefined");
    transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

    if(!smppMan) throw Exception("SmppManager undefined");

    try {
        smppMan->deleteSmppEntity(systemId.c_str());
    }
    catch(Exception& e) {
        char msg[1024];
        sprintf(msg, "Failed to delete smsc. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to delete smsc. Unknown exception");
        throw AdminException("Failed to delete smsc. Unknown exception");
    }

    smsc_log_info(logger, "CommandDeleteSmsc is processed ok");
    return new Response(Response::Ok, "CommandDeleteSmsc is processed ok", responseEncoding);
}

//================================================================

Response * CommandAddSmsc::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandAddSmsc is processing...");
    if(!ScagApp) throw Exception("Scag undefined");

    transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
    if(!smppMan) throw Exception("SmppManager undefined");

    transport::smpp::SmppEntityInfo info;
    if(!smppMan->LoadEntityFromConfig(info,systemId.c_str(),transport::smpp::etSmsc))
    {
      throw Exception("Failed to load entity from config:'%s'",systemId.c_str());
    }

    try {
        smppMan->addSmppEntity(info);
    }
    catch(std::exception& e) {
        char msg[1024];
        sprintf(msg, "Failed to add smsc. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to add smsc. Unknown exception");
        throw AdminException("Failed to add smsc. Unknown exception");
    }

    smsc_log_info(logger, "CommandAddSmsc is processed ok");
    return new Response(Response::Ok, "none", responseEncoding);
}

//================================================================

Response * CommandUpdateSmsc::CreateResponse(Scag * ScagApp)
{
    smsc_log_info(logger, "CommandUnregSmsc is processing...");

    if (!ScagApp) throw Exception("Scag undefined");

    transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
    if (!smppMan) throw Exception("SmppManager undefined");

    transport::smpp::SmppEntityInfo info;
    if(!smppMan->LoadEntityFromConfig(info,systemId.c_str(),transport::smpp::etSmsc))
    {
      throw Exception("Failed to load entity from config:'%s'",systemId.c_str());
    }

    try {
        smppMan->updateSmppEntity(info);
    }
    catch(Exception& e) {
        char msg[1024];
        sprintf(msg, "Failed to update smsc. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg, responseEncoding);
    } catch (...) {
        smsc_log_warn(logger, "Failed to update smsc. Unknown exception");
        throw AdminException("Failed to update smsc. Unknown exception");
    }

    smsc_log_info(logger, "CommandUpdateSmsc is processed");
    return new Response(Response::Ok, "none", responseEncoding);
}


}}
