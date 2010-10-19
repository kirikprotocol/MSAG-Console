/* ************************************************************************** *
 * Simple console application testing MAP Any Time Subscription Interrogation
 * service of HLR.
 * ************************************************************************** */
#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <string>

#include "inman/common/console.hpp"
using smsc::inman::common::Console;

#include "inman/GsmSCFInfo.hpp"
using smsc::inman::GsmSCFinfo;

#include "inman/utilbin/URCInitMAP.hpp"
#include "inman/utilbin/atih_srv.hpp"
using smsc::inman::ATCSIListener;
using smsc::inman::ServiceATIH;
using smsc::inman::ServiceATIH_CFG;
using smsc::inman::inap::MAPUsrCfgReader;

#include "inman/utilbin/AbonentsDb.hpp"
using smsc::inman::AbonentsDb;
using smsc::inman::AbonentInfo;

#include "util/config/XCFManager.hpp"
using smsc::util::config::XCFManager;
using smsc::util::config::Config;
using smsc::util::config::ConfigException;

static char         _runService = 0;
static ServiceATIH* g_pService = 0;

extern "C" static void sighandler(int signal)
{
    _runService = 0;
}


//GLOBAL:
AbonentsDb  abnData;


class ATIHClient: public ATCSIListener {
private:
    FILE * outStream;

public:
    ATIHClient(FILE * out_stream)
        : outStream(out_stream)
    { }
    ~ATIHClient()
    { }
    //-- ATCSIListener interface
    void onCSIresult(const std::string &subcr_addr, const GsmSCFinfo* scfInfo)
    {
        unsigned ab_id = abnData.lookUp(subcr_addr);
        if (ab_id) {
            abnData.setAbnInfo(ab_id, scfInfo);
            abnData.printAbnInfo(outStream, ab_id);
        } else
            fprintf(outStream, "ERR: CSI result for unregistered subscriber %s",
                    subcr_addr.c_str());
        return;
    }
    void onCSIabort(const std::string &subcr_addr, RCHash ercode)
    {
        fprintf(outStream, "ERR: CSI request for subscriber %s failed: code %u, %s\n",
                subcr_addr.c_str(), ercode, URCRegistry::explainHash(ercode).c_str());
    }
};


#define NOT_IMSI(addr) ((addr).typeOfNumber || ((addr).numPlanInd != 1))
/* ------------------------------------------------------------------- *
 * Static data
 * ------------------------------------------------------------------- */ 
enum FlagIdx_e {
  idxODB = 0, idxVLR_Phases, idxSGSN_Phases
};

static const char * const _nmFlags[] = {
  "ODB", "VLR_Phases", "SGSN_Phases"
};

#define FLAGS_IDS_NUM (sizeof(_nmFlags)/sizeof(const char *))

static std::string _listOfFlags;

static void init_listOfFlags(void)
{
  for (unsigned i = 0; i < FLAGS_IDS_NUM; ++i)
    format(_listOfFlags, "%s%s", i ? ", " : "", _nmFlags[i]);
}

//Retuns index in _nmFlags[] array or -1 if unknown
static int isKnownFlag(const char * nm_flag)
{
  for (unsigned i = 0; i < FLAGS_IDS_NUM; ++i) {
    if (!strcmp(nm_flag, _nmFlags[i])) {
      return (int)i;
    }
  }
  return -1;
}

/* ------------------------------------------------------------------- */
static const char * const _nmCSI[] = {
    "O"       // = 0
  , "T"       // = 1
  , "VT"      // = 2
  , "TIF"     // = 3
  , "GPRS"    // = 4
  , "MO_SMS"  // = 5
  , "SS"      // = 6
  , "M"       // = 7
  , "D"       // = 8
  //additional CSI follows
  , "MT_SMS"  // = 9 + 0
  , "MG"      // = 9 + 1
  , "O_IM"    // = 9 + 2
  , "D_IM"    // = 9 + 3
  , "VT_IM"   // = 9 + 4
};

#define CSI_IDS_NUM (sizeof(_nmCSI)/sizeof(const char *))

static std::string _listOfCSI;

static const unsigned _xCSI_nmIdx = 9;

