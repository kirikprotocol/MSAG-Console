/* ************************************************************************** *
 * Simple console application testing MAP Any Time Subscription Interrogation
 * service of HLR.
 * ************************************************************************** */
#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <string>

#include "inman/common/console.hpp"
using smsc::inman::common::Console;

#include "inman/utilbin/URCInitMAP.hpp"
#include "inman/utilbin/atih_srv.hpp"
using smsc::inman::comp::CSIUid_e;
using smsc::inman::comp::UnifiedCSI;
using smsc::inman::comp::atih::RequestedSubscription;

using smsc::inman::AT_CSIListenerIface;
using smsc::inman::ServiceATIH;
using smsc::inman::ServiceATIH_CFG;
using smsc::inman::inap::MAPUsrCfgReader;

#include "inman/tests/AbonentsDB.hpp"
using smsc::inman::AbonentContractInfo;
using smsc::inman::test::AbonentsDB;
using smsc::inman::test::AbonentInfo;
using smsc::inman::test::AbonentPreset;

using smsc::util::IMSIString;

#include "util/config/XCFManager.hpp"
using smsc::util::config::XCFManager;
using smsc::util::config::Config;
using smsc::util::config::ConfigException;

static char         _runService = 0;
static ServiceATIH * g_pService = 0;

extern "C" void sighandler(int signal)
{
  _runService = 0;
}


//GLOBAL:
smsc::inman::test::AbonentsDB * _abonentsData = NULL;

static const AbonentPreset  _abonents[] = {
  //Nezhinsky phone(prepaid):
    AbonentPreset(AbonentContractInfo::abtPrepaid, ".1.1.79139343290", "250013900405871")
  //tst phone (prepaid):
  , AbonentPreset(AbonentContractInfo::abtPrepaid, ".1.1.79133821781", "250013903368782")
  //Ryzhkov phone(postpaid):
  , AbonentPreset(AbonentContractInfo::abtPostpaid, ".1.1.79139859489", "250013901464251")
  //Stupnik phone(postpaid):
  , AbonentPreset(AbonentContractInfo::abtPostpaid, ".1.1.79139033669", "250013901464251")
};
#define PRE_ABONENTS_NUM (sizeof(_abonents)/sizeof(AbonentPreset))


class ATIHClient: public AT_CSIListenerIface {
private:
  AbonentsDB &  _abnDb;
  FILE *        _outStream;

public:
  ATIHClient(AbonentsDB & use_db, FILE * out_stream)
  : _abnDb(use_db), _outStream(out_stream)
  { }
  ~ATIHClient()
  { }
  // ---------------------------------
  // -- AT_CSIListenerIface interface
  // ---------------------------------
  virtual void onCSIresult(const AbonentInfo & ab_info)
  {
    unsigned ab_id = _abnDb.searchAbn(ab_info.msIsdn);
    if (ab_id) {
      _abnDb.setAbnInfo(ab_id, ab_info);
      _abnDb.printAbnInfo(_outStream, ab_id);
    } else
      fprintf(_outStream, "ERR: CSI result for unregistered subscriber %s",
              ab_info.msIsdn.toString().c_str());
  }
  virtual void onCSIabort(const TonNpiAddress & subcr_addr, RCHash ercode)
  {
    fprintf(_outStream, "ERR: CSI request for subscriber %s failed: code %u, %s\n",
            subcr_addr.toString().c_str(), ercode, URCRegistry::explainHash(ercode).c_str());
  }
};


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
static std::string _listOfCSI;

static void init_listOfCSI(void)
{
  for (unsigned i = UnifiedCSI::csi_O_BC; i < UnifiedCSI::csi_VT_IM; ++i)
    format(_listOfCSI, "%s%s", i ? ", " : "", UnifiedCSI::nmCSI(UnifiedCSI::get(i)));
}

