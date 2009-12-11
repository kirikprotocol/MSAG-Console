/* ************************************************************************** *
 * Simple console application testing MAP Any Time Subscription Interrogation
 * service of HLR.
 * ************************************************************************** */
#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

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

#include "inman/common/XCFManager.hpp"
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
/* ************************************************************************** *
 * Console commands:
 * ************************************************************************** */
void cmd_config(Console&, const std::vector<std::string> &args)
{
    abnData.printAbnInfo(stdout, 0, 0);
}


//> set_abn ['.1.1.signals' | 'isdn_number'] ['imsi_number']
static const char hlp_set_abn[] = "USAGE: set_abn ['.1.1.signals' | 'isdn_number'] ['imsi_number']\n";
void cmd_set_abn(Console&, const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        fprintf(stdout, hlp_set_abn);
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

//requests SCF info via O_CSI
//> o_csi address
// where addres either 'abn.N' or '.1.1.signals' or 'imsi_number'
void cmd_o_csi(Console&, const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        fprintf(stdout, "USAGE: o_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
        return;
    }
    bool        imsi = false;
    std::string subscr;
    unsigned abId = 0;

    const char * s_abn = args[1].c_str();
    if (!strncmp(s_abn, "abn.", 4)) {       //abonent id
        abId = (unsigned)atoi(s_abn + 4);
        if (!abId || abId > abnData.getMaxAbId()) {
            fprintf(stdout, "ERR:  %s abonent Id %s\n", !abId ? "bad" : "unknown", s_abn);
            fprintf(stdout, "USAGE: o_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
            return;
        }
        AbonentInfo ab_inf = *abnData.getAbnInfo(abId);
        subscr = ab_inf.addr.toString();
    } else if (!strncmp(s_abn, ".1.1.", 5)) { //msisdn
        TonNpiAddress addr;
        if (!addr.fromText(s_abn)) {
            fprintf(stdout, "ERR:  bad MSISDN %s\n", s_abn);
            fprintf(stdout, "USAGE: o_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
            return;
        }
        abId = abnData.addAbonent(&addr, NULL);
        subscr = addr.toString();
    } else {                                //imsi or unknown
        TonNpiAddress addr;
        if (!addr.fromText(s_abn)) {
            fprintf(stdout, "ERR:  bad IMSI or MSISDN: %s\n", s_abn);
            fprintf(stdout, "USAGE: o_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
            return;
        }
        if (NOT_IMSI(addr)) {
            fprintf(stdout, "ERR:  bad IMSI or MSISDN %s\n", s_abn);
            fprintf(stdout, "USAGE: o_csi [abn.NN | '.1.1.signals' | 'imsi_number']\n");
            return;
        }
        fprintf(stdout, "[o_csi]: %s\n", addr.getSignals());
        fprintf(stdout, "ERR:  IMSI is  not supported yet.\n");
        return;
/*
        abId = abnData.addAbonent(NULL, &addr);
        subscr = addr.toString();
        imsi = true;
*/
    }
    fprintf(stdout, "[o_csi] abn.%u: %s, imsi = %s\n",  abId, subscr.c_str(),
            imsi ? "true" : "false");
    if (!g_pService->requestCSI(subscr, imsi))
        fprintf(stdout, "[o_csi] failed to request CSI");
}

static const char * const _nmTst = "ATIH";
int main(int argc, char** argv)
{
    int     rval = 0;
    const char *  cfgFile = "config.maptst.xml";

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
        console.addItem("o_csi", cmd_o_csi);
        console.addItem("config", cmd_config);
        console.addItem("set_abn", cmd_set_abn);

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

