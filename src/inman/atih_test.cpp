static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>
#include <string.h>
#include <string>


#include "inman/comp/map_atih/MapATIHFactory.hpp"
using smsc::inman::comp::atih::initMAPATIH3Components;

#include "util/config/ConfigView.h"
using smsc::util::config::ConfigView;
using smsc::util::config::CStrSet;

#include "atih_srv.hpp"
using smsc::inman::ATCSIListener;

#include "inman/common/console.hpp"
using smsc::inman::common::Console;


namespace smsc {
  namespace inman {
    namespace inap {
        Logger* inapLogger;
        extern Logger * _EINSS7_logger_DFLT;
    } //inap
  }
};
using smsc::inman::inap::inapLogger;
using smsc::inman::inap::_EINSS7_logger_DFLT;

using smsc::inman::ServiceATIH;
using smsc::inman::ServiceATIH_CFG;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;

static const unsigned short _in_CFG_DFLT_CAP_TIMEOUT = 20;
static const unsigned char _in_CFG_DFLT_SS7_USER_ID = 3; //USER03_ID

static char     _runService = 0;
static ServiceATIH* g_pService = 0;


extern "C" static void sighandler(int signal)
{
    _runService = 0;
}


typedef struct {
    TonNpiAddress  addr;
    TonNpiAddress  imsi;
    MAPSCFinfo     scf;
} AbonentInfo;

typedef struct {
    const char *  addr;
    const char *  imsi;
} Abonent;

static const Abonent  _abonents[] = {
    //Nezhinsky phone(prepaid):
     { ".1.1.79139343290", "250013900405871" }
    //Ryzhkov phone(postpaid):
    ,{ ".1.1.79139859489", "250013901464251" }
    //Stupnik phone(postpaid):
    ,{ ".1.1.79139033669", "250013901464251" }
};
#define PRE_ABONENTS_NUM (sizeof(_abonents)/sizeof(Abonent))

class AbonentsInfoMAP {
protected:
    typedef std::map<unsigned, AbonentInfo> AbonentsMAP;

    Mutex           _sync;
    AbonentsMAP     registry;
    unsigned        lastAbnId;

public:
    static void printAbnInfo(FILE * stream, const AbonentInfo & abn, unsigned ab_id)
    {
        fprintf(stream, "abn.%u: isdn <%s>, imsi <%s>\n\tSCF <%s>, servKey %u\n",
                ab_id, abn.addr.length ? abn.addr.toString().c_str() : " ",
                abn.imsi.length ? abn.imsi.getSignals() : " ",
                abn.scf.scfAddress.length ? abn.scf.scfAddress.getSignals() : " ",
                abn.scf.serviceKey);
    }

    AbonentsInfoMAP(void) : lastAbnId(0)
    { }
    ~AbonentsInfoMAP()
    { }

    void init(unsigned n_abn, const Abonent * p_abn) {
        for (unsigned i = 0; i < n_abn; i++) {
            AbonentInfo  abn;
            if (p_abn[i].addr)
                abn.addr.fromText(p_abn[i].addr);
            if (p_abn[i].imsi)
                abn.imsi.fromText(p_abn[i].imsi);

            registry.insert(AbonentsMAP::value_type(++lastAbnId, abn));
        }
    }

    unsigned getMaxAbId(void) const { return lastAbnId; }

    unsigned addAbonent(TonNpiAddress * p_isdn, TonNpiAddress * p_imsi)
    {
        MutexGuard  grd(_sync);
        AbonentInfo  abn;
        if (p_isdn && p_isdn->length)
            abn.addr = *p_isdn;
        if (p_imsi && p_imsi->length)
            abn.imsi = *p_imsi;
        registry.insert(AbonentsMAP::value_type(++lastAbnId, abn));
        return lastAbnId;
    }