//Retuns index in _nmCSI[] array or -1 if unknown
static CSIUid_e isKnownCSI(const char * nm_csi)
{
  for (unsigned i = 0; i < UnifiedCSI::csi_VT_IM; ++i) {
    if (!strcmp(nm_csi, UnifiedCSI::nmCSI(UnifiedCSI::get(i)))) {
      return UnifiedCSI::get(i);
    }
  }
  return UnifiedCSI::csi_UNDEFINED;
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
    fprintf(stdout, "  CSI:         yes (%s)\n", reqCfg.nmCSI());
  else
    fprintf(stdout, "  CSI:         no\n");

  if (reqCfg.getFlags().xcsi)
    fprintf(stdout, "  xCSI:        yes (%s)\n", reqCfg.nmCSI());
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
                                  "       csi_id_str is one of:\n       %s\n";
void utl_switch_csi(const std::vector<std::string> &args, bool is_mark)
{
  if (args.size() < 2) {
    fprintf(stdout, hlp_switch_csi, is_mark ? "" : "un", _listOfCSI.c_str());
    return;
  }

  CSIUid_e idCsi = isKnownCSI(args[1].c_str());
  if (idCsi == UnifiedCSI::csi_UNDEFINED) {
    fprintf(stdout, "ERROR: unknwon CSI id (%s)\n", args[1].c_str());
    fprintf(stdout, hlp_switch_csi, is_mark ? "" : "un", _listOfCSI.c_str());
    return;
  }
  if (is_mark)
    g_pService->getRequestCfg().setCSI(idCsi);
  else
    g_pService->getRequestCfg().unsetCSI(idCsi);
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
                                  "       flag_id_str is one of:\n       %s\n";

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
    fputs(hlp_set_abn, stdout);
    _abonentsData->printAbonents(stdout, 0, 0);
    return;
  }
  TonNpiAddress   abIsdn;
  IMSIString      abImsi;

  if (args.size() >= 2) { //msisdn
    if (!abIsdn.fromText(args[1].c_str()) || !abIsdn.interISDN()) {
      fprintf(stdout, "ERR: bad MSISDN \'%s\'", args[1].c_str());
      fputs(hlp_set_abn, stdout);
      return;
    }
  }
  if (args.size() >= 3) { //imsi: (isdn, unknown)
    if (!abImsi.fromText(args[2].c_str())) {
      fprintf(stdout, "ERR: bad IMSI \'%s\'", args[2].c_str());
      fputs(hlp_set_abn, stdout);
      return;
    }
  }
  _abonentsData->setAbonent(abIsdn, AbonentContractInfo::abtUnknown,
                           abImsi.empty() ? NULL : abImsi.c_str());
}

/* ------------------------------------------------------------------- */
//requests SubscriptionInfo according to RequestedSubscription settings
static const char hlp_get_csi[] = "USAGE: get_csi [abn.NN | '.1.1.signals']\n";
void cmd_get_csi(Console&, const std::vector<std::string> &args)
{
  if (args.size() < 2) {
    fputs(hlp_get_csi, stdout);
    return;
  }
  TonNpiAddress sbscrIsdn;
  unsigned      abId = 0;

  const char * s_abn = args[1].c_str();
  if (!strncmp(s_abn, "abn.", 4)) {       //abn.NN
    abId = (unsigned)atoi(s_abn + 4);
    if (!abId || (abId > _abonentsData->getMaxAbId())) {
      fprintf(stdout, "ERR: %s abonent Id %s\n", !abId ? "bad" : "unknown", s_abn);
      fputs(hlp_get_csi, stdout);
      return;
    }
    AbonentInfo * abInfo = _abonentsData->getAbnInfo(abId); //cann't be NULL here!
    sbscrIsdn = abInfo->msIsdn;
    /* */
  } else if ((s_abn[0] == '+') || !strncmp(s_abn, ".1.1.", 5)) { //msisdn
    if (!sbscrIsdn.fromText(s_abn)) {
      fprintf(stdout, "ERR:  bad MSISDN \'%s\'\n", s_abn);
      fputs(hlp_get_csi, stdout);
      return;
    }
    abId = _abonentsData->setAbonent(sbscrIsdn);
    /* */
  } else {
    fprintf(stdout, "ERR:  bad MSISDN \'%s\'\n", s_abn);
    fputs(hlp_get_csi, stdout);
    return;
  }
  fprintf(stdout, "[get_csi] abn.%u: %s\n",  abId, sbscrIsdn.toString().c_str());
  if (!g_pService->requestCSI(sbscrIsdn))
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
  _abonentsData = AbonentsDB::getInstance();
  _abonentsData->Init((unsigned)PRE_ABONENTS_NUM, _abonents);

  ATIHClient  atihClient(*_abonentsData, stdout);
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

  } catch (const std::exception & error) {
    smsc_log_fatal(rootLogger, "%s: %s", _nmTst, error.what());
    fprintf(stdout, "%s: Fatal error: %s\n", _nmTst, error.what());
    rval = 1;
  }
  if (g_pService)
    delete g_pService;
  smsc_log_info(rootLogger, "%s TEST shutdown complete", _nmTst);
  return rval;
}

