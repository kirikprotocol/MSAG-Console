#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */
/* ************************************************************************** *
 * INMan testing console
 * ************************************************************************** */
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdexcept>
#include <string>

#include "inman/common/console.hpp"
using smsc::inman::common::Console;

#include "inman/InTstDefs.hpp"
using smsc::inman::test::AbonentPreset;
using smsc::inman::test::AbonentType;
using smsc::inman::test::AbonentInfo;

#include "inman/InTstBill.hpp"
using smsc::inman::test::BillFacade;
using smsc::inman::test::INDialogCfg;

#include "inman/InTstDtcr.hpp"
using smsc::inman::test::DtcrFacade;

using smsc::inman::AbonentContractInfo;
using smsc::inman::interaction::ConnectSrv;
using smsc::inman::cdr::CDRRecord;

/* ************************************************************************** *
 * 
 * ************************************************************************** */
static const AbonentPreset  _abonents[] = {
    //Nezhinsky phone(prepaid):
     { AbonentContractInfo::abtPrepaid, ".1.1.79139343290", "250013900405871" }
    //tst phone (prepaid):
    ,{ AbonentContractInfo::abtPrepaid, ".1.1.79133821781", "250013903368782" }
    //Ryzhkov phone(postpaid):
    ,{ AbonentContractInfo::abtPostpaid, ".1.1.79139859489", "250013901464251" }
    //Stupnik phone(postpaid):
    ,{ AbonentContractInfo::abtPostpaid, ".1.1.79139033669", "250013901464251" }
    //
    ,{ AbonentContractInfo::abtUnknown,  ".5.0.smsx:201", 0 }
    ,{ AbonentContractInfo::abtUnknown,  ".5.0.ussd:448", 0 }
};
#define PRE_ABONENTS_NUM (sizeof(_abonents)/sizeof(AbonentPreset))

class ConnectionClosedException : public std::exception {
public:
    const char* what() const throw()
    { return "IN manager doesn't connected"; }
};

static BillFacade* _billFacade = 0;
static DtcrFacade* _dtcrFacade = 0;

/* ************************************************************************** *
 * Console commands: sending INMan commands
 * ************************************************************************** */

static void utl_multi_charge(const std::vector<std::string> &args, uint32_t delivery = 0)
{
    if (_billFacade->isActive()) {
        unsigned int dnum = 10;
        if ((args.size() > 1) && !(dnum = (unsigned int)atoi(args[1].c_str()))) {
            fprintf(stdout, "USAGE: %s num_of_dialogs!\n", args[0].c_str());
            return;
        }
        AbonentsDB * abDb = AbonentsDB::getInstance();
        INDialogCfg  cfg = *(_billFacade->getDlgConfig());
        for (; dnum > 0; dnum--, cfg.abId = abDb->nextId(cfg.abId)) {
            unsigned int did = _billFacade->initDialog(0, true, delivery, &cfg);
            _billFacade->sendChargeSms(did);
        }
    } else
        throw ConnectionClosedException();
}
//USAGE: m_chargeOk [num_dialogs [dflt = 10]]
//sends specified number of ChargeSMS requests with successfull delivery, iterating over abonentsDB.
void cmd_multi_chargeOk(Console&, const std::vector<std::string> &args)
{
    utl_multi_charge(args, 0);
}
//USAGE: m_chargeErr [num_dialogs [dflt = 10]]
//sends specified number of ChargeSMS requests with failed delivery, iterating over abonentsDB.
void cmd_multi_chargeErr(Console&, const std::vector<std::string> &args)
{
    utl_multi_charge(args, 1016);
}

void cmd_charge(Console&, const std::vector<std::string> &args)
{
    if (_billFacade->isActive()) {
        unsigned int did = 0;

        if (args.size() > 1) {
            if (!(did = (unsigned int)atoi(args[1].c_str()))) {
                fprintf(stdout, "ERR: bad dialog id specified (%s)!\n", args[1].c_str());
                return;
            }
        }
        if (!did || !_billFacade->findDialog(did))
            did = _billFacade->initDialog(did);
        _billFacade->sendChargeSms(did);
    } else
        throw ConnectionClosedException();
}

