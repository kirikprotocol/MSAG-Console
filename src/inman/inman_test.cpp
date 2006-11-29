static char const ident[] = "$Id$";
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "inman/common/adrutil.hpp"

//#include "logger/Logger.h"
#include "core/synchronization/Event.hpp"
using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

#include "core/threads/Thread.hpp"
using smsc::core::threads::Thread;

#include "inman/common/console.hpp"
using smsc::inman::common::Console;

//#include "util/vformat.hpp"
//using smsc::util::format;

#include "inman/interaction/connect.hpp"
using smsc::inman::interaction::Connect;
using smsc::inman::interaction::ObjectBuffer;
using smsc::inman::interaction::SerializerException;

#include "inman/interaction/MsgBilling.hpp"
using smsc::inman::interaction::INPPacketAC;
using smsc::inman::interaction::INPSerializer;
using smsc::inman::interaction::INPCSBilling;
using smsc::inman::interaction::ChargeSms;
using smsc::inman::interaction::ChargeSmsResult;
using smsc::inman::interaction::DeliverySmsResult;
using smsc::inman::interaction::SMSCBillingHandlerITF;
using smsc::inman::interaction::CsBillingHdr_dlg;
using smsc::inman::interaction::SPckChargeSms;
using smsc::inman::interaction::SPckChargeSmsResult;
using smsc::inman::interaction::SPckDeliverySmsResult;


using smsc::inman::_InmanErrorSource;

/* ************************************************************************** *
 * class AbonentsDB (singleton): abonents registry
 * ************************************************************************** */

typedef enum { abPrepaid = 0, abPostpaid = 1} AbonentType;
typedef struct {
    AbonentType  abType;
    const char *  addr;
    const char *  imsi;
} Abonent;

static const Abonent  _abonents[] = {
    //Nezhinsky phone(prepaid):
     { abPrepaid, ".1.1.79139343290", "250013900405871" }
    //tst phone (prepaid):
    ,{ abPrepaid, ".1.1.79133821781", "250013903368782" }
    //Ryzhkov phone(postpaid):
    ,{ abPostpaid, ".1.1.79139859489", "250013901464251" }
    //Stupnik phone(postpaid):
    ,{ abPostpaid, ".1.1.79139033669", "250013901464251" }
};
#define PRE_ABONENTS_NUM (sizeof(_abonents)/sizeof(Abonent))

struct AbonentInfo {
    AbonentType    abType;
    TonNpiAddress  addr;
    char           imsi[MAP_MAX_IMSI_AddressValueLength];

    AbonentInfo() : abType(abPrepaid)  { imsi[0] = 0; }
};

class AbonentsDB {
protected:
    typedef std::map<unsigned, AbonentInfo> AbonentsMAP;

    Mutex           _sync;
    AbonentsMAP     registry;
    unsigned        lastAbnId;

    void initDB(unsigned n_abn, const Abonent * p_abn)
    {
        for (unsigned i = 0; i < n_abn; i++) {
            AbonentInfo  abn;
            if (p_abn[i].addr)
                abn.addr.fromText(p_abn[i].addr);
            else
                abn.addr.clear();
            if (p_abn[i].imsi[0])
                strcpy(abn.imsi, p_abn[i].imsi);
            else
                abn.imsi[0] = 0;
            abn.abType = p_abn[i].abType;
            registry.insert(AbonentsMAP::value_type(++lastAbnId, abn));
        }
    }

    AbonentsDB(void) : lastAbnId(0) { }
    ~AbonentsDB() { }

public:
    static AbonentsDB * getInstance(void)
    {
        static AbonentsDB     abnData;
        return &abnData;
    }

    static AbonentsDB * Init(unsigned n_abn, const Abonent * p_abn)
    {
        AbonentsDB * adb = AbonentsDB::getInstance();
        adb->initDB(n_abn, p_abn);
        return adb;
    }

    static void printAbnInfo(FILE * stream, const AbonentInfo & abn, unsigned ab_id)
    {
        fprintf(stream, "abn.%u: %s, isdn <%s>, imsi <%s>\n", ab_id,
                abn.abType ? "abPostpaid" : "abPrepaid",
                abn.addr.length ? abn.addr.toString().c_str() : " ",
                abn.imsi[0] ? abn.imsi : "none");
    }

