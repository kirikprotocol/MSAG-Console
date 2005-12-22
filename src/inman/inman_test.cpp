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
#include "inman/interaction/connect.hpp"


using smsc::core::threads::Thread;
using smsc::inman::common::Console;
using smsc::inman::common::format;

using smsc::inman::interaction::Connect;
using smsc::inman::interaction::SerializerInap;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::SmscHandler;
using smsc::inman::interaction::SmscCommand;


#define prompt(str)     fprintf(stdout, str.c_str()); smsc_log_debug(logger, str.c_str())
#define promptLog(log, str)     fprintf(stdout, str.c_str()); smsc_log_debug(log, str.c_str())

typedef enum { abPrepaid = 0, abPostpaid = 1} AbonentType;
typedef struct {
    AbonentType  abType;
    const char *  addr;
    const char *  imsi;
} Abonent;

typedef enum { adrNumeric = 0, adrAlphaNum = 1 } AddressTypeInd;

static const char * const _dstAdr[] = {
     ".1.1.79139859489"
    ,".5.0.ussd:448"
};

static const Abonent  _abonents[2] = {
    //Nezhinsky phone(prepaid):
     { abPrepaid, ".1.1.79139343290", "250013900405871" }
    //Ryzhkov phone(postpaid:
    ,{ abPostpaid, ".1.1.79139859489", "250013901464251" }
};

class INDialog {
public:
    typedef enum { dIdle = 0, dCharged = 1, dApproved, dReported } INState;

    INDialog(unsigned int id, AbonentType ab_type, AddressTypeInd adr_type,
             bool ussd_op, bool batch_mode = false, uint32_t delivery = 0)
        : did(id), state(INDialog::dIdle), abType(ab_type), ussdOp(ussd_op)
        , batchMode(batch_mode), dlvrRes(delivery), adrType(adr_type)
    {}

    void setState(INState new_state) { state = new_state; }
    INState getState(void) { return state; }
    uint32_t getDlvrResult(void) { return dlvrRes; }

    bool    isBatchMode(void) { return batchMode; }

protected:
    unsigned int        did;
    INState             state;
    bool                batchMode;
    uint32_t dlvrRes;
    AbonentType         abType;
    AddressTypeInd      adrType; //destination address type
    bool                ussdOp;
};


class Facade : public Thread, public SmscHandler
{
protected:
    typedef std::map<unsigned int, INDialog*> INDialogsMap;

