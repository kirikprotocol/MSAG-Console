#include "SMSCAdminCommands.h"
#include "util/xml/utilFunctions.h"


namespace scag { namespace admin {

using namespace smsc::util::xml;

//================================================================
//================ Smsc commands =================================

void Abstract_CommandSmsc::init()
{

    smsc_log_info(logger, "Abstract_CommandSmsc got parameters:");

    smppEntityInfo.timeOut = -1;
    smppEntityInfo.uid = -1;
    smppEntityInfo.port = -1;
    smppEntityInfo.altPort = -1;
    smppEntityInfo.systemType = "";

    BEGIN_SCAN_PARAMS

    GETSTRPARAM((char*)smppEntityInfo.systemId,      "systemId")
//    GETSTRPARAM((char*)smppEntityInfo.password,      "password")
    GETSTRPARAM((char*)smppEntityInfo.bindSystemId,  "bindSystemId")
    GETSTRPARAM((char*)smppEntityInfo.bindPassword,  "bindPassword")
    GETSTRPARAM((char*)smppEntityInfo.systemType,    "systemType")
    GETINTPARAM(smppEntityInfo.timeOut,              "timeout")
    GETINTPARAM(smppEntityInfo.uid,                  "uid")

    if (::strcmp("mode", name) == 0)
    {
        if (::strcmp("trx", value.get()) == 0) smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
        else if(::strcmp("tx", value.get()) == 0) smppEntityInfo.bindType = scag::transport::smpp::btTransmitter;
        else if(::strcmp("rx", value.get()) == 0) smppEntityInfo.bindType = scag::transport::smpp::btReceiver;
        else smppEntityInfo.bindType = scag::transport::smpp::btTransceiver;
        smsc_log_info(logger, "mode: %s, %d", value.get(), smppEntityInfo.bindType);
    }

    GETSTRPARAM((char*)smppEntityInfo.host,         "host")
    GETINTPARAM(smppEntityInfo.port,                "port")
    GETSTRPARAM((char*)smppEntityInfo.altHost,      "altHost")
    GETINTPARAM(smppEntityInfo.altPort,             "altPort")
    GETSTRPARAM((char*)smppEntityInfo.addressRange, "addressRange")

    END_SCAN_PARAMS


    smppEntityInfo.type = scag::transport::smpp::etSmsc;

    std::string errorStr;

    if (smppEntityInfo.systemId == "") errorStr = "Unknown 'systemId' parameter";
//    if (smppEntityInfo.password == "") errorStr = "Unknown 'password' parameter";
    if (smppEntityInfo.bindSystemId == "") errorStr = "Unknown 'bindSystemId' parameter";
    if (smppEntityInfo.bindPassword == "") errorStr = "Unknown 'bindPassword' parameter";

    if (smppEntityInfo.timeOut == -1) errorStr = "Unknown 'timeOut' parameter";
    if (smppEntityInfo.uid == -1) errorStr = "Unknown 'uid' parameter";

    if (smppEntityInfo.host == "") errorStr = "Unknown 'host' parameter";
    if (smppEntityInfo.port <= 0) errorStr = "Invalid 'port' parameter";
//    if (smppEntityInfo.altHost == "") errorStr = "Unknown 'altHost' parameter";
//    if (smppEntityInfo.altPort == -1) errorStr = "Unknown 'altPort' parameter";

    if (errorStr.size() > 0)
    {
        smsc_log_error(logger, errorStr.c_str());
        throw AdminException(errorStr.c_str());
    }

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

Response * CommandDeleteSmsc::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandDeleteSmsc is processing");
    if(!ScagApp) throw Exception("Scag undefined");
    scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

    if(!smppMan) throw Exception("SmppManager undefined");

    try {
        smppMan->deleteSmppEntity(systemId.c_str());
    }
    catch(Exception& e) {
        char msg[1024];
        sprintf(msg, "Failed to delete smsc. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to delete smsc. Unknown exception");
        throw AdminException("Failed to delete smsc. Unknown exception");
    }

    smsc_log_info(logger, "CommandDeleteSmsc is processed ok");
    return new Response(Response::Ok, "CommandDeleteSmsc is processed ok");
}

//================================================================

Response * CommandAddSmsc::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandAddSmsc is processing...");
    if(!ScagApp) throw Exception("Scag undefined");

    scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
    if(!smppMan) throw Exception("SmppManager undefined");

    try {
        smppMan->addSmppEntity(getSmppEntityInfo());
    }
    catch(std::exception& e) {
        char msg[1024];
        sprintf(msg, "Failed to add smsc. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to add smsc. Unknown exception");
        throw AdminException("Failed to add smsc. Unknown exception");
    }

    smsc_log_info(logger, "CommandAddSmsc is processed ok");
    return new Response(Response::Ok, "none");
}

//================================================================

Response * CommandUpdateSmsc::CreateResponse(scag::Scag * ScagApp)
{
    smsc_log_info(logger, "CommandUnregSmsc is processing...");

    if (!ScagApp) throw Exception("Scag undefined");

    scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();
    if (!smppMan) throw Exception("SmppManager undefined");

    try {
        smppMan->updateSmppEntity(getSmppEntityInfo());
    }
    catch(Exception& e) {
        char msg[1024];
        sprintf(msg, "Failed to update smsc. Details: %s", e.what());
        smsc_log_error(logger, msg);
        return new Response(Response::Error, msg);
    } catch (...) {
        smsc_log_warn(logger, "Failed to update smsc. Unknown exception");
        throw AdminException("Failed to update smsc. Unknown exception");
    }

    smsc_log_info(logger, "CommandUpdateSmsc is processed");
    return new Response(Response::Ok, "none");
}


}}
