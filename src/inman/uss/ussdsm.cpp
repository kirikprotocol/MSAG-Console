static char const ident[] = "$Id$";
#include <assert.h>

#include "inman/common/cvtutil.hpp"
#include "inman/common/adrutil.hpp"
#include "inman/common/util.hpp"
#include "inman/uss/ussdsm.hpp"
#include "inman/uss/vlr.hpp"
#include "inman/interaction/ussmessages.hpp"
#include "inman/comp/acdefs.hpp"

using smsc::inman::interaction::USSRequestMessage;
using smsc::inman::interaction::USSResultMessage;
using smsc::inman::interaction::USS2CMD;
using smsc::inman::interaction::USSDATA_T;

using smsc::inman::common::dump;
using smsc::cvtutil::packTextAs7BitPadded;

using smsc::inman::usscomp::ProcessUSSRequestRes;
using smsc::inman::usscomp::ProcessUSSRequestArg;
using smsc::inman::inap::Session;
using smsc::inman::inap::Dialog;
using smsc::inman::inap::DialogListener;
using smsc::inman::inap::Invoke;

using smsc::inman::usscomp::MAPUSS_OpCode;

namespace smsc {
namespace inman {
namespace uss {

/* ************************************************************************** *
 * class USSTcapListener implementation:
 * ************************************************************************** */
USSTcapListener::USSTcapListener(Invoke * op, USSDSM * dsm)
    : orgInv(op), _dSM(dsm)
{ 
    assert(op);
    assert(dsm);
}

void USSTcapListener::result(TcapEntity* resL)
{
    assert(resL->getParam());
    ProcessUSSRequestRes* res = static_cast<ProcessUSSRequestRes*>(resL->getParam());
    _dSM->onUSSRequestResult(res);
}

void USSTcapListener::error(TcapEntity* resE)
{
    _dSM->onUSSRequestError(resE->getOpcode());
}

Invoke * USSTcapListener::getInvId() const
{
    return orgInv;
}
/* ************************************************************************** *
 * class USSDSM implementation:
 * ************************************************************************** */
static void makeUI(vector<unsigned char> & ui, const Address & msadr, const char * vlr_adr)
{
    static const UCHAR_T map_open[] = {06,0x07,0x04,0x00,0x00,0x01,0x01,0x01,0x01,0xA0};

    unsigned char addrOcts[sizeof(TONNPI_ADDRESS_OCTS) + 1];
    ui.clear();

    unsigned addrLen = packMAPAddress2OCTS(msadr, (TONNPI_ADDRESS_OCTS*)addrOcts);
    ui.insert(ui.begin(), addrOcts, addrOcts + addrLen);
    ui.insert(ui.begin(), (unsigned char)addrLen);
    ui.insert(ui.begin(), 0x82);

    Address vlradr(vlr_adr);
    addrLen = packMAPAddress2OCTS(vlradr, (TONNPI_ADDRESS_OCTS*)addrOcts);
    ui.insert(ui.begin(), addrOcts, addrOcts + addrLen);
    ui.insert(ui.begin(), (unsigned char)addrLen);
    ui.insert(ui.begin(), 0x81);

    addrLen = ui.size();
    ui.insert(ui.begin(), (unsigned char)addrLen);
    ui.insert(ui.begin(), 0xA0);

    addrLen = ui.size();
    ui.insert(ui.begin(), (unsigned char)addrLen);
    ui.insert(ui.begin(), map_open, map_open + sizeof(map_open));

    addrLen = ui.size();
    ui.insert(ui.begin(), (unsigned char)addrLen);
    ui.insert(ui.begin(), 0x28);
}

USSDSM::USSDSM(VLR* vlr, int dsmId, Session* sess, Connect* conn)
    : _vLR (vlr)     //parent VLR
    , _dsmId (dsmId) //unique state machine id
    , _session(sess) //TCAP dialog factory
    , _connect(conn) //TCP connect
    , _ires(NULL)    //Invoke reult listener
    
{
    assert(sess);
    _dlg = new USSDialog(this, _session);
    assert(_dlg);
    _session->registerDialog(_dlg);
    logger = Logger::getInstance("smsc.inman.ussdsm");
}


USSDSM::~USSDSM()
{
    if (_ires) {
        Invoke* inv = _ires->getInvId();
        inv->setListener(NULL);
        delete _ires;
    }
    _session->closeDialog(_dlg); //frees Invoke
}

unsigned int USSDSM::getDSMId(void) const
{
    return _dsmId;
}

/* -------------------------------------------------------------------------- *
 * USSCommandHandler interface implementation:
 * -------------------------------------------------------------------------- */
//Adds info into request denial packet
void USSDSM::onDenyUSSRequest(USSRequestMessage* req)
{
    req->setObjectId(USS2CMD::PROCESS_USS_RESULT_TAG);
    req->setStatus(USS2CMD::STATUS_USS_REQUEST_DENIAL);
    //TO DO: Possibly add extra data to packet
}

//Creates request to Signal System and initiates dialog over TCAP,
//sets TCAP result listener.
void USSDSM::onProcessUSSRequest(USSRequestMessage* req)
{
    //create Component for TCAP request
    ProcessUSSRequestArg    arg;
    unsigned char           dcs = req->getDCS();

    if (dcs == USSMAN_LATIN1_DCS) { //plain text
        arg.setUSSData(&(req->getUSSData())[0], (req->getUSSData()).size());
        smsc_log_debug(logger, "DSM: incoming USS request[0x%X]: %.*s" , _dsmId,
                       (req->getUSSData()).size(), &(req->getUSSData())[0]);
    } else {
        arg.setRAWUSSData(dcs, &(req->getUSSData())[0], (req->getUSSData()).size());
        smsc_log_debug(logger, "DSM: incoming USS request[0x%X]: %s", _dsmId,
                        dump((req->getUSSData()).size(),
                             (unsigned char*)(&(req->getUSSData())[0]), 0).c_str());
    }

/* GVR NOTE: though MAP USS ASN.1 notation specifies that msISDN address   *
 * may be present in component portion of TCAP request, the Ericsson tools *
 * transfers it only in user info section.                                 */

    //prepare user info
    vector<unsigned char>   ui4;
    _msAdr = req->getMSISDNadr();
    makeUI(ui4, _msAdr, (_vLR->getCFG()).vlr_addr);

    //set result listener, and initiate TCAP dialog
    Invoke* op = _dlg->invoke( MAPUSS_OpCode::processUSS_Request );
    assert( op );
    op->setListener(_ires = new USSTcapListener(op, this));

    op->setParam( &arg );
    op->send( _dlg );
    _dlg->beginDialog(&ui4[0], ui4.size());
}

/* -------------------------------------------------------------------------- *
 * USSResultHandler interface implementation:
 * -------------------------------------------------------------------------- */
//Gets response from Signal System, sends USSRequest result back to socket
void USSDSM::onUSSRequestResult(ProcessUSSRequestRes* resL)
{
    USSResultMessage  resp;
    resp.setDialogId(_dsmId);
    resp.setRAWUSSData(resL->getDCS(), resL->getUSSData());
    resp.setMSISDNadr(_msAdr);

    if (logger->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG)) {
        std::string ussd;
        if (!resL->getUSSDataAsLatin1Text(ussd))
            ussd = dump((resp.getUSSData()).size(), (unsigned char*)&(resp.getUSSData())[0], 0);
        smsc_log_debug(logger, "DSM: USS request[0x%X] got result: %s", _dsmId, ussd.c_str());
    }
                   