static void utl_charge(const std::vector<std::string> &args, uint32_t delivery_status)
{
    if (_billFacade->isActive()) {
        unsigned int did = _billFacade->initDialog(0, true, delivery_status);
        _billFacade->sendChargeSms(did);
    } else
        throw ConnectionClosedException();
}
void cmd_chargeOk(Console&, const std::vector<std::string> &args)
{
    utl_charge(args, 0);
}
void cmd_chargeErr(Console&, const std::vector<std::string> &args)
{
    utl_charge(args, 1016);
}

static void utl_chargeCD(const std::vector<std::string> &args, uint32_t delivery_status)
{
    if (_billFacade->isActive()) {
        unsigned int did = _billFacade->initDialog(0, true, delivery_status);
        _billFacade->sendDeliveredSmsData(did);
    } else
        throw ConnectionClosedException();
}
void cmd_chargeCDOk(Console&, const std::vector<std::string> &args)
{
    utl_chargeCD(args, 0);
}
void cmd_chargeCDErr(Console&, const std::vector<std::string> &args)
{
    utl_chargeCD(args, 1016);
}


static void utl_reportDlg(const std::vector<std::string> &args, uint32_t delivery_status)
{
    if (_billFacade->isActive()) {
        unsigned int did = 0;

        if (args.size() > 1)
            did = (unsigned int)atoi(args[1].c_str());
        if (!did) {
            fprintf(stdout, "ERR: dialog id %s is bad or missed!\n",
                    (args.size() > 1) ? args[1].c_str() : "");
            return;
        }
        _billFacade->sendDeliverySmsResult(did, delivery_status);
    } else
        throw ConnectionClosedException();
}
void cmd_reportOk(Console&, const std::vector<std::string> &args)
{
    utl_reportDlg(args, 0);
}
void cmd_reportErr(Console&, const std::vector<std::string> &args)
{
    utl_reportDlg(args, 1016);
}

static unsigned utl_cmdExc(const std::vector<std::string> &args, uint32_t & size)
{
    if (!_billFacade->isActive())
        throw ConnectionClosedException();
    unsigned dlgId = 0;

    size = 0;
    if (args.size() > 1) {
        if (!(size = (uint32_t)atoi(args[1].c_str()))) {
            fprintf(stdout, "ERR: num_bytes %s is invalid!\n", args[1].c_str());
            return 0;
        }
    }
    if ((args.size() > 2) && !(dlgId = (unsigned)atoi(args[2].c_str())))
        fprintf(stdout, "WRN: dialog id %s is invalid!\n", args[1].c_str());
    if (!dlgId || !_billFacade->findDialog(dlgId))
        dlgId = _billFacade->getNextDialogId();
    return dlgId;
}
//USAGE: chargeExc [num_bytes] [dlgId]
//sends specified number of bytes of ChargeSMS packet, causing exception on remote point
void cmd_chargeExc(Console&, const std::vector<std::string> &args)
{
    uint32_t size;
    unsigned dlgId = utl_cmdExc(args, size);
    _billFacade->sendChargeSms(dlgId, size ? size : 8); //send prefix only by default
}
//USAGE: dlvrExc [num_bytes] [dlgId]
//sends specified number of bytes of DeliverySmsResult packet, causing exception on remote point
void cmd_dlvrExc(Console&, const std::vector<std::string> &args)
{
    uint32_t size;
    unsigned dlgId = utl_cmdExc(args, size);
    _billFacade->sendDeliverySmsResult(dlgId, 1179, size ? size : 8); //send prefix only by default
}

/* ************************************************************************** *
 * Console commands: setting INMan dialog params
 * ************************************************************************** */

void cmd_config(Console&, const std::vector<std::string> &args)
{
    _billFacade->printDlgConfig();
}

void cmd_dpsms(Console&, const std::vector<std::string> &args)
{
    _billFacade->setUssdOp(false);
    _billFacade->printDlgConfig();
}

void cmd_dpussd(Console&, const std::vector<std::string> &args)
{
    _billFacade->setUssdOp(true);
    _billFacade->printDlgConfig();
}