    bool                _running;
    unsigned            dialogId;
    Socket*             socket;
    Connect*            pipe;
    Logger*             logger;
    AbonentType         abType;
    bool                _ussdOp;
    INDialogsMap        _Dialogs;
    AddressTypeInd      _adrType; //dafault destination address type

public:
    Facade(const char* host, int port)
        : logger(Logger::getInstance("smsc.InFacade"))
        , dialogId(0), abType(abPrepaid), _running (false)
        , _ussdOp(false), _adrType(adrNumeric)
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
        pipe = new Connect(socket, SerializerInap::getInstance(),
                                        Connect::frmLengthPrefixed, logger);
    }

    virtual ~Facade()
    { 
        _running = false; //stop thread 
        delete pipe;
        delete socket; 

        INDialogsMap::const_iterator it;
        for (it = _Dialogs.begin(); it != _Dialogs.end(); it++) {
            INDialog * dlg = (*it).second;
            delete dlg;
        }
    }

    SerializerITF * getSerializer(void) const { return SerializerInap::getInstance(); }
    Connect *       getConnect(void) const { return pipe; }

    void setAddressType(AddressTypeInd adr_type) { _adrType = adr_type; }
    AddressTypeInd getAddressType(void) { return _adrType; }

    bool isUssdOp(void) { return _ussdOp; }
    void setUssdOp(bool op) { _ussdOp = op; }

    AbonentType getAbonent(void) const { return abType; }
    void setAbonent(AbonentType ab) { abType = ab; }

    bool isRunning(void) const { return _running; }
    unsigned getNextDialogId(void) { return ++dialogId; }

    unsigned int initDialog(unsigned int did = 0, bool batch_mode = false,
                            uint32_t delivery = 0)
    {
        if (!did)
            did = getNextDialogId();
        INDialog * dlg = new INDialog(did, abType, _adrType, _ussdOp, batch_mode, delivery);
        _Dialogs.insert(INDialogsMap::value_type(did, dlg));
        return did;
    }

    INDialog * findDialog(unsigned int did)
    {
        INDialogsMap::const_iterator it = _Dialogs.find(did);

        if (it != _Dialogs.end())
            return (*it).second;
        return NULL;
    }


    void composeChargeSms(ChargeSms& op, unsigned int dlgId, bool ussd_op)
    {
        op.setDialogId(dlgId);
        op.setDestinationSubscriberNumber( _dstAdr[_adrType]);
        op.setCallingPartyNumber(_abonents[abType].addr);
        op.setCallingIMSI(_abonents[abType].imsi);

        op.setLocationInformationMSC( ".1.1.79139860001" );
        op.setSMSCAddress(".1.1.79029869990");

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
        op.setServiceOp(ussd_op ? 0 : -1);
        op.setMsgLength(160);
    }

    void sendChargeSms(unsigned int dlgId)
    {
        //compose ChargeSms
        ChargeSms op;
        composeChargeSms(op, dlgId, _ussdOp);

        std::string msg = format("Sending ChargeSms [dlgId: %u] ..\n", op.getDialogId());
        prompt(msg);
        pipe->sendObj(&op);

        INDialog * dlg = findDialog(dlgId);
        assert(dlg);
        if (dlg->getState() == INDialog::dIdle)
            dlg->setState(INDialog::dCharged);
        else {
            msg =  format("WRN: Dialog[%u] state is %u!\n", dlg->getState());
            prompt(msg);
        }
    }

    void sendDeliverySmsResult(unsigned int dlgId, uint32_t deliveryStatus)
    {
        DeliverySmsResult   op(deliveryStatus);
        op.setDialogId(dlgId);
        std::string msg = format("Sending DeliverySmsResult: DELIVERY_%s [dlgId: %u]\n",
                                 !deliveryStatus ? "SUCCEEDED" : "FAILED", dlgId);
        prompt(msg);
        //fill fields for CDR creation
        op.setDestIMSI("250013901464251");
        op.setDestMSC(".1.1.79139860001");
        op.setDestSMEid("DST_MAP_PROXY");
        op.setDivertedAdr(_dstAdr[_adrType]);
        op.setDeliveryTime(time(NULL));
        
        
        INDialog * dlg = findDialog(dlgId);
        if (dlg) {
            if (dlg->getState() == INDialog::dApproved)
                dlg->setState(INDialog::dReported);
            else {
                msg =  format("WRN: Dialog[%u] state is %u!\n", dlg->getState());
                prompt(msg);
            }
        } else {
            msg =  format("WRN: Dialog[%u] is inknown!\n", dlgId);
            prompt(msg);
        }
        pipe->sendObj(&op);
    }

    void onChargeSmsResult(ChargeSmsResult* result)
    {
        unsigned int did = result->getDialogId();
        std::string msg = format("Dialog[%u] got ChargeSmsResult: CHARGING_%sPOSSIBLE", did,
                (result->GetValue() == smsc::inman::interaction::CHARGING_POSSIBLE ) ?
                 "" : "NOT_");
        if (result->GetRPCause())
            msg += format(", RPCause: %u", (unsigned)result->GetRPCause());
        else if (result->GetCAP3Error())
            msg += format(", CAP3Error: %u", result->GetCAP3Error());
        else if (result->GetINprotocolError())
            msg += ", TCP dialog error";
        else if (result->GetTCAPError())
            msg += ", TCAP dialog error";
        msg += "\n";
        prompt(msg);

        INDialog * dlg = findDialog(did);
        if (dlg) {
            if (dlg->getState() == INDialog::dCharged) {
                dlg->setState(INDialog::dApproved);
                if (dlg->isBatchMode()) {
                    if (result->GetValue() == smsc::inman::interaction::CHARGING_POSSIBLE)
                        sendDeliverySmsResult(did, dlg->getDlvrResult());
                }
            } else {
                msg =  format("ERR: Dialog[%u] was not Charged!", did);
                prompt(msg);
            }
        } else {
            msg =  format("ERR: Dialog[%u] is inknown!", did);
            prompt(msg);
        }
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
                SmscCommand* cmd = static_cast<SmscCommand*>(pipe->receiveObj());
                if (cmd) {
                    if (cmd->getObjectId() == smsc::inman::interaction::CHARGE_SMS_RESULT_TAG) {
                        bool goon = true;
                        try { cmd->loadDataBuf(); }
                        catch (SerializerException& exc) {
                            std::string msg = format("ERR: corrupted cmd %u (dlgId: %u): %s",
                                                    cmd->getObjectId(), cmd->getDialogId(),
                                                    exc.what());
                            fprintf(stdout, msg.c_str());
                            smsc_log_error(logger, msg.c_str());
                            goon = false;
                        }
                        if (goon)
                            cmd->handle(this);
                    } else
                        fprintf(stdout, "ERR: unknown command recieved: %u\n",
                                cmd->getObjectId());
                } else { //socket closed or socket error
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

void cmd_charge(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        unsigned int did = 0;

        if (args.size() > 1) {
            did = (unsigned int)atoi(args[1].c_str());
            if (!did) {
                fprintf(stdout, "ERR: bad dialog id specified (%s)!\n", args[1].c_str());
                return;
            }
        }
        if (!_pFacade->findDialog(did))
            did = _pFacade->initDialog(did);
        _pFacade->sendChargeSms(did);
    } else
        throw ConnectionClosedException();
}


void cmd_chargeOk(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        unsigned int did = _pFacade->initDialog(0, true, 0);
        _pFacade->sendChargeSms(did);
    } else
        throw ConnectionClosedException();
}