    unsigned getMaxAbId(void) const { return lastAbnId; }
    const AbonentInfo * getAbnInfo(unsigned ab_id)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::const_iterator it = registry.find(ab_id);
        return (it != registry.end()) ? &((*it).second) : NULL;
    }

    unsigned searchNextAbn(AbonentType ab_type, unsigned min_id = 0)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::const_iterator it = registry.begin();
        if (min_id && (min_id <= registry.size())) {
            it = registry.find(min_id);
        }
        for (; it != registry.end(); it ++) {
            const AbonentInfo & abn = (*it).second;
            if (abn.abType == ab_type)
                return (*it).first;
        }
        return 0;
    }

    unsigned searchAbn(const std::string & addr)
    {
        MutexGuard  grd(_sync);
        TonNpiAddress   subscr;
        if (!subscr.fromText(addr.c_str()))
            return 0;

        for (AbonentsMAP::const_iterator it = registry.begin();
                                    it != registry.end(); it ++) {
            const AbonentInfo & abn = (*it).second;

            if (abn.addr.length) {
                if (abn.addr == subscr)
                    return (*it).first;
            }
        }
        return 0; //unknown
    }

    void printAbnInfo(FILE * stream, unsigned ab_id)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::const_iterator it = registry.find(ab_id);
        if (it != registry.end()) {
            const AbonentInfo & abn = (*it).second;
            AbonentsDB::printAbnInfo(stream, abn, ab_id);
        }
    }

    //0, 0 - print ALL
    void printAbonents(FILE * stream, unsigned min_id = 0, unsigned max_id = 0)
    {
        MutexGuard  grd(_sync);
        if (!registry.size())
            return;
        if (!min_id || (min_id > registry.size()))
            min_id = 1;
        if (!max_id || (max_id > registry.size()))
            max_id = registry.size();

        AbonentsMAP::const_iterator it = registry.find(min_id);
        while (min_id <= max_id) {
            const AbonentInfo & abn = (*it).second;
            printAbnInfo(stream, abn, min_id);
            min_id++;
            it++;
        }
    }
/*
    unsigned addAbonent(TonNpiAddress * p_isdn, char * p_imsi)
    {
        MutexGuard  grd(_sync);
        AbonentInfo  abn;
        if (p_isdn && p_isdn->length)
            abn.addr = *p_isdn;
        else
            abn.addr.clear();
        if (p_imsi && p_imsi[0])
            strcpy(abn.imsi, p_imsi);
        else
            abn.imsi[0] = 0;

        registry.insert(AbonentsMAP::value_type(++lastAbnId, abn));
        return lastAbnId;
    }

    bool setAbnInfo(unsigned ab_id, const MAPSCFinfo * p_scf = NULL, const char * p_imsi = NULL)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::iterator it = registry.find(ab_id);
        if (it != registry.end()) {
            AbonentInfo & abn = (*it).second;
            if (p_scf)
                abn.scf = *p_scf;
            else {
                abn.scf.serviceKey = 0;
                abn.scf.scfAddress.clear();
            }
                
            if (p_imsi && p_imsi[0])
                strcpy(abn.imsi, p_imsi);
            return true;
        }
        return false;
    }
*/
};

/* ************************************************************************** *
 * class TNPIAddressDB: TomNpiAddresses registry
 * ************************************************************************** */
//typedef enum { adrNumeric = 0, adrNumericISDN = 1, adrAlphaNum = 5 } AddressTypeInd;

static const char * const _dstAdr[] = {
     ".1.1.79139859489"
    ,".5.0.ussd:448"
    ,".5.0.smsx:201"
};
#define PRE_ADDRESSES_NUM (sizeof(_dstAdr)/sizeof(char *))

static const char *_nm_ToN[] = {
    // 0        1                2    3    4   5
    "unknown", "international", "2", "3", "4", "alphanumeric"
};
class TNPIAddressDB {
protected:
    typedef std::map<unsigned, TonNpiAddress> TNPIAddressMap;

    Mutex           _sync;
    TNPIAddressMap  registry;
    unsigned        lastAdrId;

    unsigned init_db(unsigned n_abn, const char * p_abn[])
    {
        for (unsigned i = 0; i < n_abn; i++) {
            TonNpiAddress  abn;
            if (abn.fromText(p_abn[i]))
                registry.insert(TNPIAddressMap::value_type(++lastAdrId, abn));
        }
        return registry.size();
    }

    TNPIAddressDB() : lastAdrId(0) { }
    ~TNPIAddressDB() { }

public:
    static TNPIAddressDB * getInstance(void)
    {
        static TNPIAddressDB     abnData;
        return &abnData;
    }