//USAGE: use_abn [?|help | abn_NN]
static const char hlp_use_abnts[] = "USAGE: %s [?|help | org.abn_NN] [dst.abn_NN]\n";
static void cmd_use_abnts(Console&, const std::vector<std::string> &args)
{
    unsigned abId = 0;

    if (args.size() ==  1) {
        fprintf(stdout, hlp_use_abnts, args[0].c_str());
        fprintf(stdout, "Known abonents:\n");
        AbonentsDB::getInstance()->printAbonents(stdout);
        return;
    }
    if (args.size() > 1) {
        if (!strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
            fprintf(stdout, hlp_use_abnts, args[0].c_str());
            return;
        }
        abId = (unsigned)atoi(args[1].c_str());
        if (!abId) {
            fprintf(stdout, "ERR: invalid abonent number: %s\n", args[1].c_str());
            return;
        }
        if (!_billFacade->setAbonentId(abId, true)) {
            fprintf(stdout, "ERR: Unknown abonent #%u\n", abId);
            return;
        }
    }
    if (args.size() > 2) {
        abId = (unsigned)atoi(args[2].c_str());
        if (!abId) {
            fprintf(stdout, "ERR: invalid abonent number: %s\n", args[2].c_str());
            return;
        }
        if (!_billFacade->setAbonentId(abId, false)) {
            fprintf(stdout, "ERR: Unknown abonent #%u\n", abId);
            return;
        }
    }
    _billFacade->printDlgConfig();
}

static const char hlp_use_dabn[] = "USAGE: %s [?|help | dst.abn_NN]\n";
void cmd_use_dabn(Console&, const std::vector<std::string> &args)
{
    unsigned abId = 0;

    if (args.size() ==  1) {
        fprintf(stdout, hlp_use_dabn, args[0].c_str());
        fprintf(stdout, "Known abonents:\n");
        AbonentsDB::getInstance()->printAbonents(stdout);
        return;
    }
    if (args.size() > 1) {
        if (!strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
            fprintf(stdout, hlp_use_dabn, args[0].c_str());
            return;
        }
        abId = (unsigned)atoi(args[1].c_str());
        if (!abId) {
            fprintf(stdout, "ERR: invalid abonent number: %s\n", args[1].c_str());
            return;
        }
        if (!_billFacade->setAbonentId(abId, false)) {
            fprintf(stdout, "ERR: Unknown abonent #%u\n", abId);
            return;
        }
    }
    _billFacade->printDlgConfig();
}

static const char hlp_contract[] = "USAGE: %s [?|help | org.abn_NN]\n";
static void utl_next_abn(const std::vector<std::string> &args, AbonentType ab_type)
{
    unsigned abId = 0;

    if (args.size() > 1) {
        if (!strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
            fprintf(stdout, hlp_contract, args[0].c_str());
            return;
        }
        abId = (uint32_t)atoi(args[1].c_str());
    }
    abId = AbonentsDB::getInstance()->searchNextAbn(ab_type, abId);
    if (!abId)
        fprintf(stdout, "ERR: no known abonent found!");
    else {
        _billFacade->setAbonentId(abId, true);
        _billFacade->printDlgConfig();
    }
    return;
}
//USAGE: prepaid [?|help | abn_NN]
void cmd_prepaid(Console&, const std::vector<std::string> &args)
{
    utl_next_abn(args, AbonentInfo::abtPrepaid);
}
//USAGE: postpaid [?|help | abn_NN]
void cmd_postpaid(Console&, const std::vector<std::string> &args)
{
    utl_next_abn(args, AbonentInfo::abtPostpaid);
}


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
    _billFacade->setSmsXIds((uint32_t)val);
    _billFacade->printDlgConfig();
}


void cmd_no_xsms(Console&, const std::vector<std::string> &args)
{
    _billFacade->setSmsXIds(0);
    _billFacade->printDlgConfig();
}

//USAGE: chg_mode_abn [?|help | submit | delivery | alldata]
static const char hlp_chg_policy[] = "USAGE: %s [?|help | submit | delivery | allData | submitData]\n";
void cmd_charge_policy(Console&, const std::vector<std::string> &args)
{
    if ((args.size() < 2)
        || !strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
        fprintf(stdout, hlp_chg_policy, args[0].c_str());
        return;
    }
    const char * mode = args[1].c_str();
    CDRRecord::ChargingPolicy chg_policy = CDRRecord::ON_DELIVERY;
    if (!strcmp(mode, "submit")) {
        chg_policy = CDRRecord::ON_SUBMIT;
    } else if (!strcmp(mode, "allData")) {
        chg_policy = CDRRecord::ON_DATA_COLLECTED;
    } else if (!strcmp(mode, "submitData")) {
        chg_policy = CDRRecord::ON_SUBMIT_COLLECTED;
    } else if (strcmp(mode, "delivery")) {
        fprintf(stdout, "ERR: invalid charging mode: %s!", mode);
        return;
    }
    _billFacade->setChargePolicy(chg_policy);
    _billFacade->printDlgConfig();
}