static const char * _nmXCSI(AdditionalRequestedCAMEL_SubscriptionInfo_e id_xcsi)
{
  return (id_xcsi <= AdditionalRequestedCAMEL_SubscriptionInfo_vt_IM_CSI) ?
          _nmCSI[_xCSI_nmIdx + id_xcsi] : "unknown";
};

static void init_listOfCSI(void)
{
  for (unsigned i = 0; i < CSI_IDS_NUM; ++i)
    format(_listOfCSI, "%s%s", i ? ", " : "", _nmCSI[i]);
}

//Retuns index in _nmCSI[] array or -1 if unknown
static int isKnownCSI(const char * nm_csi)
{
  for (unsigned i = 0; i < CSI_IDS_NUM; ++i) {
    if (!strcmp(nm_csi, _nmCSI[i])) {
      return (int)i;
    }
  }
  return -1;
}

static void setRequestedCSI(RequestedSubscription & reqCfg, unsigned nm_csi_idx, bool is_mark)
{
  if (nm_csi_idx < _xCSI_nmIdx) {
    if (is_mark)
      reqCfg.setRequestedCSI(static_cast<RequestedCAMEL_SubscriptionInfo_e>(nm_csi_idx));
    else
      reqCfg.unsetRequestedCSI();
  } else {
    nm_csi_idx -= _xCSI_nmIdx;
    if (is_mark)
      reqCfg.setRequestedXCSI(static_cast<AdditionalRequestedCAMEL_SubscriptionInfo_e>(nm_csi_idx));
    else
      reqCfg.unsetRequestedXCSI();
  }
}

static void printConfig(const RequestedSubscription & reqCfg)
{
  fprintf(stdout, "RequestedSubscription settings: \n");

  if (reqCfg.getFlags().ss) {
    std::string ostr;
    format(ostr, "%2Xh", reqCfg.getRequestedSSInfo().ssCode);
    if (reqCfg.getRequestedSSInfo().bsCodeKind == BasicServiceCode_PR_bearerService)
      format(ostr, " for bearerService %2Xh", reqCfg.getRequestedSSInfo().bsCode);
    else if (reqCfg.getRequestedSSInfo().bsCodeKind == BasicServiceCode_PR_teleservice)
      format(ostr, " for teleService %2Xh", reqCfg.getRequestedSSInfo().bsCode);

    fprintf(stdout, "  SS_ForBS:    yes (%s)\n", ostr.c_str());
  } else
    fprintf(stdout, "  SS_ForBS:    no\n");

  fprintf(stdout, "  ODB:         %s\n", reqCfg.getFlags().odb ? "yes" : "no");

  if (reqCfg.getFlags().csi)
    fprintf(stdout, "  CSI:         yes (%s_CSI)\n", _nmCSI[reqCfg.getRequestedCSI()]);
  else
    fprintf(stdout, "  CSI:         no\n");

  if (reqCfg.getFlags().xcsi)
    fprintf(stdout, "  xCSI:        yes (%s_CSI)\n", _nmXCSI(reqCfg.getRequestedXCSI()));
  else
    fprintf(stdout, "  xCSI:        no\n");

  fprintf(stdout, "  VLR_Phases:  %s\n", reqCfg.getFlags().vlr ? "yes" : "no");
  fprintf(stdout, "  SGSN_Phases: %s\n", reqCfg.getFlags().sgsn ? "yes" : "no");
}

/* ************************************************************************** *
 * Console commands:
 * ************************************************************************** */

void cmd_config(Console&, const std::vector<std::string> &args)
{
  printConfig(g_pService->getRequestCfg());
}

/* ------------------------------------------------------------------- */
//> mark_csi [csi_id_str]
static const char hlp_switch_csi[] = "USAGE: %smark_csi [csi_id_str]\n"
                                  "       csi_id_str s one of:\n       %s\n";
void utl_switch_csi(const std::vector<std::string> &args, bool is_mark)
{
  if (args.size() < 2) {
    fprintf(stdout, hlp_switch_csi, is_mark ? "" : "un", _listOfCSI.c_str());
    return;
  }

  int nmCsiIdx = isKnownCSI(args[1].c_str());
  if (nmCsiIdx < 0) {
    fprintf(stdout, "ERROR: unknwon CSI id (%s)\n", args[1].c_str());
    fprintf(stdout, hlp_switch_csi, is_mark ? "" : "un", _listOfCSI.c_str());
    return;
  }
  setRequestedCSI(g_pService->getRequestCfg(), (unsigned)nmCsiIdx, is_mark);
  printConfig(g_pService->getRequestCfg());
}

