/* ************************************************************************** *
 * Simple console application testing MAP Send Routing Info (Call Handling)
 * service of HLR.
 * ************************************************************************** */
#ifndef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/console.hpp"
using smsc::inman::common::Console;

#include "inman/utilbin/URCInitMAP.hpp"
#include "inman/utilbin/chsri_srv.hpp"
using smsc::util::RCHash;
using smsc::util::URCRegistry;
using smsc::util::format;
using smsc::util::TonNpiAddress;

using smsc::inman::ServiceCHSRI;
using smsc::inman::ServiceCHSRI_CFG;
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
static ServiceCHSRI* g_pService = 0;


extern "C" void sighandler(int signal)
{
    _runService = 0;
}

//GLOBAL:
smsc::inman::test::AbonentsDB * _abonentsData;

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


class CHSRIClient: public smsc::inman::SRI_CSIListenerIface {
private:
  AbonentsDB &  _abnDb;
  FILE *        _outStream;

public:
  explicit CHSRIClient(AbonentsDB & abn_db, FILE * out_stream)
  : _abnDb(abn_db), _outStream(out_stream)
  { }
  ~CHSRIClient()
  { }
  // ---------------------------------------
  // -- SRI_CSIListenerIface interface
  // ---------------------------------------
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
  void onCSIabort(const TonNpiAddress & subcr_addr, RCHash ercode)
  {
    fprintf(_outStream, "ERR: CSI request for subscriber %s failed: code %u, %s\n",
            subcr_addr.toString().c_str(), ercode, URCRegistry::explainHash(ercode).c_str());
  }
};

/* ************************************************************************** *
 * Console commands:
 * ************************************************************************** */
void cmd_config(Console&, const std::vector<std::string> &args)
{
  _abonentsData->printAbonents(stdout, 0, 0);
}


//> set_abn ['.1.1.signals' | 'isdn_number'] ['imsi_number']
static const char hlp_set_abn[] = "USAGE: set_abn ['.1.1.signals' | 'isdn_number'] ['imsi_number']\n";
void cmd_set_abn(Console&, const std::vector<std::string> &args)
{
  if (args.size() < 2) {
    fputs(hlp_set_abn, stdout);
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

//requests SCF info via O_CSI
//> o_csi address
// where addres either 'abn.N' or '.1.1.signals' or 'isdn_number'
static const char hlp_get_csi[] = "USAGE: get_csi [abn.NN | '.1.1.signals' | 'isdn_number']\n";
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
    AbonentInfo * abInfo = _abonentsData->getAbnInfo(abId);  //cann't be NULL here!
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
  } else {                                //isdn unknown
    fprintf(stdout, "ERR:  bad MSISDN \'%s\'\n", s_abn);
    fputs(hlp_get_csi, stdout);
    return;
  }
  fprintf(stdout, "[get_csi] abn.%u: %s\n",  abId, sbscrIsdn.toString().c_str());
  if (!g_pService->requestCSI(sbscrIsdn))
    fprintf(stdout, "[get_csi] failed to request CSI");
}

/* ---------------------------------------------------------- */
static const char * const _nmTst = "CH-SRI";
static const char * const _nmLogger = "map.chsri";

int main(int argc, char** argv)
{
  int     rval = 0;
  const char *  cfgFile = "config.maptst.xml";

  tzset();
  URCRegistryInit4MAP();
  Logger::Init();
  Logger * rootLogger = Logger::getInstance(_nmLogger);

  smsc_log_info(rootLogger,"***************************");
  smsc_log_info(rootLogger,"* Eyeline MAP %s TEST *", _nmTst);
  smsc_log_info(rootLogger,"***************************");
  if (argc > 1)
    cfgFile = argv[1];
  smsc_log_info(rootLogger,"* Config file: %s", cfgFile);
  smsc_log_info(rootLogger,"******************************");

  ServiceCHSRI_CFG  tstCfg;
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

  CHSRIClient  sriClient(*_abonentsData, stdout);
  tstCfg.client = &sriClient;

  try {
    g_pService = new ServiceCHSRI(tstCfg, rootLogger);
    Console console;
    console.addItem("get_csi", cmd_get_csi);
    console.addItem("config", cmd_config);
    console.addItem("set_abn", cmd_set_abn);

    _runService = 1;
    if (g_pService->start()) {
      //handle SIGTERM only in main thread
      sigset(SIGTERM, sighandler);
    } else {
      smsc_log_fatal(rootLogger, "%s: SRISrv startup failure. Exiting.", _nmTst);
      _runService = 0;
      rval = 1;
    }
    if (_runService)
      console.run("SRISrv>");
    g_pService->stop();

  } catch (const std::exception& error) {
    smsc_log_fatal(rootLogger, "%s: %s", _nmTst, error.what());
    fprintf(stdout, "%s: Fatal error: %s\n", _nmTst, error.what());
    rval = 1;
  }
  if (g_pService)
    delete g_pService;
  smsc_log_info(rootLogger, "%s TEST shutdown complete", _nmTst);
  return rval;
}

