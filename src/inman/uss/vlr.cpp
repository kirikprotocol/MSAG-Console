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
    tcpServer = new Server(cfg.host, cfg.port, SerializerUSS::getInstance());
    tcpServer->addListener( this );

    session = factory->openSession(cfg.usr_ssn, cfg.vlr_ssn, cfg.vlr_addr, cfg.in_ssn, cfg.in_addr);
    assert( session );
}

VLR::~VLR()
{
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
  smsc_log_debug( logger, "Start dispatcher" );
  dispatcher->Start();
}

void VLR::stop()
{
  smsc_log_debug( logger, "Stop dispatcher" );
  dispatcher->Stop();
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
        unsigned int reqId = cmd->getReqId();
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
//constructor for local testing, no TCP interaction
VLR::VLR(UCHAR_T user_ssn, UCHAR_T vlr_ssn, const char* vlr_addr, 
         UCHAR_T in_ssn, const char* in_addr)
  : logger( Logger::getInstance("smsc.inman.vlr") )
  , session( 0 )
  , dispatcher( 0 )
  , tcpServer( 0 )
{
  smsc_log_debug( logger, "Create VLR" );

  cfg.host = NULL;
  cfg.port = 0;
  cfg.in_addr = in_addr;
  cfg.in_ssn = in_ssn;
  cfg.usr_ssn = user_ssn;
  cfg.vlr_addr = vlr_addr;
  cfg.vlr_ssn = vlr_ssn;

  InSessionFactory* factory = InSessionFactory::getInstance();
  assert( factory );

  dispatcher = new Dispatcher();

  session = factory->openSession(user_ssn, vlr_ssn, vlr_addr, in_ssn, in_addr );
  assert( session );
}


void VLR::make102(const char * who)
{
    USSRequestMessage   req;
    static unsigned char rstr[] = "*100#";
    req.setReqId(1);
    req.setDCS(0x0F);
    req.setUSSData(&rstr[0], 5);
    req.setMSISDNadr(who);

    onCommandReceived(NULL, &req);
}


} // namespace uss
} // namespace inman
} // namespace smsc