void cmd_mark_csi(Console&, const std::vector<std::string> &args)
{
  utl_switch_csi(args, true);
}
void cmd_unmark_csi(Console&, const std::vector<std::string> &args)
{
  utl_switch_csi(args, false);
}

/* ------------------------------------------------------------------- */
//> mark_flag [flag_id_str]
static const char hlp_switch_flag[] = "USAGE: %smark_flag [flag_id_str]\n"
                                  "       flag_id_str s one of:\n       %s\n";

void utl_switch_flag(const std::vector<std::string> &args, bool is_mark)
{
  if (args.size() < 2) {
    fprintf(stdout, hlp_switch_flag, is_mark ? "" : "un", _listOfFlags.c_str());
    return;
  }

  int nmIdx = isKnownFlag(args[1].c_str());
  if (nmIdx < 0) {
    fprintf(stdout, "ERROR: unknwon flag id (%s)\n", args[1].c_str());
    fprintf(stdout, hlp_switch_flag, is_mark ? "" : "un", _listOfFlags.c_str());
    return;
  }
  if (nmIdx == idxODB) {
    g_pService->getRequestCfg().setRequestedODB(is_mark);
  } else if (nmIdx == idxVLR_Phases) {
    g_pService->getRequestCfg().setRequestedVLRPhases(is_mark);
  } else if (nmIdx == idxSGSN_Phases) {
    g_pService->getRequestCfg().setRequestedSGSNPhases(is_mark);
  }
  printConfig(g_pService->getRequestCfg());
}

void cmd_mark_flag(Console&, const std::vector<std::string> &args)
{
  utl_switch_flag(args, true);
}

void cmd_unmark_flag(Console&, const std::vector<std::string> &args)
{
  utl_switch_flag(args, false);
}

/* ------------------------------------------------------------------- */
//> set_abn ['.1.1.signals' | 'isdn_number'] ['imsi_number']
static const char hlp_set_abn[] = "USAGE: set_abn ['.1.1.signals' | 'isdn_number'] ['imsi_number']\n";
void cmd_set_abn(Console&, const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        fprintf(stdout, hlp_set_abn);
        abnData.printAbnInfo(stdout, 0, 0);
        return;
    }
    TonNpiAddress   adr[2]; //msisdn, imsi

    if (args.size() >= 2) { //msisdn
        if (!adr[0].fromText(args[1].c_str()) || !adr[0].interISDN()) {
            fprintf(stdout, "ERR: bad MSISDN");
            fprintf(stdout, hlp_set_abn);
            return;
        }
    }
    if (args.size() >= 3) { //imsi: (isdn, unknown)
        if (!adr[1].fromText(args[2].c_str()) || NOT_IMSI(adr[1])) {
            fprintf(stdout, "ERR: bad IMSI");
            fprintf(stdout, hlp_set_abn);
            return;
        }
    }
    abnData.addAbonent(&adr[0], adr[1].signals);
}