    if (_connect)   //may be absent, in case of local testing
        _connect->send(&resp);
    _vLR->onCommandProcessed(this);
}

void USSDSM::onUSSRequestError(unsigned char ercode)
{
    smsc_log_error(logger, "DSM: USS request[0x%X] got error: %d", _dsmId, (int)ercode);

    USSResultMessage  resp;
    resp.setDialogId(_dsmId);
    resp.setStatus(USS2CMD::STATUS_USS_REQUEST_FAILED);
    if (_connect)   //may be absent, in case of local testing
        _connect->send(&resp);
    _vLR->onCommandProcessed(this);
}
/* ************************************************************************** *
 * class USSDialog implemetation:
 * ************************************************************************** */

USSDialog::USSDialog(USSDSM *pDsm, Session* pSession)
    : Dialog(pSession, 0, id_ac_map_networkUnstructuredSs_v2)
    , _pDsm(pDsm)
{
    assert(pDsm);
    assert(pSession);
}

USSDialog::~USSDialog()
{
}

USHORT_T USSDialog::handleEndDialog()
{
    smsc_log_debug(logger," USSDialog{id = %d} received END", this->getId());
//    _pDsm->onDialogEnd(this);
    return 0;
}

}//namespace uss
}//namespace inman
}//namespace smsc