void cmd_charge_type_MO(Console&, const std::vector<std::string> &args)
{
    _billFacade->setChargeType(CDRRecord::MO_Charge);
    _billFacade->printDlgConfig();
}

void cmd_charge_type_MT(Console&, const std::vector<std::string> &args)
{
    _billFacade->setChargeType(CDRRecord::MT_Charge);
    _billFacade->printDlgConfig();
}

//USAGE: cdr_charge [?|help | on | off]
static const char hlp_cdr_charge[] = "USAGE: %s [?|help | .Ton.Npi.Adr | string]\n";

void cmd_cdr_charge(Console&, const std::vector<std::string> &args)
{
  if ((args.size() < 2)
      || !strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
    fprintf(stdout, hlp_cdr_charge, args[0].c_str());
    return;
  }

  if (!strcmp("on", args[1].c_str())) {
    _billFacade->setForcedCDR(true);
  } else if (!strcmp("off", args[1].c_str())) {
    _billFacade->setForcedCDR(false);
  } else {
    fprintf(stdout, hlp_cdr_charge, args[0].c_str());
    return;
  }
  _billFacade->printDlgConfig();
}

//USAGE: chg_mode_abn [?|help | submit | delivery | alldata]
static const char hlp_org_msc[] = "USAGE: %s [?|help | .Ton.Npi.Adr | string]\n";

void cmd_org_msc(Console&, const std::vector<std::string> &args)
{
  if ((args.size() < 2)
      || !strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
      fprintf(stdout, hlp_org_msc, args[0].c_str());
      return;
  }

  std::string adrStr = args[1];
  for (unsigned i = 2; i < args.size(); ++i) {
      adrStr += ' '; adrStr += args[i];
  }
  TonNpiAddress tadr;
  if (!tadr.fromText(adrStr.c_str())) {
      fprintf(stdout, "ERR: invalid MSC address: %s!", adrStr.c_str());
      return;
  }

  _billFacade->setLocMSC(adrStr);
  _billFacade->printDlgConfig();
}


/* ************************************************************************** *
 * Console commands: sending INMan AbonentDetector commands 
 * ************************************************************************** */
//USAGE: detect [?|help | abn.NN | .T.N.Addr | +ISDN_Addr] [cacheMode:{true|false}]
static const char hlp_detect_abn[] = "USAGE: %s [?|help | abn.NN | .T.N.Addr | +ISDN_Addr] [cacheMode:{true|false}]\n";
void cmd_detect_abn(Console&, const std::vector<std::string> &args)
{
    if ((args.size() < 2)
        || !strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
        fprintf(stdout, hlp_detect_abn, args[0].c_str());
        return;
    }
    bool use_cache = true;
    if ((args.size() >= 3) && !strcmp("false", args[2].c_str()))
        use_cache = false;

    if (!strncmp("abn.", args[1].c_str(), 4)) {
        unsigned abId = (unsigned)atoi(args[1].c_str() + 4);
        if (!abId) {
            fprintf(stdout, "ERR: invalid abonentId %s!\n", args[1].c_str() + 4);
            return;
        }
        _dtcrFacade->detectAbn(abId, use_cache);
        return;
    }
    TonNpiAddress sbscr;
    if (!sbscr.fromText(args[1].c_str())) {
        fprintf(stdout, "ERR: invalid abonent address %s!\n", args[1].c_str());
    } else {
        _dtcrFacade->detectAbn(sbscr, use_cache);
    }
}