    static TNPIAddressDB * Init(unsigned n_abn, const char * p_abn[])
    {
        TNPIAddressDB * adb = TNPIAddressDB::getInstance();
        adb->init_db(n_abn, p_abn);
        return adb;
    }

    static void print_address(FILE * stream, const TonNpiAddress & adr, unsigned ab_id)
    {
        fprintf(stream, "adr.%u: %s (%s)\n", ab_id, adr.toString().c_str(),
                _nm_ToN[adr.typeOfNumber]);
    }


    const TonNpiAddress * get(unsigned adr_id)
    {
        MutexGuard  grd(_sync);
        TNPIAddressMap::const_iterator it = registry.find(adr_id);
        return (it != registry.end()) ? &(*it).second : NULL;
    }

    unsigned add(TonNpiAddress & abn)
    {
        MutexGuard  grd(_sync);
        registry.insert(TNPIAddressMap::value_type(++lastAdrId, abn));
        return lastAdrId;
    }

    unsigned searchNextAdr(unsigned char adr_type, unsigned min_id = 0)
    {
        MutexGuard  grd(_sync);
        TNPIAddressMap::const_iterator it = registry.begin();
        if (min_id && (min_id <= registry.size())) {
            it = registry.find(min_id);
        }
        for (; it != registry.end(); it ++) {
            const TonNpiAddress & abn = (*it).second;
            if (abn.typeOfNumber == adr_type)
                return (*it).first;
        }
        return 0;
    }

    void printAddress(FILE * stream, unsigned ab_id)
    {
        MutexGuard  grd(_sync);
        TNPIAddressMap::const_iterator it = registry.find(ab_id);
        if (it != registry.end()) {
            const TonNpiAddress & abn = (*it).second;
            TNPIAddressDB::print_address(stream, abn, ab_id);
        }
    }

    //0, 0 - print ALL
    void printAddresses(FILE * stream, unsigned min_id = 0, unsigned max_id = 0)
    {
        MutexGuard  grd(_sync);
        if (!registry.size())
            return;
        if (!min_id || (min_id > registry.size()))
            min_id = 1;
        if (!max_id || (max_id > registry.size()))
            max_id = registry.size();

        TNPIAddressMap::const_iterator it = registry.find(min_id);
        while (min_id <= max_id) {
            const TonNpiAddress & abn = (*it).second;
            TNPIAddressDB::print_address(stream, abn, min_id);
            min_id++;
            it++;
        }
    }

};

/* ************************************************************************** *
 * class INDialog: INMan dialog params and result
 * ************************************************************************** */
struct INDialogCfg {
    unsigned            abId;
    unsigned            dstId; //destination address 
    bool                ussdOp;
    uint32_t            xsmsIds; //SMS Extra services id

    INDialogCfg() : abId(1), dstId(1), ussdOp(false), xsmsIds(0) { }
};

class INDialog {
public:
    typedef enum { dIdle = 0, dCharged = 1, dApproved, dReported } INState;

    INDialog(unsigned int dlg_id, const INDialogCfg & use_cfg, bool batch_mode = false,
             uint32_t dlvr_res = 1016)
        : did(dlg_id), cfg(use_cfg), batchMode(batch_mode), dlvrRes(dlvr_res)
        , state(INDialog::dIdle)
    {}

    void    setState(INState new_state) { state = new_state; }
    INState getState(void) const { return state; }
    uint32_t getDlvrResult(void) const { return dlvrRes; }
    bool    isBatchMode(void) const { return batchMode; }

protected:
    unsigned int        did;
    INState             state;
    bool                batchMode;
    INDialogCfg         cfg;
    uint32_t            dlvrRes; //failure by default 
};

/* ************************************************************************** *
 * class Facade: thread serving the INMan connection
 * ************************************************************************** */
#define prompt(str)             fprintf(stdout, str.c_str()); smsc_log_debug(logger, str.c_str())
#define promptLog(log, str)     fprintf(stdout, str.c_str()); smsc_log_debug(log, str.c_str())

#define SELECT_TIMEOUT_STEP 400 //millisecs


class Facade : public Thread, public SMSCBillingHandlerITF {
protected:
    typedef std::map<unsigned int, INDialog*> INDialogsMap;