/* ------------------------------------------------------------------- */
//requests SubscriptionInfo according to RequestedSubscription settings
void cmd_get_csi(Console&, const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        fprintf(stdout, "USAGE: get_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
        return;
    }
    bool        imsi = false;
    std::string subscr;
    unsigned    abId = 0;

    const char * s_abn = args[1].c_str();
    if (!strncmp(s_abn, "abn.", 4)) {       //abonent id
        abId = (unsigned)atoi(s_abn + 4);
        if (!abId || abId > abnData.getMaxAbId()) {
            fprintf(stdout, "ERR:  %s abonent Id %s\n", !abId ? "bad" : "unknown", s_abn);
            fprintf(stdout, "USAGE: get_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
            return;
        }
        AbonentInfo ab_inf = *abnData.getAbnInfo(abId);
        subscr = ab_inf.addr.toString();
    } else if (!strncmp(s_abn, ".1.1.", 5)) { //msisdn
        TonNpiAddress addr;
        if (!addr.fromText(s_abn)) {
            fprintf(stdout, "ERR:  bad MSISDN %s\n", s_abn);
            fprintf(stdout, "USAGE: get_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
            return;
        }
        abId = abnData.addAbonent(&addr, NULL);
        subscr = addr.toString();
    } else {                                //imsi or unknown
        TonNpiAddress addr;
        if (!addr.fromText(s_abn)) {
            fprintf(stdout, "ERR:  bad IMSI or MSISDN: %s\n", s_abn);
            fprintf(stdout, "USAGE: get_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
            return;
        }
        if (NOT_IMSI(addr)) {
            fprintf(stdout, "ERR:  bad IMSI or MSISDN %s\n", s_abn);
            fprintf(stdout, "USAGE: get_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
            return;
        }
        fprintf(stdout, "[get_csi]: %s\n", addr.getSignals());
        fprintf(stdout, "ERR:  IMSI is  not supported yet.\n");
        return;
/*
        abId = abnData.addAbonent(NULL, &addr);
        subscr = addr.toString();
        imsi = true;
*/
    }
    fprintf(stdout, "[get_csi] abn.%u: %s, imsi = %s\n",  abId, subscr.c_str(),
            imsi ? "true" : "false");
    if (!g_pService->requestCSI(subscr, imsi))
        fprintf(stdout, "[get_csi] failed to request CSI");
}

static const char * const _nmTst = "ATIH";
int main(int argc, char** argv)
{
    int     rval = 0;
    const char *  cfgFile = "config.maptst.xml";

    init_listOfFlags();
    init_listOfCSI();
    tzset();
    URCRegistryInit4MAP();
    Logger::Init();
    Logger * rootLogger = Logger::getInstance("smsc.inman");
    
    smsc_log_info(rootLogger,"*************************");
    smsc_log_info(rootLogger,"* SIBINCO MAP %s TEST *", _nmTst);
    smsc_log_info(rootLogger,"*************************");
    if (argc > 1)
        cfgFile = argv[1];
    smsc_log_info(rootLogger,"* Config file: %s", cfgFile);
    smsc_log_info(rootLogger,"******************************");

    ServiceATIH_CFG  tstCfg;
    try {
        std::auto_ptr<Config> config(XCFManager::getInstance().getConfig(cfgFile)); //throws
        MAPUsrCfgReader parser(_nmTst, rootLogger);
        parser.readConfig(*config.get()); //throws
        tstCfg.mapCfg = *parser.getConfig();
    } catch (const ConfigException & exc) {
        smsc_log_error(rootLogger, "%s: %s", _nmTst, exc.what());
        smsc_log_error(rootLogger, "%s: Exiting!", _nmTst);
        exit(-1);
    }
    abnData.Init();
    ATIHClient  atihClient(stdout);
    tstCfg.client = &atihClient;

    try {
        g_pService = new ServiceATIH(tstCfg, rootLogger);
        Console console;

        console.addItem("config", cmd_config);
        console.addItem("mark_csi", cmd_mark_csi);
        console.addItem("unmark_csi", cmd_unmark_csi);
        console.addItem("mark_flag", cmd_mark_flag);
        console.addItem("unmark_flag", cmd_unmark_flag);

        console.addItem("set_abn", cmd_set_abn);
        console.addItem("get_csi", cmd_get_csi);

        _runService = 1;
        if (g_pService->start()) {
            //handle SIGTERM only in main thread
            sigset(SIGTERM, sighandler);
        } else {
            smsc_log_fatal(rootLogger, "%s: ATIHSrv startup failure. Exiting.", _nmTst);
            _runService = 0;
            rval = 1;
        }
        if (_runService)
            console.run("ATIHSrv>");
        g_pService->stop();

    } catch(const std::exception& error) {
        smsc_log_fatal(rootLogger, "%s: %s", _nmTst, error.what());
        fprintf(stdout, "%s: Fatal error: %s\n", _nmTst, error.what());
        rval = 1;
    }
    if (g_pService)
        delete g_pService;
    smsc_log_info(rootLogger, "%s TEST shutdown complete", _nmTst);
    return rval;
}