void cmd_chargeErr(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        unsigned int did = _pFacade->initDialog(0, true, 1016);
        _pFacade->sendChargeSms(did);
    } else
        throw ConnectionClosedException();
}


void cmd_reportOk(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        unsigned int did = 0;

        if (args.size() > 1) //create new dialog
            did = (unsigned int)atoi(args[1].c_str());

        if (!did) {
            fprintf(stdout, "ERR: bad dialog id specified (%s)!\n", args[1].c_str());
            return;
        }

        if (!_pFacade->findDialog(did))
            fprintf(stdout, "WRN: unknown dialog id specified (%s)!\n", args[1].c_str());

        _pFacade->sendDeliverySmsResult(did, 0);
    } else
        throw ConnectionClosedException();
}

void cmd_reportErr(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        unsigned int did = 0;

        if (args.size() > 1) //create new dialog
            did = (unsigned int)atoi(args[1].c_str());

        if (!did) {
            fprintf(stdout, "ERR: bad dialog id specified (%s)!\n", args[1].c_str());
            return;
        }

        if (!_pFacade->findDialog(did))
            fprintf(stdout, "WRN: unknown dialog id specified (%s)!\n", args[1].c_str());

        _pFacade->sendDeliverySmsResult(did, 1016);
    } else
        throw ConnectionClosedException();
}


void cmd_config(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        AbonentType ab = _pFacade->getAbonent();
        fprintf(stdout,
                "Current Abonent: %s (%s)\n"
                "  bearerType to use: dp%s\n"
                "  dest.adr type: %sumeric\n"
                ,_abonents[ab].addr, (ab == abPrepaid) ? "prepaid":"postpaid",
                _pFacade->isUssdOp() ? "USSD" : "SMS",
                _pFacade->getAddressType() ? "alphaN" : "n");
    } else
        throw ConnectionClosedException();
}

void cmd_prepaid(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        _pFacade->setAbonent(abPrepaid);
        std::string msg = format("Current Abonent: %s (prepaid)\n", _abonents[abPrepaid].addr);
        fprintf(stdout, msg.c_str());
    } else
        throw ConnectionClosedException();
}
void cmd_postpaid(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        _pFacade->setAbonent(abPostpaid);
        std::string msg = format("Current Abonent: %s (postpaid)\n", _abonents[abPostpaid].addr);
        fprintf(stdout, msg.c_str());
    } else
        throw ConnectionClosedException();
}


void cmd_dpsms(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        _pFacade->setUssdOp(false);
        fprintf(stdout, "Current dpBearerType: dpSMS\n");
    } else
        throw ConnectionClosedException();
}

void cmd_dpussd(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        _pFacade->setUssdOp(true);
        fprintf(stdout, "Current dpBearerType: dpUSSD\n");
    } else
        throw ConnectionClosedException();
}