    Mutex               _mutex;
    Event               lstEvent;
    bool                _running;
    unsigned            dialogId;
    Socket*             socket;
    Connect*            pipe;
    Logger*             logger;
    INDialogsMap        _Dialogs;
    INDialogCfg         _dlgCfg;
    AbonentsDB *        _abDB;
    TNPIAddressDB *     _adrDB;

public:
    Facade(const char* host, int port)
        : logger(Logger::getInstance("smsc.InFacade"))
        , dialogId(0), _running (false)
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
        pipe = new Connect(socket, Connect::frmLengthPrefixed,
                           INPSerializer::getInstance(), logger);
        _abDB = AbonentsDB::Init(PRE_ABONENTS_NUM, _abonents);
        _adrDB = TNPIAddressDB::Init(PRE_ADDRESSES_NUM, _dstAdr);
    }

    virtual ~Facade()
    { 
        _mutex.Lock();
        _running = false; //stop thread 
        _mutex.Unlock();
        lstEvent.Wait(1000L*SELECT_TIMEOUT_STEP*2);

        socket->Close();
        if (socket->getSocket() != INVALID_SOCKET)
            socket->Abort();
        delete pipe; 

        INDialogsMap::const_iterator it;
        for (it = _Dialogs.begin(); it != _Dialogs.end(); it++) {
            INDialog * dlg = (*it).second;
            delete dlg;
        }
    }

    bool isRunning(void) const { return _running; }

    // -- INDialog template params -- //
    const INDialogCfg * getDlgConfig(void) const { return &_dlgCfg; }
    void  printDlgConfig(void) const
    {
        const AbonentInfo * abi = _abDB->getAbnInfo(_dlgCfg.abId);
        const TonNpiAddress * dAdr = _adrDB->get(_dlgCfg.dstId);
        fprintf(stdout, "INDialog config:\n"
                "  Abonent[%u]: %s (%s)\n"
                "  bearerType : dp%s\n"
                "  destAdr[%u]: %s (%s)\n"
                "  SMSExtra: %u\n",
                _dlgCfg.abId, (abi->addr.toString()).c_str(),
                (abi->abType == abPrepaid) ? "prepaid" : "postpaid",
                _dlgCfg.ussdOp ? "USSD" : "SMS",
                _dlgCfg.dstId, dAdr->toString().c_str(), 
                _nm_ToN[dAdr->typeOfNumber], _dlgCfg.xsmsIds);
    }

    void setUssdOp(bool op) { _dlgCfg.ussdOp = op; }
    void setSmsXIds(uint32_t srv_ids) { _dlgCfg.xsmsIds = srv_ids; }
    bool setAddressId(unsigned adr_id)
    { 
        if (!_adrDB->get(adr_id))
            return false;
        _dlgCfg.dstId = adr_id; 
        return true;
    }
    bool setAbonentId(unsigned ab_id)
    { 
        if (!_abDB->getAbnInfo(ab_id))
            return false;
        _dlgCfg.abId = ab_id;
        return true;
    }
    
    // -- INDialog management methods -- //
    unsigned getNextDialogId(void) { return ++dialogId; }

    unsigned int initDialog(unsigned int did = 0, bool batch_mode = false,
                            uint32_t delivery = 1016)
    {
        if (!did)
            did = getNextDialogId();
        if (!_abDB->getAbnInfo(_dlgCfg.abId)) {
            smsc_log_error(logger, "WRN: unknown abId: %u, using #1", _dlgCfg.abId);
            fprintf(stdout, "\nWRN: unknown abId: %u, using #1", _dlgCfg.abId);
            _dlgCfg.abId = 1;
        }
        INDialog * dlg = new INDialog(did, _dlgCfg, batch_mode, delivery);
        _Dialogs.insert(INDialogsMap::value_type(did, dlg));
        return did;
    }

    INDialog * findDialog(unsigned int did)
    {
        INDialogsMap::const_iterator it = _Dialogs.find(did);
        return (it != _Dialogs.end()) ? (*it).second : NULL;
    }

    // -- INMan commands composition and sending methods -- //
    void composeChargeSms(ChargeSms& op)
    {
        static uint32_t _msg_ref = 0x0100;
        static uint64_t _msg_id = 0x010203040000;

        const TonNpiAddress * dAdr = _adrDB->get(_dlgCfg.dstId);
        op.setDestinationSubscriberNumber(dAdr->toString());

        const AbonentInfo * abi = _abDB->getAbnInfo(_dlgCfg.abId);
        op.setCallingPartyNumber(abi->addr.toString());
        op.setCallingIMSI(abi->imsi);
        
        op.setLocationInformationMSC((dAdr->typeOfNumber != ToN_ALPHANUM) ?
                                     ".1.1.79139860001" : "");
        op.setSMSCAddress(".1.1.79029869990");

        op.setSubmitTimeTZ(time(NULL));
        op.setTPShortMessageSpecificInfo(0x11);
        op.setTPValidityPeriod(60*5);
        op.setTPProtocolIdentifier(0x00);
        op.setTPDataCodingScheme(0x08);
        //data for CDR
        op.setCallingSMEid("MAP_PROXY");
        op.setRouteId("sibinco.sms > plmn.kem");
        op.setServiceId(1234);
        op.setUserMsgRef(++_msg_ref);
        op.setMsgId(++_msg_id);
        op.setServiceOp(_dlgCfg.ussdOp ? 0 : -1);
        op.setMsgLength(160);
#ifdef SMSEXTRA
        op.setSmsXSrvs(_dlgCfg.xsmsIds);
#endif /* SMSEXTRA */
    }

    void composeDeliverySmsResult(DeliverySmsResult& op)
    {
        //fill fields for CDR creation
        op.setDestIMSI("250013901464251");
        op.setDestMSC(".1.1.79139860001");
        op.setDestSMEid("DST_MAP_PROXY");
        op.setDivertedAdr(_adrDB->get(_dlgCfg.dstId)->toString());
        op.setDeliveryTime(time(NULL));
    }

    void sendChargeSms(unsigned int dlgId)
    {
        //compose ChargeSms
        SPckChargeSms   pck;
        pck.Hdr().dlgId = dlgId;
        composeChargeSms(pck.Cmd());

        std::string msg = format("Sending ChargeSms [dlgId: %u] ..\n", dlgId);
        prompt(msg);
        pipe->sendPck(&pck);

        INDialog * dlg = findDialog(dlgId);
        if (dlg) {
            if (dlg->getState() == INDialog::dIdle)
                dlg->setState(INDialog::dCharged);
            else {
                msg =  format("WRN: Dialog[%u] state is %u!\n", dlg->getState());
                prompt(msg);
            }
        }
    }

    void sendDeliverySmsResult(unsigned int dlgId, uint32_t deliveryStatus)
    {
        SPckDeliverySmsResult   pck;
        pck.Hdr().dlgId = dlgId;
        pck.Cmd().setResultValue(deliveryStatus);
        composeDeliverySmsResult(pck.Cmd());

        std::string msg = format("Sending DeliverySmsResult: DELIVERY_%s [dlgId: %u]\n",
                                 !deliveryStatus ? "SUCCEEDED" : "FAILED", dlgId);
        prompt(msg);
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
        pipe->sendPck(&pck);
    }

    //Customized variant of Connect::sendObj(): it sends specified
    //number of bytes from ObjectBuffer
