static char const ident[] = "$Id$";
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "inman/common/console.hpp"
#include "inman/common/util.hpp"
#include "inman/interaction/messages.hpp"


using smsc::core::threads::Thread;
using smsc::inman::common::Console;
using smsc::inman::common::format;
using smsc::inman::interaction;
using smsc::inman::interaction::ObjectPipe;
using smsc::inman::interaction::SerializerInap;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::SmscHandler;
using smsc::inman::interaction::SmscCommand;
using smsc::inman::interaction::DeliverySmsResult_t;


#define prompt(str)     fprintf(stdout, str.c_str()); smsc_log_debug(logger, str.c_str())

class Facade : public Thread, public SmscHandler
{
protected:
    bool                _running;
    unsigned            dialogId;
    Socket*             socket;
    ObjectPipe*         pipe;
    Logger*             logger;
    DeliverySmsResult_t delivery;

public:
    Facade(const char* host, int port)
        : logger(Logger::getInstance("smsc.InFacade"))
        , dialogId(0)
        , _running (false)

    {
        std::string msg;
        msg = format("InFacade: connecting to InManager at %s:%d...\n", host, port);
        smsc_log_info(logger, msg.c_str());
        fprintf(stdout, msg.c_str());

        socket = new Socket();
        if (socket->Init(host, port, 1000)) {
            msg = format("InFacade: can't init socket: %s (%d)\n", strerror(errno), errno);
            smsc_log_error(logger, msg.c_str());
            throw std::runtime_error(msg.c_str());
        }
        if (socket->Connect()) {
            msg = format("InFacade: can't connect socket: %s (%d)\n", strerror(errno), errno);
            smsc_log_error(logger, msg.c_str());
            throw std::runtime_error(msg.c_str());
        }
        pipe = new ObjectPipe(socket, SerializerInap::getInstance(),
                                        ObjectPipe::frmLengthPrefixed);
        pipe->setLogger(logger);
    }

    virtual ~Facade()
    { 
        _running = false; //stop thread 
        delete pipe;
        delete socket; 
    }

    bool isRunning(void) const { return _running; }
    unsigned getNextDialogId(void) { return ++dialogId; }

    void sendChargeSms(DeliverySmsResult_t deliveryStatus)
    {
        //set delivery status to send on ChargeSmsResult
        delivery = deliveryStatus;
        //compose ChargeSms
        ChargeSms op;

        op.setDialogId(getNextDialogId());

        op.setDestinationSubscriberNumber( ".1.1.79139859489" ); // missing for MT
//Nezhinsky phone:        
        op.setCallingPartyNumber( ".1.1.79139343290" );
        op.setCallingIMSI( "250013900405871" );
//Ryzhkov phone:
//        op.setCallingPartyNumber( ".1.1.79139859489" );
//        op.setIMSI( "250013901464251" );

        op.setLocationInformationMSC( ".1.1.79139860001" );
        op.setSMSCAddress(".1.1.79029869990");

//        time_t tm;
//        time( &tm );
        op.setSubmitTimeTZ(time(NULL));
        op.setTPShortMessageSpecificInfo( 0x11 );
        op.setTPValidityPeriod( 60*5 );
        op.setTPProtocolIdentifier( 0x00 );
        op.setTPDataCodingScheme( 0x08 );
        //data for CDR
        op.setCallingSMEid("MAP_PROXY");
        op.setRouteId("sibinco.sms > plmn.kem");
        op.setServiceId(1234);
        op.setUserMsgRef(0x01fa);
        op.setMsgId(0x010203040506);
        op.setServiceOp(0);

        pipe->send(&op); 
    }

    void sendDeliverySmsResult(unsigned int dlgId)
    {
        DeliverySmsResult   op(delivery);
        op.setDialogId(dlgId);
        std::string msg = format("Sending DeliverySmsResult: DELIVERY_%s\n",
                                !delivery ? "DELIVERY_SUCCESSED" : "DELIVERY_FAILED");
        prompt(msg);
        if (!delivery) { //fill optional fields
            op.setDestIMSI("250013901464251");
            op.setDestMSC(".1.1.79139860001");
            op.setDestSMEid("DST_MAP_PROXY");
            op.setDeliveryTime(time(NULL));
        }
        pipe->send(&op);
    }

    void onChargeSmsResult(ChargeSmsResult* result)
    {
        assert(result);
        assert(result->getObjectId() == smsc::inman::interaction::CHARGE_SMS_RESULT_TAG);
        std::string msg = format("ChargeSmsResult: CHARGING_%sPOSSIBLE",
                ( result->GetValue() == smsc::inman::interaction::CHARGING_POSSIBLE ) ?
                 "" : "NOT_");
        if (result->GetRPCause())
            msg += format(", RPCause: %u", (unsigned)result->GetRPCause());
        else if (result->GetCAP3Error())
            msg += format(", CAP3Error: %u", result->GetCAP3Error());
        else if (result->GetErrorCode())
            msg += ", TCP dialog error";
        msg += "\n";

        prompt(msg);
        if (result->GetErrorClass() <= ChargeSmsResult::chgRPCause)
            sendDeliverySmsResult(result->getDialogId());
    }

    // Thread entry point
    virtual int  Execute()
    {
        _running = true;
        while(_running) {
            fd_set  read;
            FD_ZERO( &read );
            FD_SET( socket->getSocket(), &read );
            int n = select(  socket->getSocket()+1, &read, 0, 0, 0 );
            if ( n > 0 ) {
                SmscCommand* cmd = static_cast<SmscCommand*>(pipe->receive());
                if (cmd)
                    cmd->handle( this );
                else { //socket closed or socket error
                    _running = false;        
                }
            }
        }
        return 0;
    }

};

class ConnectionClosedException : public std::exception
{
public:
    const char* what() const throw()
    {
        return "IN manager closed connection.";
    }
};

static Facade* _pFacade = 0;



void cmd_chargeOk(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning())
        _pFacade->sendChargeSms(smsc::inman::interaction::DELIVERY_SUCCESSED);
    else
        throw ConnectionClosedException();
}

void cmd_chargeErr(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning())
        _pFacade->sendChargeSms(smsc::inman::interaction::DELIVERY_FAILED);
    else
        throw ConnectionClosedException();
}

void cmd_delivery(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning())
        _pFacade->sendDeliverySmsResult(_pFacade->getNextDialogId());
    else
        throw ConnectionClosedException();
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
    try {
        _pFacade = new Facade(host, port);

        Console console;
        console.addItem( "chargeOk", cmd_chargeOk );
        console.addItem( "chargeErr",  cmd_chargeErr );
        console.addItem( "delivery",  cmd_delivery );

        _pFacade->Start();

        console.run("inman>");

    } catch(const std::exception& error) {
        fprintf(stderr, error.what() );
    }

    delete _pFacade;
    exit(0);
}