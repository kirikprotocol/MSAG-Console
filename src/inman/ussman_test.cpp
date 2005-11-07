static char const ident[] = "$Id$";
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "inman/common/console.hpp"
#include "inman/common/util.hpp"
#include "inman/interaction/ussmessages.hpp"

using smsc::logger::Logger;
using smsc::core::threads::Thread;
using smsc::inman::common::dump;
using smsc::inman::common::Console;
using smsc::inman::interaction::ObjectPipe;
using smsc::inman::interaction::SerializerUSS;
using smsc::inman::interaction::USS2CMD;
using smsc::inman::interaction::USSRequestMessage;
using smsc::inman::interaction::USSResultMessage;
using smsc::inman::interaction::USSDATA_T;


class USSFacade : public Thread
{
protected:
    unsigned        _reqId;
    Socket*         socket;
    ObjectPipe*     pipe;
    Logger*         logger;

public:
    USSFacade(Socket* sock) 
        : socket(sock)
        , pipe(new ObjectPipe(sock, SerializerUSS::getInstance(), ObjectPipe::frmLengthPrefixed))
        , logger(Logger::getInstance("smsc.USSFacade"))
        , _reqId(0)
        { 
            pipe->setLogger(logger);
        }

    virtual ~USSFacade() { delete pipe; delete socket; }

    unsigned getNextReqId(void)
    {
        return ++_reqId;
    }
    USSRequestMessage* composeRequest(USSRequestMessage* req, 
                                      const unsigned char * rstr, const char * who)
    {
        req->setObjectId(USS2CMD::PROCESS_USS_REQUEST_TAG);
        req->setDialogId(getNextReqId());
        req->setUSSData(rstr, strlen((const char*)rstr));
        req->setMSISDNadr(who);
        fprintf(stdout, "Request[%d]: requesting %s for %s\n", req->getDialogId(), rstr, who);
        return req;
    }

    void sendRequest(const unsigned char * rstr, const char * who)
    {
        USSRequestMessage req;
        
        pipe->send(composeRequest(&req, rstr, who));
    }

    void onRequestResult(USSResultMessage* res)
    {
        assert(res->getObjectId() == USS2CMD::PROCESS_USS_RESULT_TAG);
        unsigned short status = res->getStatus();
        switch (status) {
        case USS2CMD::STATUS_USS_REQUEST_OK: {
            std::string  str;
            if (!res->getUSSDataAsLatin1Text(str))
                str = dump((res->getUSSData()).size(), (unsigned char*)&(res->getUSSData())[0], 0);
            fprintf(stdout, "Request[%d]: got result %s\n", res->getDialogId(), str.c_str());
        } break;

        case USS2CMD::STATUS_USS_REQUEST_DENIAL: {
            fprintf(stdout, "Request[%d]: got result status: STATUS_USS_REQUEST_DENIAL\n", res->getDialogId());
        } break;

        case USS2CMD::STATUS_USS_REQUEST_FAILED: {
            fprintf(stdout, "Request[%d]: got result status: STATUS_USS_REQUEST_FAILED\n", res->getDialogId());
        } break;

        default:
            fprintf(stdout, "Request[%d]: got unknown result status\n", res->getDialogId());
        }
    }

    virtual int  Execute()
    {
        for(;;) {
            fd_set  read;
            FD_ZERO( &read );
            FD_SET( socket->getSocket(), &read );
            int n = select(  socket->getSocket()+1, &read, 0, 0, 0 );
            if ( n > 0 ) {
                USSResultMessage* cmd = static_cast<USSResultMessage*>(pipe->receive());
                assert( cmd );
                onRequestResult(cmd);
            }
        }
        return 0;
    }

};

static USSFacade*   _pFacade = 0;
static std::string  _msAdr("79139343290"); //100
                                           //79139859489 - 102


/* 
 * Console commands:
 */

void cmd_set_msadr(Console&, const std::vector<std::string> &args)
{
    if (args.size() < 2)
        fprintf(stdout, "USAGE: set_msadr adr_string\n");
    _msAdr = args[1];
    fprintf(stdout, "Setting ms ISDN adr to %s\n", _msAdr.c_str());
}

void cmd_get_msadr(Console&, const std::vector<std::string> &args)
{
    fprintf(stdout, "Current ms ISDN adr is %s\n", _msAdr.c_str());
}

void cmd_req100(Console&, const std::vector<std::string> &args)
{
    _pFacade->sendRequest((const unsigned char*)"*100#", _msAdr.c_str());
}

void cmd_req102(Console&, const std::vector<std::string> &args)
{
    _pFacade->sendRequest((const unsigned char*)"*102#", _msAdr.c_str());
}


int main(int argc, char** argv)
{
    if ( argc < 3 ) {
        fprintf( stderr, "Usage: %s <host> <port>\n", argv[0] );
        exit(1);
    }

    const char* host = argv[1];
    int         port = atoi(argv[2]);

    Logger::Init();

    fprintf(stdout, "Connecting to USS manager at %s:%d...\n", host, port);
    try {
        Socket* sock = new Socket();

        if (sock->Init(host, port, 1000)) {
            fprintf(stderr, "Can't init socket: %s (%d)\n", strerror(errno), errno);
            throw std::runtime_error("Can't init socket");
        }

        if (sock->Connect()) {
            fprintf(stderr, "Can't connect socket: %s (%d)\n", strerror( errno), errno);
            throw std::runtime_error("Can't connect socket");
        }

        _pFacade = new USSFacade(sock);

        Console console;
        console.addItem("req100", cmd_req100);
        console.addItem("req102", cmd_req102);
        console.addItem("get_msadr", cmd_get_msadr);
        console.addItem("set_msadr", cmd_set_msadr);

        _pFacade->Start();

        console.run("ussman>");

    } catch(const std::exception& error) {
        fprintf(stderr, "Fatal error: %s\n", error.what());
    }

    delete _pFacade;
    exit(0);
}