//    int  sendPckPart(SerializableObject &obj, uint32_t num_bytes)
    int  sendPckPart(INPPacketAC &pck, uint32_t num_bytes)
    {
        int             offs = 4;
        ObjectBuffer    buffer(1024);

        buffer.setPos(offs);
        pck.serialize(buffer);
        //always Connect::frmLengthPrefixed format
        {
            uint32_t len = htonl(num_bytes);
            memcpy(buffer.get(), (const void *)&len, 4);
            offs = 0;
        }
        return pipe->send(buffer.get() + offs, num_bytes + 4);
    }

    // -- INMan commands listening and handling methods -- //
    void onChargeSmsResult(ChargeSmsResult* result, CsBillingHdr_dlg * hdr)
    {
        std::string msg = format("Dialog[%u] got ChargeSmsResult: CHARGING_%sPOSSIBLE", hdr->dlgId,
                (result->GetValue() == ChargeSmsResult::CHARGING_POSSIBLE ) ?
                 "" : "NOT_");
        if (result->getCombinedError()) {
            uint16_t        errCode;
            InmanErrorType  errType = result->splitError(errCode);
            msg += format(", error %s: %u", _InmanErrorSource[errType], errCode);
        }
        msg += "\n";
        prompt(msg);

        INDialog * dlg = findDialog(hdr->dlgId);
        if (dlg) {
            if (dlg->getState() == INDialog::dCharged) {
                dlg->setState(INDialog::dApproved);
                if (dlg->isBatchMode()) {
                    if (result->GetValue() == ChargeSmsResult::CHARGING_POSSIBLE)
                        sendDeliverySmsResult(hdr->dlgId, dlg->getDlvrResult());
                }
            } else {
                msg =  format("ERR: Dialog[%u] was not Charged!", hdr->dlgId);
                prompt(msg);
            }
        } else {
            msg =  format("ERR: Dialog[%u] is inknown!", hdr->dlgId);
            prompt(msg);
        }
    }
    
    virtual int  Execute() // Thread entry point
    {
        SOCKET   sockId = socket->getSocket();
        struct timeval tmo;
        tmo.tv_sec = 0;
        tmo.tv_usec = 1000L*SELECT_TIMEOUT_STEP;

        _mutex.Lock();
        _running = true;
        while (_running) {
            fd_set  readSet;
            fd_set  errorSet;

            FD_ZERO(&readSet);
            FD_ZERO(&errorSet);

            FD_SET(sockId, &readSet);
            FD_SET(sockId, &errorSet);

            _mutex.Unlock();
            int n = select(sockId + 1, &readSet, 0, &errorSet, &tmo);
            if (n < 0) {
                MutexGuard grd(_mutex);
                std::string msg = format("TCPLst: select() failed, cause: %d (%s)",
                                         errno, strerror(errno));
                smsc_log_fatal(logger, msg.c_str());
                fprintf(stdout, msg.c_str());
                _running = false;
            } else if (n > 0) {
                if (FD_ISSET(sockId, &readSet)) {
                    INPPacketAC* pck = static_cast<INPPacketAC*>(pipe->recvPck());
                    if (!pck) {
                        CustomException * exc = pipe->hasException();
                        if (exc) {
                            smsc_log_error(logger, "TCPLst: %s", exc->what());
                            pipe->resetException();
                        } else { //remote point ends connection
                            MutexGuard grd(_mutex);
                            std::string msg = format("TCPLst: client ends connection", socket);
                            prompt(msg);
                            _running = false;
                            socket->Close();
                        }
                    } else {
                        if (!pck->pHdr() || ((pck->pHdr())->Id() != INPCSBilling::HDR_DIALOG)) {
                            std::string msg;
                            format(msg, "ERR: unknown cmd header: %u", (pck->pHdr())->Id());
                            fprintf(stdout, msg.c_str());
                            smsc_log_error(logger, msg.c_str());
                        } else {
                            if ((pck->pCmd())->Id() == INPCSBilling::CHARGE_SMS_RESULT_TAG) {
                                ChargeSmsResult * cmd = static_cast<ChargeSmsResult*>(pck->pCmd());
                                CsBillingHdr_dlg * hdr = static_cast<CsBillingHdr_dlg*>(pck->pHdr());

                                bool goon = true;
                                try { cmd->loadDataBuf(); }
                                catch (SerializerException& exc) {
                                    std::string msg;
                                    format(msg, "ERR: corrupted cmd %u (dlgId: %u): %s",
                                            cmd->Id(), hdr->Id(), exc.what());
                                    fprintf(stdout, msg.c_str());
                                    smsc_log_error(logger, msg.c_str());
                                    goon = false;
                                }
                                if (goon)
                                    onChargeSmsResult(cmd, hdr);
                            } else {
                                std::string msg = format("TCPLst: unknown command recieved: %u\n",
                                                        (pck->pCmd())->Id());
                                prompt(msg);
                            }
                        }
                    }
                }
                if (FD_ISSET(sockId, &errorSet)) {
                    MutexGuard grd(_mutex);
                    std::string msg = format("ERR: Error Event on socket[%u].", sockId);
                    smsc_log_error(logger, msg.c_str());
                    fprintf(stdout, msg.c_str());
                    _running = false;
                    socket->Abort();
                }
            }
            _mutex.Lock();
        } /* eow */
        _mutex.Unlock();
        lstEvent.Signal();
        return 0;
    }

};

