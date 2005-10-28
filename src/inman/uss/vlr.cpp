static char const ident[] = "$Id$";
#include <assert.h>

#include "inman/inap/infactory.hpp"
#include "inman/inap/dialog.hpp"
#include "inman/interaction/server.hpp"
#include "inman/interaction/ussmessages.hpp"
#include "inman/uss/vlr.hpp"
#include "inman/uss/ussdsm.hpp"

using smsc::inman::inap::Dialog;
using smsc::inman::inap::InSessionFactory;
using smsc::inman::interaction::SerializerITF;
using smsc::inman::interaction::SerializerUSS;
using smsc::inman::interaction::Server;
using smsc::inman::interaction::USSMessageBase;
using smsc::inman::interaction::USSRequestMessage;
using smsc::inman::interaction::USS2CMD;

namespace smsc  {
namespace inman {
namespace uss {

/* ************************************************************************** *
 * class VLR implementation:
 * ************************************************************************** */
VLR::VLR (const VLR_CFG * inCfg)
    : logger( Logger::getInstance("smsc.inman.vlr") )
    , cfg (*inCfg)
    , session( 0 )
    , dispatcher( 0 )
    , tcpServer( 0 )
{
    smsc_log_debug( logger, "Creating VLR" );

    //init INAP session factory
    InSessionFactory* factory = InSessionFactory::getInstance();
    assert( factory );

    //init TCAP dispatcher
    dispatcher = new Dispatcher();

    //init TCP server
    if (cfg.host) { //may be missed in case of local testing
        tcpServer = new Server(cfg.host, cfg.port, SerializerUSS::getInstance());
        tcpServer->addListener( this );
    }

    session = factory->openSession(cfg.usr_ssn, cfg.vlr_ssn, cfg.vlr_addr, cfg.in_ssn, cfg.in_addr);
    assert( session );
}

VLR::~VLR()
{
  if (running)
    stop();
  smsc_log_debug( logger, "Release VLR" );
  InSessionFactory* factory = InSessionFactory::getInstance();
  assert( factory );
  smsc_log_debug( logger, "ReleaseSession" );
  factory->closeSession( session );
  smsc_log_debug( logger, "Delete server" );
  if (tcpServer)
    delete tcpServer;
  smsc_log_debug( logger, "Delete dispatcher" );
  delete dispatcher;
}

const VLR_CFG & VLR::getCFG() const
{
    return cfg;
}

void VLR::start()
{
  smsc_log_debug( logger, "Start TCP server" );
  tcpServer->Start();
  smsc_log_debug( logger, "Start dispatcher" );
  dispatcher->Start();
  running = true;
}

void VLR::stop()
{
  smsc_log_debug(logger, "Stop TCP server" );
  tcpServer->Stop();
  tcpServer->WaitFor();
  smsc_log_debug( logger, "Stop dispatcher" );
  dispatcher->Stop();
  running = false;
  dispatcher->WaitFor();
}

//frees the DSM, which successfully processed request.
void VLR::onCommandProcessed(USSDSM* dsm)
{
    unsigned int reqId = dsm->getDSMId();

    USSDSMmap::iterator it = workers.find(reqId);
    if (it == workers.end())
        smsc_log_error(logger, "Attempt to free unregistered USSDSM, id: 0x%X", reqId);
    else
        workers.erase(reqId);
    delete dsm;
    smsc_log_debug(logger, "USSDSM deleted, id: 0x%X", reqId);
}
/* -------------------------------------------------------------------------- *
 * ServerListener interface implementation:
 * -------------------------------------------------------------------------- */
void VLR::onConnectOpened(Server*, Connect* connect)
{
    assert(connect);
    smsc_log_debug(logger, "New connection opened" );
    connect->addListener(this);
}

void VLR::onConnectClosed(Server*, Connect* connect)
{
    assert(connect);
    smsc_log_debug(logger, "Connection closed");
    connect->removeListener(this);
}

/* -------------------------------------------------------------------------- *
 * ConnectListener interface implementation:
 * -------------------------------------------------------------------------- */
void VLR::onCommandReceived(Connect* conn, SerializableObject* recvCmd)
{
    USSRequestMessage* cmd = static_cast<USSRequestMessage*>(recvCmd);
    assert( cmd );
    unsigned short cmdId = cmd->getObjectId();
    if (cmdId != USS2CMD::PROCESS_USS_REQUEST_TAG)
        smsc_log_error(logger, "Unknown command received: 0x%X", cmdId);
    else {
        unsigned int reqId = cmd->getDialogId();
        smsc_log_debug(logger, "USS Command received 0x%X, id = 0x%X", cmdId, reqId );

        USSDSMmap::iterator it = workers.find(reqId);
        if (it == workers.end()) {
            USSDSM* dsm = new USSDSM(this, reqId, session, conn);
            workers.insert( USSDSMmap::value_type( reqId, dsm ));
            dsm->onProcessUSSRequest(cmd);
        } else {
            smsc_log_error(logger, "USS Command 0x%X, id = 0x%X is already in process",
                             cmdId, reqId );
            USSDSM* dsm = (*it).second;
            dsm->onDenyUSSRequest(cmd);
            //NOTE: DSM is binded to other connect. Use this one to send response.
            conn->send(cmd);
        }
    }
}

/* -------------------------------------------------------------------------- *
 * Local testing utilities:
 * -------------------------------------------------------------------------- */

} // namespace uss
} // namespace inman
} // namespace smsc