    const AbonentInfo * getAbnInfo(unsigned ab_id)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::const_iterator it = registry.find(ab_id);
        return (it != registry.end()) ? &((*it).second) : NULL;
    }

    bool setAbnInfo(unsigned ab_id, const MAPSCFinfo * abn_info)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::iterator it = registry.find(ab_id);
        if (it != registry.end()) {
            AbonentInfo & abn = (*it).second;
            abn.scf = *abn_info;
            return true;
        }
        return false;
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
            if (abn.imsi.length) {
                if (abn.imsi == subscr)
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
            AbonentsInfoMAP::printAbnInfo(stream, abn, ab_id);
        }
    }

    //0, 0 - print ALL
    void printAbnInfo(FILE * stream, unsigned min_id, unsigned max_id)
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
};
//GLOBAL:
AbonentsInfoMAP     abnData;


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
    void onCSIresult(const std::string &subcr_addr, const MAPSCFinfo* scfInfo)
    {
        unsigned ab_id = abnData.searchAbn(subcr_addr);
        if (ab_id) {
            abnData.setAbnInfo(ab_id, scfInfo);
            abnData.printAbnInfo(outStream, ab_id);
        } else
            fprintf(outStream, "ERR: CSI result for unregistered subscriber %s",
                    subcr_addr.c_str());
        return;
    }
    void onCSIabort(const std::string &subcr_addr, unsigned short ercode, InmanErrorType errLayer)
    {
        fprintf(outStream, "ERR: CSI request for subscriber %s failed: code %u, layer %s\n",
                subcr_addr.c_str(), ercode, _InmanErrorSource[errLayer]);
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


//> set_abn ['.1.1.signals'] ['imsi_number']
void cmd_set_abn(Console&, const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        fprintf(stdout, "USAGE: set_abn ['.1.1.signals'] ['imsi_number']\n");
        return;
    }
    TonNpiAddress   adr[2]; //msisdn, imsi

    if (args.size() >= 2) { //msisdn
        if (!adr[0].fromText(args[1].c_str()) 
            || ((adr[0].numPlanInd != adr[0].typeOfNumber)
                || (adr[0].numPlanInd != 1))) {
            fprintf(stdout, "ERR: bad MSISDN");
            fprintf(stdout, "USAGE: set_abn ['.1.1.signals'] ['imsi_number']\n");
            return;
        }
    }
    if (args.size() >= 3) { //imsi: (isdn, unknown)
        if (!adr[1].fromText(args[2].c_str()) || NOT_IMSI(adr[1])) {
            fprintf(stdout, "ERR: bad IMSI");
            fprintf(stdout, "USAGE: set_abn ['.1.1.signals'] ['imsi_number']\n");
            return;
        }
    }
    abnData.addAbonent(&adr[0], &adr[1]);
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



struct ATIHTestConfig : public ServiceATIH_CFG {
public:
    ATIHTestConfig()
    {
        client = NULL;
        hlr.scf_addr = 0;
        hlr.scf_ssn = hlr.hlr_ssn = 0;
        //OPTIONAL PARAMETERS:
        hlr.userId = _in_CFG_DFLT_SS7_USER_ID;
        hlr.mapTimeout = _in_CFG_DFLT_CAP_TIMEOUT;
    }
    ~ATIHTestConfig()
    { }

    void read(Manager& manager) throw(ConfigException)
    {
        uint32_t tmo = 0;
        char *   cstr = NULL;
        std::string cppStr;

        /* *************************** *
         * HLR interaction parameters: *
         * *************************** */
        if (!manager.findSection("HLR_interaction"))
            throw ConfigException("\'HLR_interaction\' section is missed");

        ConfigView hlrCfg(manager, "HLR_interaction");
        try {
            hlr.scf_ssn = hlrCfg.getInt("scf_ssn");
            hlr.scf_addr = hlrCfg.getString("scf_address");
            smsc_log_info(inapLogger, "SCF : GT=%s, SSN=%u", hlr.scf_addr, hlr.scf_ssn);
        } catch (ConfigException& exc) {
            hlr.scf_addr = 0; hlr.scf_ssn = 0;
            throw ConfigException("SCF address or SSN missing");
        }
        try {
            hlr.hlr_ssn = hlrCfg.getInt("hlr_ssn");
            smsc_log_info(inapLogger, "HLR : SSN=%u", hlr.hlr_ssn);
        } catch (ConfigException& exc) {
            hlr.hlr_ssn = 0;
            throw ConfigException("HLR SSN missing");
        }
        /*  optional IN interaction parameters */

        tmo = 0;    //ss7UserId
        try { tmo = (uint32_t)hlrCfg.getInt("ss7UserId"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (!tmo || (tmo > 20))
                throw ConfigException("'ss7UserId' should fall into the range [1..20]");
            hlr.userId = (unsigned char)tmo;
        }
        smsc_log_info(inapLogger, "ss7UserId: %s%u", !tmo ? "default ":"", hlr.userId);

        tmo = 0;    //MapTimeout
        try { tmo = (uint32_t)hlrCfg.getInt("mapTimeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535)
                throw ConfigException("'mapTimeout' should be less than 65535 seconds");
            hlr.mapTimeout = (unsigned short)tmo;
        }
        smsc_log_info(inapLogger, "mapTimeout: %s%u secs", !tmo ? "default ":"", hlr.mapTimeout);
        /**/
        return;
    }
};


int main(int argc, char** argv)
{
    int     rval = 0;
    char *  cfgFile = (char*)"config.xml";

    tzset();
    Logger::Init();
    inapLogger = Logger::getInstance("smsc.inman");
    _EINSS7_logger_DFLT = Logger::getInstance("smsc.inman.inap");

    smsc_log_info(inapLogger,"*************************");
    smsc_log_info(inapLogger,"* SIBINCO MAP ATIH TEST *");
    smsc_log_info(inapLogger,"*************************");
    if (argc > 1)
        cfgFile = argv[1];
    smsc_log_info(inapLogger,"* Config file: %s", cfgFile);
    smsc_log_info(inapLogger,"******************************");

    ATIHTestConfig cfg;
    try {
        Manager::init((const char *)cfgFile);
        Manager& manager = Manager::getInstance();
        cfg.read(manager);
    } catch (ConfigException& exc) {
        smsc_log_error(inapLogger, "Config: %s", exc.what());
        smsc_log_error(inapLogger, "Configuration invalid. Exiting.");
        exit(-1);
    }
    assert(
        ApplicationContextFactory::Init(ACOID::id_ac_map_anyTimeInfoHandling_v3,
                                        initMAPATIH3Components)/*;*/
    );

    abnData.init(PRE_ABONENTS_NUM, _abonents);
    ATIHClient  atihClient(stdout);
    cfg.client = &atihClient;
    try {
        g_pService = new ServiceATIH(&cfg, inapLogger);
        assert(g_pService);

        Console console;
        console.addItem("o_csi", cmd_o_csi);
        console.addItem("config", cmd_config);
        console.addItem("set_abn", cmd_set_abn);

        _runService = 1;
        if (g_pService->start()) {
            //handle SIGTERM only in main thread
            sigset(SIGTERM, sighandler);
        } else {
            smsc_log_fatal(inapLogger, "ATIHSrv: startup failure. Exiting.");
            _runService = 0;
        }

        if (_runService)
            console.run("atih>");

//        g_pService->stop();
    } catch(const std::exception& error) {
        smsc_log_fatal(inapLogger, "%s", error.what() );
        fprintf( stderr, "Fatal error: %s\n", error.what() );
        rval = 1;
    }
    if (g_pService)
        delete g_pService;
    smsc_log_info(inapLogger, "ATIH TEST shutdown complete");
    return rval;
}