class ConnectionClosedException : public std::exception {
public:
    const char* what() const throw()
    {
        return "IN manager closed connection.";
    }
};

static Facade* _pFacade = 0;

/* ************************************************************************** *
 * Console commands: sending INMan commands
 * ************************************************************************** */

static void utl_multi_charge(const std::vector<std::string> &args, uint32_t delivery = 0)
{
    if (_pFacade->isRunning()) {
        if (args.size() < 2) {
            fprintf(stdout, "USAGE: %s num_of_dialogs!\n", args[0].c_str());
            return;
        }
        unsigned int dnum = (unsigned int)atoi(args[1].c_str());
        if (!dnum) {
            fprintf(stdout, "ERR: bad number specified (%s)!\n", args[1].c_str());
            return;
        }
        for (; dnum > 0; dnum--) {
            unsigned int did = _pFacade->initDialog(0, true, delivery);
            _pFacade->sendChargeSms(did);
        }
    } else
        throw ConnectionClosedException();
}

void cmd_multi_chargeOk(Console&, const std::vector<std::string> &args)
{
    utl_multi_charge(args, 0);
}

void cmd_multi_chargeErr(Console&, const std::vector<std::string> &args)
{
    utl_multi_charge(args, 1016);
}

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
            fprintf(stdout, "ERR: dialog id %s is bad or missed!\n",
                    (args.size() > 1) ? args[1].c_str() : "");
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
            fprintf(stdout, "ERR: dialog id %s is bad or missed!\n",
                    (args.size() > 1) ? args[1].c_str() : "");
            return;
        }

        if (!_pFacade->findDialog(did))
            fprintf(stdout, "WRN: unknown dialog id specified (%s)!\n", args[1].c_str());

        _pFacade->sendDeliverySmsResult(did, 1016);
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

        SPckChargeSms   pck;
        pck.Hdr().dlgId = dlgId;
        _pFacade->composeChargeSms(pck.Cmd());
        _pFacade->sendPckPart(pck, size);
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

        SPckDeliverySmsResult   pck;
        pck.Hdr().dlgId = dlgId;
        pck.Cmd().setResultValue(1016);
        _pFacade->composeDeliverySmsResult(pck.Cmd());
        _pFacade->sendPckPart(pck, size);
        fprintf(stdout, "Sent %u bytes of DeliverySmsResult (dlgId: %u)\n", size, dlgId);
    } else
        throw ConnectionClosedException();
}