//USAGE: detect [?|help | abn.NN |.T.N.Addr|+ISDN_Addr] [range] [cacheMode:{true|false}]
static const char hlp_detect_mlt[] = "USAGE: %s [?|help | abn.NN | .T.N.Addr | +ISDN_Addr] [num_of_abnt] [cacheMode:{true|false}]\n";
void cmd_detect_mlt(Console&, const std::vector<std::string> &args)
{
    if ((args.size() < 2)
        || !strcmp("?", args[1].c_str()) || !strcmp("help", args[1].c_str())) {
        fprintf(stdout, hlp_detect_mlt, args[0].c_str());
        return;
    }
    bool use_cache = true;
    if ((args.size() >= 4) && !strcmp("false", args[3].c_str()))
        use_cache = false;

    unsigned range = 10;
    if ((args.size() >= 3) && !(range = (unsigned)atoi(args[2].c_str()))) {
        fprintf(stdout, "ERR: invalid range %s!\n", args[2].c_str());
        return;
    }

    if (!strncmp("abn.", args[1].c_str(), 4)) {
        unsigned abId = (unsigned)atoi(args[1].c_str() + 4);
        if (!abId) {
            fprintf(stdout, "ERR: invalid abonentId %s!\n", args[1].c_str() + 4);
            return;
        }
        _dtcrFacade->detectAbnMlt(abId, range, use_cache);
        return;
    }
    TonNpiAddress sbscr;
    if (!sbscr.fromText(args[1].c_str())) {
        fprintf(stdout, "ERR: invalid abonent address %s!\n", args[1].c_str());
    } else {
        _dtcrFacade->detectAbnMlt(sbscr, range, use_cache);
    }
    return;
}


/* ************************************************************************** *
 * MAIN:
 * ************************************************************************** */
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
    Logger * _logger = Logger::getInstance("smsc.InTST");

    AbonentsDB::Init((unsigned)PRE_ABONENTS_NUM, _abonents);

    std::auto_ptr<ConnectSrv> _connServ(new ConnectSrv(ConnectSrv::POLL_TIMEOUT_ms, _logger));
    try {
        _billFacade = new BillFacade(_connServ.get(), _logger);
        _dtcrFacade = new DtcrFacade(_connServ.get(), _logger);
        Console console;
/* ************************************************************************** *
 * Console commands: sending Billing commands to INMan
 * ************************************************************************** */
        console.addItem("charge", cmd_charge); //chargeSMS -> 
        console.addItem("chargeOk", cmd_chargeOk); //chargeSMS -> reportOk
        console.addItem("chargeErr",  cmd_chargeErr);//chargeSMS -> reportErr
        console.addItem("reportOk",  cmd_reportOk);
        console.addItem("reportErr",  cmd_reportErr);
        console.addItem("chargeExc",  cmd_chargeExc);
        console.addItem("dlvrExc",  cmd_dlvrExc); 
        console.addItem("m_chargeOk", cmd_multi_chargeOk); //[chargeSMS -> reportOk]
        console.addItem("m_chargeErr",  cmd_multi_chargeErr);//[chargeSMS -> reportErr]
        /**/
        console.addItem("chargeCDOk", cmd_chargeCDOk); //DeliveredSMSData(Ok) -> 
        console.addItem("chargeCDErr",  cmd_chargeCDErr);//DeliveredSMSData(Ok) -> 
/* ************************************************************************** *
 * Console commands: setting INMan Billing dialog params
 * ************************************************************************** */
        console.addItem("config",  cmd_config);
        console.addItem("dpsms",  cmd_dpsms);
        console.addItem("dpussd",  cmd_dpussd);
        /**/
        console.addItem("use_abn",  cmd_use_abnts);
        console.addItem("use_dabn",  cmd_use_dabn);
        console.addItem("prepaid",  cmd_prepaid);
        console.addItem("postpaid",  cmd_postpaid);
        /**/
        console.addItem("use_xsms",  cmd_use_xsms);
        console.addItem("no_xsms",  cmd_no_xsms);
        /**/
        console.addItem("chg_policy",  cmd_charge_policy);
        console.addItem("mo_charge",  cmd_charge_type_MO);
        console.addItem("mt_charge",  cmd_charge_type_MT);
        console.addItem("org_msc", cmd_org_msc);
        console.addItem("cdr_charge",  cmd_cdr_charge);
/* ************************************************************************** *
 * Console commands: sending INMan AbonentDetector commands 
 * ************************************************************************** */
        console.addItem("detect",  cmd_detect_abn);
        console.addItem("m_detect",  cmd_detect_mlt);
/* -------------------------------------------------------------------------- */
        _connServ->Start();
        _billFacade->initConnect(host, port);
        _dtcrFacade->initConnect(host, port);
        console.run("inman>");
    } catch (const std::exception& error) {
        fprintf(stderr, error.what());
    }
    _billFacade->Disconnect();
    _dtcrFacade->Disconnect();
    _connServ->Stop();
    delete _billFacade;
    delete _dtcrFacade;
    exit(0);
}