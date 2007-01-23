static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>
#include <string.h>
#include <string>

#include "inman/comp/map_chsri/MapCHSRIFactory.hpp"
using smsc::inman::comp::chsri::initMAPCHSRI3Components;

#include "util/config/ConfigView.h"
using smsc::util::config::ConfigView;
using smsc::util::config::CStrSet;

#include "chsri_srv.hpp"
using smsc::inman::SRI_CSIListener;

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

using smsc::inman::ServiceCHSRI;
using smsc::inman::ServiceCHSRI_CFG;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;

static const unsigned short _in_CFG_DFLT_CAP_TIMEOUT = 20;
static const unsigned char _in_CFG_DFLT_SS7_USER_ID = 3; //USER03_ID

static char     _runService = 0;
static ServiceCHSRI* g_pService = 0;


extern "C" static void sighandler(int signal)
{
    _runService = 0;
}

struct AbonentInfo {
    TonNpiAddress  addr;
    char           imsi[MAP_MAX_IMSI_AddressValueLength];
    GsmSCFinfo     scf;

    AbonentInfo()   { imsi[0] = 0; scf.serviceKey = 0; }
};

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
                abn.imsi[0] ? abn.imsi : "none",
                abn.scf.scfAddress.length ? abn.scf.scfAddress.getSignals() : "none",
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
            else
                abn.addr.clear();
            if (p_abn[i].imsi[0])
                strcpy(abn.imsi, p_abn[i].imsi);
            else
                abn.imsi[0] = 0;

            registry.insert(AbonentsMAP::value_type(++lastAbnId, abn));
        }
    }

    unsigned getMaxAbId(void) const { return lastAbnId; }

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

    const AbonentInfo * getAbnInfo(unsigned ab_id)
    {
        MutexGuard  grd(_sync);
        AbonentsMAP::const_iterator it = registry.find(ab_id);
        return (it != registry.end()) ? &((*it).second) : NULL;
    }

    bool setAbnInfo(unsigned ab_id, const GsmSCFinfo * p_scf = NULL, const char * p_imsi = NULL)
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


class CHSRIClient: public SRI_CSIListener {
private:
    FILE * outStream;

public:
    CHSRIClient(FILE * out_stream)
        : outStream(out_stream)
    { }
    ~CHSRIClient()
    { }
    //-- SRI_CSIListener interface
    void onCSIresult(const std::string &subcr_addr,
                     const char * subcr_imsi, const GsmSCFinfo* scfInfo)
    {
        unsigned ab_id = abnData.searchAbn(subcr_addr);
        if (ab_id) {
            abnData.setAbnInfo(ab_id, scfInfo, subcr_imsi);
            fprintf(outStream, "\n");
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

#define IS_ISDN(addr) ( ((addr).numPlanInd == 1) && ((addr).typeOfNumber <= 1) )
//#define NOT_IMSI(addr) ((addr).typeOfNumber || ((addr).numPlanInd != 1))
/* ************************************************************************** *
 * Console commands:
 * ************************************************************************** */
void cmd_config(Console&, const std::vector<std::string> &args)
{
    abnData.printAbnInfo(stdout, 0, 0);
}


//> set_abn ['.1.1.signals'] ['imsi_number']
static const char hlp_set_abn[] = "USAGE: set_abn ['.1.1.signals' | 'isdn_number']\n";
void cmd_set_abn(Console&, const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        fprintf(stdout, hlp_set_abn);
        return;
    }
    
    TonNpiAddress   adr;
    if (!adr.fromText(args[1].c_str()) || !IS_ISDN(adr)) {
        fprintf(stdout, "ERR: bad MSISDN");
        fprintf(stdout, hlp_set_abn);
        return;
    }
    abnData.addAbonent(&adr, NULL);
}

//requests SCF info via O_CSI
//> o_csi address
// where addres either 'abn.N' or '.1.1.signals' or 'isdn_number'
static const char hlp_o_csi[] = "USAGE: o_csi [abn.NN | '.1.1.signals' | 'isdn_number']\n";
void cmd_o_csi(Console&, const std::vector<std::string> &args)
{
    if (args.size() < 2) {
        fprintf(stdout, hlp_o_csi);
        return;
    }
    std::string subscr;
    unsigned abId = 0;

    const char * s_abn = args[1].c_str();
    if (!strncmp(s_abn, "abn.", 4)) {       //abonent id
        abId = (unsigned)atoi(s_abn + 4);
        if (!abId || abId > abnData.getMaxAbId()) {
            fprintf(stdout, "ERR:  %s abonent Id %s\n", !abId ? "bad" : "unknown", s_abn);
            fprintf(stdout, hlp_o_csi);
            return;
        }
        AbonentInfo ab_inf = *abnData.getAbnInfo(abId);
        subscr = ab_inf.addr.toString();
    } else if (!strncmp(s_abn, ".1.1.", 5)) { //msisdn
        TonNpiAddress addr;
        if (!addr.fromText(s_abn)) {
            fprintf(stdout, "ERR:  bad MSISDN %s\n", s_abn);
            fprintf(stdout, hlp_o_csi);
            return;
        }
        abId = abnData.addAbonent(&addr, NULL);
        subscr = addr.toString();
    } else {                                //isdn unknown
        TonNpiAddress addr;
        if (!addr.fromText(s_abn) || !IS_ISDN(addr)) {
            fprintf(stdout, "ERR:  bad MSISDN: %s\n", s_abn);
            fprintf(stdout, hlp_o_csi);
            return;
        }
        abId = abnData.addAbonent(&addr, NULL);
        subscr = addr.toString();
    }
    fprintf(stdout, "[o_csi] abn.%u: %s\n",  abId, subscr.c_str());
    if (!g_pService->requestCSI(subscr))
        fprintf(stdout, "[o_csi] failed to request CSI");
}


struct CHSRITestConfig : public ServiceCHSRI_CFG {
public:
    CHSRITestConfig()
    {
        client = NULL;
        hlr.scf_addr = 0;
        hlr.scf_ssn = hlr.hlr_ssn = 0;
        //OPTIONAL PARAMETERS:
        hlr.userId = _in_CFG_DFLT_SS7_USER_ID;
        hlr.mapTimeout = _in_CFG_DFLT_CAP_TIMEOUT;
    }
    ~CHSRITestConfig()
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

    smsc_log_info(inapLogger,"***************************");
    smsc_log_info(inapLogger,"* SIBINCO MAP CH-SRI TEST *");
    smsc_log_info(inapLogger,"***************************");
    if (argc > 1)
        cfgFile = argv[1];
    smsc_log_info(inapLogger,"* Config file: %s", cfgFile);
    smsc_log_info(inapLogger,"******************************");

    CHSRITestConfig cfg;
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
        ApplicationContextFactory::Init(ACOID::id_ac_map_locInfoRetrieval_v3,
                                        initMAPCHSRI3Components)/*;*/
    );

    abnData.init(PRE_ABONENTS_NUM, _abonents);
    CHSRIClient  sriClient(stdout);
    cfg.client = &sriClient;
    try {
        g_pService = new ServiceCHSRI(&cfg, inapLogger);
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
            smsc_log_fatal(inapLogger, "SRISrv: startup failure. Exiting.");
            _runService = 0;
        }
        if (_runService)
            console.run("sri>");

    } catch(const std::exception& error) {
        smsc_log_fatal(inapLogger, "%s", error.what() );
        fprintf( stderr, "Fatal error: %s\n", error.what() );
        rval = 1;
    }
    if (g_pService)
        delete g_pService;
    smsc_log_info(inapLogger, "CH-SRI TEST shutdown complete");
    return rval;
}