/* ************************************************************************** *
 * Console commands: setting INMan dialog params
 * ************************************************************************** */

void cmd_config(Console&, const std::vector<std::string> &args)
{
    _pFacade->printDlgConfig();
}

void cmd_dpsms(Console&, const std::vector<std::string> &args)
{
    _pFacade->setUssdOp(false);
    _pFacade->printDlgConfig();
}

void cmd_dpussd(Console&, const std::vector<std::string> &args)
{
    _pFacade->setUssdOp(true);
    _pFacade->printDlgConfig();
}

//USAGE: use_adr [?|help | adr_NN]
static const char hlp_use_adr[] = "USAGE: %s [?|help | adr_NN]\n";
void cmd_use_adr(Console&, const std::vector<std::string> &args)
{
    unsigned abId = 0;

    if (args.size() > 1) {
        if (!strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
            fprintf(stdout, hlp_use_adr, args[0].c_str());
            return;
        }
        abId = (uint32_t)atoi(args[1].c_str());
    }
    if (!abId) {
        fprintf(stdout, hlp_use_adr, args[0].c_str());
        fprintf(stdout, "Known destination addresses:\n");
        TNPIAddressDB::getInstance()->printAddresses(stdout);
        return;
    }
    if (!_pFacade->setAddressId(abId))
        fprintf(stdout, "ERR: Unknown address #%u!\n", abId);
    else
        _pFacade->printDlgConfig();
}
static void utl_next_adr(const std::vector<std::string> &args, unsigned char use_ton)
{
    unsigned abId = 0;

    if (args.size() > 1) {
        if (!strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
            fprintf(stdout, hlp_use_adr, args[0].c_str());
            return;
        }
        abId = (uint32_t)atoi(args[1].c_str());
    }
    abId = TNPIAddressDB::getInstance()->searchNextAdr(use_ton, abId);
    if (!abId)
        fprintf(stdout, "ERR: no address found!");
    else {
        _pFacade->setAddressId(abId);
        _pFacade->printDlgConfig();
    }
    return;
}

void cmd_adrNum(Console&, const std::vector<std::string> &args)
{
    utl_next_adr(args, ToN_INTERNATIONAL);
}

void cmd_adrAlpha(Console&, const std::vector<std::string> &args)
{
    utl_next_adr(args, ToN_ALPHANUM);
}


//USAGE: use_abn [?|help | abn_NN]
static const char hlp_use_abn[] = "USAGE: %s [?|help | abn_NN]\n";
void cmd_use_abn(Console&, const std::vector<std::string> &args)
{
    unsigned abId = 0;

    if (args.size() > 1) {
        if (!strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
            fprintf(stdout, hlp_use_abn, args[0].c_str());
            return;
        }
        abId = (uint32_t)atoi(args[1].c_str());
    }
    if (!abId) {
        fprintf(stdout, hlp_use_abn, args[0].c_str());
        fprintf(stdout, "Known abonents:\n");
        AbonentsDB::getInstance()->printAbonents(stdout);
        return;
    }
    if (!_pFacade->setAbonentId(abId))
        fprintf(stdout, "ERR: Unknown abonent #%u!\n", abId);
    else
        _pFacade->printDlgConfig();
}