void cmd_adrNum(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        _pFacade->setAddressType(adrNumeric);
        fprintf(stdout, "Current dest.adr type: numeric\n");
    } else
        throw ConnectionClosedException();
}

void cmd_adrAlpha(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        _pFacade->setAddressType(adrAlphaNum);
        fprintf(stdout, "Current dest.adr type: alphaNumeric\n");
    } else
        throw ConnectionClosedException();
}

//USAGE: chargeExc [num_bytes] [dlgId]
//sends specified number of bytes of ChargeSMS packet, causing exception on remote point
void cmd_chargeExc(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        uint32_t size = 8; //send prefix only by default
        unsigned dlgId;

        if (args.size() > 1)
            size = (uint32_t)atoi(args[1].c_str());
        if (args.size() > 2)
            dlgId = (unsigned)atoi(args[2].c_str());
        else
            dlgId = _pFacade->getNextDialogId();

        ChargeSms op;
        _pFacade->composeChargeSms(op, dlgId, true);

        ObjectBuffer    buffer(1024);
        SerializerITF * serl = _pFacade->getSerializer();
        serl->serialize(&op, buffer);
        Connect* conn = _pFacade->getConnect();

        uint32_t len = htonl(size);
        conn->send((const unsigned char*)&len, 4);
        conn->send(buffer.get(), size);
        fprintf(stdout, "Sent %u bytes of ChargeSMS (dlgId: %u)\n", size, dlgId);
    } else
        throw ConnectionClosedException();
}

//USAGE: dlvrExc [num_bytes] [dlgId]
//sends specified number of bytes of DeliverySmsResult packet, causing exception on remote point
void cmd_dlvrExc(Console&, const std::vector<std::string> &args)
{
    if (_pFacade->isRunning()) {
        uint32_t size = 8;
        unsigned dlgId;

        if (args.size() > 1)
            size = (uint32_t)atoi(args[1].c_str());
        if (args.size() > 2)
            dlgId = (unsigned)atoi(args[2].c_str());
        else
            dlgId = _pFacade->getNextDialogId();

        DeliverySmsResult   op(1016);
        op.setDialogId(dlgId);

        ObjectBuffer    buffer(1024);
        SerializerITF * serl = _pFacade->getSerializer();
        serl->serialize(&op, buffer);
        Connect* conn = _pFacade->getConnect();

        uint32_t len = htonl(size);
        conn->send((const unsigned char*)&len, 4);
        conn->send(buffer.get(), size);
        fprintf(stdout, "Sent %u bytes of DeliverySmsResult (dlgId: %u)\n", size, dlgId);
    } else
        throw ConnectionClosedException();
}

int main(int argc, char** argv)
{
    if ( argc != 3 ) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0] );
        exit(1);
    }
    int port = atoi( argv[2]);
    if (!port) {
        fprintf(stderr, "ERR: bad port specified (%s) !", argv[2]);
        exit(1);
    }
    const char* host = argv[1];

    Logger::Init();
    try {
        _pFacade = new Facade(host, port);

        Console console;
        console.addItem( "charge", cmd_charge ); //chargeSMS only
        console.addItem( "chargeOk", cmd_chargeOk ); //chargeSMS -> reportOk
        console.addItem( "chargeErr",  cmd_chargeErr );//chargeSMS -> reportErr
        console.addItem( "reportOk",  cmd_reportOk );
        console.addItem( "reportErr",  cmd_reportErr );
        console.addItem( "prepaid",  cmd_prepaid );
        console.addItem( "postpaid",  cmd_postpaid );
        console.addItem( "dpsms",  cmd_dpsms );
        console.addItem( "dpussd",  cmd_dpussd );
        console.addItem( "config",  cmd_config);
        console.addItem( "adrnum",  cmd_adrNum);
        console.addItem( "adralpha",  cmd_adrAlpha);
        console.addItem( "chargeExc",  cmd_chargeExc);
        console.addItem( "dlvrExc",  cmd_dlvrExc); 

        _pFacade->Start();

        console.run("inman>");

    } catch(const std::exception& error) {
        fprintf(stderr, error.what() );
    }

    delete _pFacade;
    exit(0);
}