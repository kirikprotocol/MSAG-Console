static char const ident[] = "$Id$";
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "logger/Logger.h"
#include "inman/common/console.hpp"
#include "inman/interaction/messages.hpp"
#include "core/threads/Thread.hpp"

using smsc::core::threads::Thread;
using smsc::inman::common::Console;
using smsc::inman::interaction;
using smsc::inman::interaction::ObjectPipe;
using smsc::inman::interaction::SerializerInap;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::SmscHandler;
using smsc::inman::interaction::SmscCommand;
using smsc::inman::interaction::DeliverySmsResult_t;


class Facade : public Thread, public SmscHandler
{
protected:
    unsigned            dialogId;
    Socket*             socket;
    ObjectPipe*         pipe;
    Logger*             logger;
    DeliverySmsResult_t delivery;

public:
    Facade(Socket* sock) 
        : socket( sock )
        , pipe( new ObjectPipe( sock, SerializerInap::getInstance(), ObjectPipe::frmLengthPrefixed ) )
        , logger( Logger::getInstance("smsc.InFacade") )
        , delivery( smsc::inman::interaction::DELIVERY_SUCCESSED )
        , dialogId(0)
        { 
            pipe->setLogger(logger);
        }

    virtual ~Facade() { delete pipe; delete socket; }

    unsigned getNextDialogId(void) { return ++dialogId; }

    void sendChargeSms(DeliverySmsResult_t deliveryStatus)
    {
        //set delivery status to send on ChargeSmsResult
        delivery = deliveryStatus;
        //compose ChargeSms
        ChargeSms op;

        op.setDialogId(getNextDialogId());

        op.setDestinationSubscriberNumber( ".1.1.79139859489" ); // missing for MT
        op.setCallingPartyNumber( ".1.1.79139343290" );
        op.setIMSI( "250013900405871" );
        op.setLocationInformationMSC( ".1.1.79139860001" );
        op.setSMSCAddress(".1.1.79029869990");

        time_t tm;
        time( &tm );

        op.setTimeAndTimezone( tm );
        op.setTPShortMessageSpecificInfo( 0x11 );
        op.setTPValidityPeriod( 60*5 );
        op.setTPProtocolIdentifier( 0x00 );
        op.setTPDataCodingScheme( 0x08 );

        pipe->send(&op); 
    }

    void sendDeliverySmsResult(unsigned int dlgId)
    {
        DeliverySmsResult   op(delivery);
        op.setDialogId(dlgId);
        pipe->send(&op);
    }

    void onChargeSmsResult(ChargeSmsResult* result)
    {
        assert(result);
        assert(result->getObjectId() == smsc::inman::interaction::CHARGE_SMS_RESULT_TAG);

        if ( result->GetValue() == smsc::inman::interaction::CHARGING_POSSIBLE ) {
            fprintf( stdout, "ChargeSmsResult( CHARGING_POSSIBLE ) received\n");
            sendDeliverySmsResult(result->getDialogId());
        } else
            fprintf( stdout, "ChargeSmsResult( CHARGING_NOT_POSSIBLE ) received\n");
    }

    virtual int  Execute()
    {
        for(;;) {
            fd_set  read;
            FD_ZERO( &read );
            FD_SET( socket->getSocket(), &read );
            int n = select(  socket->getSocket()+1, &read, 0, 0, 0 );
            if ( n > 0 ) {
                SmscCommand* cmd = static_cast<SmscCommand*>(pipe->receive());
                assert( cmd );
                cmd->handle( this );
            }
        }
        return 0;
    }

};

static Facade* g_pFacade = 0;


void cmd_chargeOk(Console&, const std::vector<std::string> &args)
{
    g_pFacade->sendChargeSms(smsc::inman::interaction::DELIVERY_SUCCESSED);
}

void cmd_chargeErr(Console&, const std::vector<std::string> &args)
{
    g_pFacade->sendChargeSms(smsc::inman::interaction::DELIVERY_FAILED);
}

void cmd_delivery(Console&, const std::vector<std::string> &args)
{
    g_pFacade->sendDeliverySmsResult(g_pFacade->getNextDialogId());
}

int main(int argc, char** argv)
{
    if ( argc != 3 ) {
        fprintf( stderr, "Usage: %s <host> <port>\n", argv[0] );
        exit(1);
    }
    const char* host = argv[1];
    int port = atoi( argv[2]);

    Logger::Init();
    fprintf( stdout, "Connecting to IN manager at %s:%d...\n", host, port );
    try {
        Socket* sock = new Socket();

        if (sock->Init( host, port, 1000 )) {
            fprintf( stderr, "Can't init socket: %s (%d)\n", strerror( errno ), errno );
            throw std::runtime_error("Can't init socket");
        }
        if (sock->Connect()) {
            fprintf( stderr, "Can't connect socket: %s (%d)\n", strerror( errno ), errno );
            throw std::runtime_error("Can't connect socket");
        }

        g_pFacade = new Facade( sock );

        Console console;
        console.addItem( "chargeOk", cmd_chargeOk );
        console.addItem( "chargeErr",  cmd_chargeErr );
        console.addItem( "delivery",  cmd_delivery );

        g_pFacade->Start();

        console.run("inman>");

    } catch(const std::exception& error) {
        fprintf(stderr, "Fatal error: %s\n", error.what() );
    }

    delete g_pFacade;
    exit(0);
}