static void utl_next_abn(const std::vector<std::string> &args, AbonentType ab_type)
{
    unsigned abId = 0;

    if (args.size() > 1) {
        if (!strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
            fprintf(stdout, hlp_use_abn, args[0].c_str());
            return;
        }
        abId = (uint32_t)atoi(args[1].c_str());
    }
    abId = AbonentsDB::getInstance()->searchNextAbn(ab_type, abId);
    if (!abId)
        fprintf(stdout, "ERR: no known abonent found!");
    else {
        _pFacade->setAbonentId(abId);
        _pFacade->printDlgConfig();
    }
    return;
}
//USAGE: prepaid [?|help | abn_NN]
void cmd_prepaid(Console&, const std::vector<std::string> &args)
{
    utl_next_abn(args, abPrepaid);
}
//USAGE: postpaid [?|help | abn_NN]
void cmd_postpaid(Console&, const std::vector<std::string> &args)
{
    utl_next_abn(args, abPostpaid);
}

//USAGE: use_abn [?|help | abn_NN]
static const char hlp_use_xsms[] = "USAGE: %s [?|help | Number[baseSym]]\n"
                                   "  baseSym: empty - Decimal, hH - Hex, Bb - Binary\n";
void cmd_use_xsms(Console&, const std::vector<std::string> &args)
{
    uint32_t xId = 0;

    if ((args.size() < 2)
        || !strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
        fprintf(stdout, hlp_use_xsms, args[0].c_str());
        return;
    }
    int base;
    char sym = (args[1])[args[1].length() - 1];
    switch (sym) {
    case 'h': case 'H':
        base = 16; break;
    case 'b': case 'B':
        base = 2; break;
    default:
        base = 0; //autodetect
    }
    unsigned long val = strtoul(args[1].c_str(), NULL, base);
    if (!val || (val > 0xFFFFFFFF)) {
        fprintf(stdout, "ERR: invalid number!");
        return;
    }
    _pFacade->setSmsXIds((uint32_t)val);
    _pFacade->printDlgConfig();
}

//USAGE: use_abn [?|help | abn_NN]
void cmd_no_xsms(Console&, const std::vector<std::string> &args)
{
    _pFacade->setSmsXIds(0);
    _pFacade->printDlgConfig();
}



int main(int argc, char** argv)
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <host> <port>\n", argv[0]);
        exit(1);
    }
    int port = atoi(argv[2]);
    if (!port) {
        fprintf(stderr, "ERR: bad port specified (%s) !", argv[2]);
        exit(1);
    }
    const char* host = argv[1];

    Logger::Init();
    try {
        _pFacade = new Facade(host, port);

        Console console;
/* ************************************************************************** *
 * Console commands: sending commands to INMan
 * ************************************************************************** */
        console.addItem("charge", cmd_charge); //chargeSMS only
        console.addItem("chargeOk", cmd_chargeOk); //chargeSMS -> reportOk
        console.addItem("chargeErr",  cmd_chargeErr);//chargeSMS -> reportErr
        console.addItem("reportOk",  cmd_reportOk);
        console.addItem("reportErr",  cmd_reportErr);
        console.addItem("chargeExc",  cmd_chargeExc);
        console.addItem("dlvrExc",  cmd_dlvrExc); 
        console.addItem("m_chargeOk", cmd_multi_chargeOk); //[chargeSMS -> reportOk]
        console.addItem("m_chargeErr",  cmd_multi_chargeErr);//[chargeSMS -> reportErr]
/* ************************************************************************** *
 * Console commands: setting INMan dialog params
 * ************************************************************************** */
        console.addItem("config",  cmd_config);
        console.addItem("dpsms",  cmd_dpsms);
        console.addItem("dpussd",  cmd_dpussd);
        /**/
        console.addItem("use_abn",  cmd_use_abn);
        console.addItem("prepaid",  cmd_prepaid);
        console.addItem("postpaid",  cmd_postpaid);
        /**/
        console.addItem("use_adr",  cmd_use_adr);
        console.addItem("adrnum",  cmd_adrNum);
        console.addItem("adralpha",  cmd_adrAlpha);
        /**/
        console.addItem("use_xsms",  cmd_use_xsms);
        console.addItem("no_xsms",  cmd_use_xsms);
        _pFacade->Start();
        console.run("inman>");

    } catch(const std::exception& error) {
        fprintf(stderr, error.what() );
    }

    delete _pFacade;
    exit(0);
}