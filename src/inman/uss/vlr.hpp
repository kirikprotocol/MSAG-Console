#ident "$Id$"

#ifndef __SMSC_INMAN_USS_VLR__
#define __SMSC_INMAN_USS_VLR__

#include <map>

#include "logger/Logger.h"
#include "inman/inap/dispatcher.hpp"
#include "inman/interaction/server.hpp"
#include "inman/inap/session.hpp"

using smsc::inman::inap::Dispatcher;
using smsc::inman::inap::Session;
using smsc::inman::interaction::Server;
using smsc::inman::interaction::ServerListener;
using smsc::inman::interaction::ConnectListener;

namespace smsc  {
namespace inman {
namespace uss {

struct VLR_CFG {
    const char* vlr_addr;
    int         vlr_ssn;
    const char* in_addr;
    int         in_ssn;
    int         usr_ssn;
    const char* host;
    int         port;
};

//Main USS service: 
class USSDSM;
class VLR : public ServerListener, ConnectListener
{
  typedef std::map<unsigned int, USSDSM*> USSDSMmap;

  public:
    VLR (const VLR_CFG * inCfg);
    VLR( UCHAR_T vlr_ssn, const char* vlr_addr);
    VLR( UCHAR_T user_ssn, UCHAR_T vlr_ssn, const char* vlr_addr, UCHAR_T in_ssn, const char* in_addr);
    virtual ~VLR();

    const VLR_CFG & getCFG() const;
    //ServerListener interface
    virtual void onConnectOpened(Server*, Connect*);
    virtual void onConnectClosed(Server*, Connect*);

    //ConnectListener interface
    virtual void onCommandReceived(Connect*, SerializableObject*);

    //own methods
    virtual void start();
    virtual void stop();
    //frees the DSM, which successfully processed request.
    virtual void onCommandProcessed(USSDSM* dsm);

  private:
    VLR_CFG     cfg;
    USSDSMmap   workers;
    Logger*     logger;
    Session*    session;
    Dispatcher* dispatcher;
    Server*	tcpServer;
    volatile bool running;
};

}//namespace uss
}//namespace inman
}//namespace smsc

#endif
