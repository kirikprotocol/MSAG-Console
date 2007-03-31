static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>
#include <string.h>
#include <string>

#include "version.hpp"
#include "inman.hpp"

#include "inman/comp/cap_sms/CapSMSFactory.hpp"
using smsc::inman::comp::initCAP3SMSComponents;

#include "util/config/ConfigView.h"
#include "util/mirrorfile/mirrorfile.h"
using smsc::util::config::ConfigView;
using smsc::util::config::CStrSet;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;

#include "inman/abprov/IAPLoader.hpp"
using smsc::inman::iaprvd::IAProviderLoader;
using smsc::inman::iaprvd::IAProviderCreatorITF;

#include "service.hpp"
using smsc::inman::Service;

namespace smsc {
  namespace inman {
    namespace inap {
        Logger* inmanLogger;
        extern Logger * _EINSS7_logger_DFLT;
    } //inap
  }
};
using smsc::inman::inap::inmanLogger;
using smsc::inman::inap::_EINSS7_logger_DFLT;

static char     _runService = 0;
static Service* g_pService = 0;


extern "C" static void sighandler(int signal)
{
    _runService = 0;
}

namespace smsc {
  namespace inman {

static const unsigned int _in_CFG_DFLT_CLIENT_CONNS = 3;
static const long _in_CFG_MIN_BILLING_INTERVAL = 10; //in seconds
static const unsigned int _in_CFG_MAX_BILLINGS = 100000;
static const unsigned int _in_CFG_DFLT_BILLINGS = 500;
static const unsigned short _in_CFG_DFLT_CAP_TIMEOUT = 20;
static const unsigned short _in_CFG_DFLT_BILL_TIMEOUT = 120;
static const unsigned short _in_CFG_DFLT_ABTYPE_TIMEOUT = 5;
static const unsigned char _in_CFG_DFLT_SS7_USER_ID = 3; //USER03_ID
static const unsigned short _in_CFG_DFLT_TCAP_DIALOGS = 2000;
static const long _in_CFG_DFLT_CACHE_INTERVAL = 1440;
static const int  _in_CFG_DFLT_CACHE_RECORDS = 10000;

//CDR_NONE = 0, CDR_ALL = 1, CDR_POSTPAID = 2
static const char * const _CDRmodes[3] = {"none", "all", "postpaid"};
//BILL_ALL = 0, BILL_USSD, BILL_SMS
static const char * const _BILLmodes[4] = {"none", "all", "ussd", "sms"};

//according to IAProviderType
static const char * const _IAPTypes[] = {"CACHE", "IN", "HLR", "DB"};
//according to IAProviderAbility_e
static const char * const _IAPAbilities[] = { "none", "abContract", "abSCF", "abContractSCF" };
// according to INScfCFG::IDPLocationAddr
static const char * const _IDPLIAddr[] = { "MSC", "SMSC", "SSF" };


class INScfsRegistry {
protected:
    INScfsMAP  scfMap;

public:
    INScfsRegistry()
    { }
    ~INScfsRegistry()
    {
        for (INScfsMAP::iterator sit = scfMap.begin(); sit != scfMap.end(); sit++)
            delete (*sit).second;
    }

    inline void copyAll(INScfsMAP & cp_map)
    {
        cp_map.insert(scfMap.begin(), scfMap.end());
    }

    //NOTE: the 'nm_scf' subsection presence must be previously checked !!!
    INScfCFG * readSCF(ConfigView * scf_sec, const char * nm_scf) throw(ConfigException)
    {
        std::auto_ptr<ConfigView> scfCfg(scf_sec->getSubConfig(nm_scf));
        std::auto_ptr<INScfCFG> pin(new INScfCFG(nm_scf));
        smsc_log_info(inmanLogger, "IN-platform '%s' config ..", pin->ident());

        char * cstr = NULL;
        cstr = scfCfg->getString("scfAddress");
        if (!cstr[0])
            throw ConfigException("SCF address missing");

        if (!pin->scf.scfAddress.fromText(cstr)
            || (pin->scf.scfAddress.numPlanInd != NUMBERING_ISDN)
            || (pin->scf.scfAddress.typeOfNumber > ToN_INTERNATIONAL))
            throw ConfigException("SCF address is invalid: %s", cstr);
        pin->scf.scfAddress.typeOfNumber = ToN_INTERNATIONAL; //correct isdn unknown

        INScfsMAP::const_iterator sit = scfMap.find(pin->scf.scfAddress.toString());
        if (sit != scfMap.end()) {
            smsc_log_info(inmanLogger, "IN-platform '%s' known ..", pin->ident());
            return (*sit).second;
        }

        pin->scf.serviceKey = (uint32_t)scfCfg->getInt("serviceKey");
        smsc_log_info(inmanLogger, "SCF: %s:{%u}",
                        pin->scf.scfAddress.toString().c_str(), pin->scf.serviceKey);

        //optional parameters:
        cstr = NULL; 
        std::string cppStr = "RPCList_reject: ";
        try { cstr = scfCfg->getString("RPCList_reject"); }
        catch (ConfigException& exc) { }
        if (cstr && cstr[0]) {
            try { pin->rejectRPC.init(cstr); }
            catch (std::exception& exc) {
                throw ConfigException("RPCList_reject: %s", exc.what());
            }
        }
        if ((pin->rejectRPC.size() <= 1) || !pin->rejectRPC.print(cppStr))
            cppStr += "unsupported";
        smsc_log_info(inmanLogger, cppStr.c_str());

        cstr = NULL; cppStr = "RPCList_postpaid: ";
        try { cstr = scfCfg->getString("RPCList_postpaid"); }
        catch (ConfigException& exc) { }
        if (cstr) {
            try { pin->postpaidRPC.init(cstr); }
            catch (std::exception& exc) {
                throw ConfigException("RPCList_postpaid: %s", exc.what());
            }
        }
        if (!pin->postpaidRPC.print(cppStr))
            cppStr += "unsupported";
        smsc_log_info(inmanLogger, cppStr.c_str());

        cstr = NULL; cppStr = "IDPLocationInfo: ";
        try { cstr = scfCfg->getString("IDPLocationInfo");}
        catch (ConfigException& exc) { }

        if (cstr && cstr[0]) {
            if (!strcmp(cstr, _IDPLIAddr[INScfCFG::idpLiSSF]))
                pin->idpLiAddr = INScfCFG::idpLiSSF;
            else if (!strcmp(cstr, _IDPLIAddr[INScfCFG::idpLiSMSC]))
                pin->idpLiAddr = INScfCFG::idpLiSMSC;
            else if (strcmp(cstr, _IDPLIAddr[INScfCFG::idpLiMSC]))
                throw ConfigException("IDPLocationInfo: invalid value");
        } else
            cstr = (char*)_IDPLIAddr[INScfCFG::idpLiMSC];
        smsc_log_info(inmanLogger, "IDPLocationInfo: %s", cstr);

        scfMap.insert(INScfsMAP::value_type(pin->scf.scfAddress.toString(), pin.get()));
        return pin.release();
    }

    unsigned readSCFs(ConfigView * scf_sec) throw(ConfigException)
    {
        std::auto_ptr<CStrSet> subs(scf_sec->getShortSectionNames());
        CStrSet::iterator sit = subs->begin();
        for (; sit != subs->end(); sit++)
            readSCF(scf_sec, sit->c_str());
        return scfMap.size();
    }
};

class INManConfig : public InService_CFG {
protected:
    typedef std::list<const char *> CPTRList;
    struct AbonentPolicyXCFG {
    protected:
        char *    text;
    public:
        const char *    ident;
        const char *    prvdNm;
        CPTRList  scfNms;
       
        AbonentPolicyXCFG(const char * nm_pol)
            : ident(nm_pol), text(NULL), prvdNm(NULL)
        { }
        ~AbonentPolicyXCFG() { if (text) delete [] text; }

        bool init(const char * str)
        {
            if (!str || !str[0])
                return false;

            size_t tSz = strlen(str);
            text = new char[tSz + 1];
            strcpy(text, str);

            char *pos = text,  *commaPos = 0;
            //cut provider name
            if ((commaPos = strchr(pos, ',')) != 0) {
                *commaPos = 0;
                while(isspace(*pos) && (pos < commaPos))
                    pos++;
                prvdNm = (pos != commaPos) ? pos : NULL;
                pos = commaPos + 1;
            }
            //cut SCF names
            while ((pos < (text + tSz)) && (commaPos = strchr(pos, ','))) {
                *commaPos = 0;
                while(isspace(*pos) && (pos < commaPos))
                    pos++;
                if (pos != commaPos)
                    scfNms.push_back(pos);
                pos = commaPos + 1;
            }
            while((pos < (text + tSz)) && isspace(*pos))
                pos++;
            if (*pos)
                scfNms.push_back(pos);

            if (!prvdNm && !scfNms.size())
                return false;
            return true;
        }

        void print(std::string & pstr) const
        {
            pstr += "Prvd: ";
            pstr += prvdNm ? prvdNm : "<none>";
            pstr += ", INs: ";
            if (scfNms.size()) {
                CPTRList::const_iterator it = scfNms.begin();
                pstr += *(it);
                for (it++; it != scfNms.end(); it++) {
                    pstr += ",  ";
                    pstr += *(it);
                }
            } else
                pstr += "<none>";
        }
    };

    INScfsRegistry      scfMap;
    std::string         smsXcfg; //SMSExtra services config file

public:
    INManConfig() : InService_CFG()
    {
        //OPTIONAL PARAMETERS:
        sock.maxConn = _in_CFG_DFLT_CLIENT_CONNS;
        bill.ss7.userId = _in_CFG_DFLT_SS7_USER_ID;
        bill.maxBilling = _in_CFG_DFLT_BILLINGS;
        bill.ss7.capTimeout = _in_CFG_DFLT_CAP_TIMEOUT;
        bill.ss7.maxDlgId = _in_CFG_DFLT_TCAP_DIALOGS;
        sock.timeout = bill.maxTimeout = _in_CFG_DFLT_BILL_TIMEOUT;
        bill.abtTimeout = _in_CFG_DFLT_ABTYPE_TIMEOUT;
        cachePrm.fileRcrd = _in_CFG_DFLT_CACHE_RECORDS;
    }

    ~INManConfig()
    { }

    const char * hasExtraConfig(void) const { return smsXcfg.length() ? smsXcfg.c_str() : NULL; }

    AbonentPolicy * readPolicyCFG(Manager & manager, const char * nm_pol) throw(ConfigException)
    {
        AbonentPolicyXCFG   polXCFG(nm_pol);

        if (!manager.findSection("AbonentPolicies"))
            throw ConfigException("\'AbonentPolicies\' section is missed");

        ConfigView  polSec(manager, "AbonentPolicies");
        if (!polSec.findSubSection(polXCFG.ident))
            throw ConfigException("\'%s\' policy is not specified", polXCFG.ident);

        std::auto_ptr<AbonentPolicy> policyCFG(new AbonentPolicy(nm_pol));

        std::auto_ptr<ConfigView> polCfg(polSec.getSubConfig(polXCFG.ident));
        //parse policy cfg
        smsc_log_info(inmanLogger, "Reading %s policy config ..", polXCFG.ident);

        char * cstr = NULL;
        cstr = polCfg->getString("policy");
        if (!cstr || !cstr[0])
            throw ConfigException("\'%s\' policy value is not specified", polXCFG.ident);
        if (!polXCFG.init(cstr))
            throw ConfigException("\'%s\' policy value is not valid: %s", polXCFG.ident, cstr);
        {
            std::string pstr;
            polXCFG.print(pstr);
            smsc_log_info(inmanLogger, "policy is: %s", pstr.c_str());
        }

        //lookup IN platforms configs
        if (polXCFG.scfNms.size()) {
            if (!manager.findSection("IN-platforms"))
                throw ConfigException("\'IN-platforms\' section is missed");
            ConfigView  scfSec(manager, "IN-platforms");

            if (!strcmp((char*)"*", *(polXCFG.scfNms.begin()))) { //include all SCFs defined
                if (!scfMap.readSCFs(&scfSec))
                    throw ConfigException("IN-platforms is not defined");
                scfMap.copyAll(policyCFG->scfMap);
            } else {
                for (CPTRList::iterator sit = polXCFG.scfNms.begin();
                                        sit != polXCFG.scfNms.end(); sit++) {
                    if (!scfSec.findSubSection(*sit))
                        throw ConfigException("IN-platform \'%s\'is not defined", *sit);
                    INScfCFG * pin = scfMap.readSCF(&scfSec, *sit);
                    policyCFG->scfMap.insert(
                        INScfsMAP::value_type(pin->scf.scfAddress.toString(), pin));
                }
            }
        }
        //lookup Abonent Provider config and load it
        if (polXCFG.prvdNm) {
            if (!manager.findSection("AbonentProviders"))
                throw ConfigException("\'AbonentProviders\' section is missed");
            ConfigView  prvdSec(manager, "AbonentProviders");
            if (!prvdSec.findSubSection(polXCFG.prvdNm))
                throw ConfigException("\'%s\' abonent provider is missed", polXCFG.prvdNm);

            std::auto_ptr<ConfigView> provCfg(prvdSec.getSubConfig(polXCFG.prvdNm));
            smsc_log_info(inmanLogger, "Loading AbonentProvider '%s'", polXCFG.prvdNm);
            policyCFG->provAllc = IAProviderLoader::LoadIAP(provCfg.get(), inmanLogger); //throws
            smsc_log_info(inmanLogger, "AbonentProvider '%s': type %s, ident: %s, ability: %s",
                          polXCFG.prvdNm, _IAPTypes[policyCFG->provAllc->type()],
                          policyCFG->provAllc->ident(), 
                          _IAPAbilities[policyCFG->provAllc->ability()]);
            policyCFG->provAllc->logConfig(inmanLogger);
        }

        return policyCFG.release();
    }

    void readSS7CFG(Manager & manager, SS7_CFG & ss7) throw(ConfigException)
    {
        if (!manager.findSection("SS7"))
            throw ConfigException("\'SS7\' section is missed");

        uint32_t tmo;
        char *   cstr;
        ConfigView ss7Cfg(manager, "SS7");

        cstr = NULL;
        try { cstr = ss7Cfg.getString("ssfAddress"); }
        catch (ConfigException& exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("SSF address is missing");
        if (!ss7.ssf_addr.fromText(cstr)
            || (ss7.ssf_addr.numPlanInd != NUMBERING_ISDN)
            || (ss7.ssf_addr.typeOfNumber > ToN_INTERNATIONAL))
            throw ConfigException("SSF address is invalid: %s", cstr);
        ss7.ssf_addr.typeOfNumber = ToN_INTERNATIONAL; //correct isdn unknown

        ss7.own_ssn = ss7Cfg.getInt("ssn"); //throws
        smsc_log_info(inmanLogger, "SSF: %u:%s", ss7.own_ssn,
                        ss7.ssf_addr.toString().c_str());

        /*  optional SS7 interaction parameters */
        tmo = 0;    //ss7UserId
        try { tmo = (uint32_t)ss7Cfg.getInt("ss7UserId"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (!tmo || (tmo > 20))
                throw ConfigException("'ss7UserId' should fall into the range [1..20]");
            ss7.userId = (unsigned char)tmo;
        }
        smsc_log_info(inmanLogger, "ss7UserId: %s%u", !tmo ? "default ":"", ss7.userId);

        tmo = 0;    //maxTimeout
        try { tmo = (uint32_t)ss7Cfg.getInt("maxTimeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535)
                throw ConfigException("'maxTimeout' should be less than 65535 seconds");
            ss7.capTimeout = (unsigned short)tmo;
        }
        smsc_log_info(inmanLogger, "maxTimeout: %s%u secs", !tmo ? "default ":"", ss7.capTimeout);

        tmo = 0;    //maxDialogs
        try { tmo = (uint32_t)ss7Cfg.getInt("maxDialogs"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if ((tmo >= 65530) || (tmo < 2))
                throw ConfigException("'maxDialogs' should fall into the range [2..65530]");
            ss7.maxDlgId = (unsigned short)tmo;
        }
        smsc_log_info(inmanLogger, "maxDialogs: %s%u", !tmo ? "default ":"", ss7.maxDlgId);
        return;
    }


    void read(Manager& manager) throw(ConfigException)
    {
        uint32_t tmo = 0;
        char *   cstr = NULL;
        char *   policyNm = NULL;
        std::string cppStr;

        /* ********************* *
         * InService parameters: *
         * ********************* */
        try { cstr = manager.getString("version");
        } catch (ConfigException& exc) { }
        if (!cstr || !cstr[0])
            smsc_log_warn(inmanLogger, "Config version is not set");
        else
            smsc_log_info(inmanLogger, "Config version: %s", cstr);

        try {
            cstr = manager.getString("host");
            sock.port = manager.getInt("port");
            smsc_log_info(inmanLogger, "INMan: %s:%d", cstr, sock.port);
        } catch (ConfigException& exc) {
            throw ConfigException("INMan host or port missing");
        }
        sock.host += cstr;

        tmo = 0;
        try { tmo = (uint32_t)manager.getInt("maxClients"); }
        catch (ConfigException& exc) { }
        if (tmo)
            sock.maxConn = (unsigned short)tmo;
        smsc_log_info(inmanLogger, "maxClients: %s%u", !tmo ? "default ":"", sock.maxConn);

        /* ******************** *
         * Billing parameters:  *
         * ******************** */
        if (!manager.findSection("Billing"))
            throw ConfigException("\'Billing\' section is missed");

        ConfigView billCfg(manager, "Billing");
        cstr = NULL;
        try { cstr = billCfg.getString("billMode");
        } catch (ConfigException& exc) {
            throw ConfigException("'billMode' is unknown or missing");
        }
        if (!strcmp(cstr, _BILLmodes[smsc::inman::BILL_SMS]))
            bill.billMode = smsc::inman::BILL_SMS;
        else if (!strcmp(cstr, _BILLmodes[smsc::inman::BILL_USSD]))
            bill.billMode = smsc::inman::BILL_USSD;
        else if (!strcmp(cstr, _BILLmodes[smsc::inman::BILL_NONE]))
            bill.billMode = smsc::inman::BILL_NONE;
        else if (strcmp(cstr, _BILLmodes[smsc::inman::BILL_ALL]))
            throw ConfigException("'billMode' is unknown or missing");
        smsc_log_info(inmanLogger, "billMode: %s [%d]", cstr, bill.billMode);

        cstr = NULL;
        try { policyNm = billCfg.getString("abonentPolicy");
        } catch (ConfigException& exc) {
            throw ConfigException("'abonentPolicy' is unknown or missing!");
        }
        if (!policyNm || !policyNm[0]) {
            policyNm = NULL;
            smsc_log_warn(inmanLogger, "Default abonent policy is not set!");
        }

        cstr = NULL;
        try { cstr = billCfg.getString("cdrMode");
        } catch (ConfigException& exc) {
            throw ConfigException("'cdrMode' is unknown or missing");
        }
        if (!strcmp(cstr, _CDRmodes[BillingCFG::CDR_POSTPAID]))
            bill.cdrMode = BillingCFG::CDR_POSTPAID;
        else if (!strcmp(cstr, _CDRmodes[BillingCFG::CDR_NONE]))
            bill.cdrMode = BillingCFG::CDR_NONE;
        else if (strcmp(cstr, _CDRmodes[BillingCFG::CDR_ALL]))
            throw ConfigException("'cdrMode' is unknown or missing");
        smsc_log_info(inmanLogger, "cdrMode: %s [%d]", cstr, bill.cdrMode);

        if (bill.cdrMode != BillingCFG::CDR_NONE) {
            cstr = NULL;
            try { cstr = billCfg.getString("cdrDir"); } 
            catch (ConfigException& exc) { }
            if (!cstr || !cstr[0])
                throw ConfigException("'cdrDir' is invalid or missing");

            try { bill.cdrInterval = billCfg.getInt("cdrInterval"); }
            catch (ConfigException& exc) {
                throw ConfigException("'cdrInterval' is invalid or missing");
            }
            if (bill.cdrInterval < _in_CFG_MIN_BILLING_INTERVAL) {
                throw ConfigException("'cdrInterval' should be grater than %ld seconds",
                                      _in_CFG_MIN_BILLING_INTERVAL);
            }
            bill.cdrDir += cstr;
            smsc_log_info(inmanLogger, "cdrDir: %s", cstr);
            smsc_log_info(inmanLogger, "cdrInterval: %d secs", bill.cdrInterval);
        }
        //cache parameters
        try {
            if (!(cachePrm.interval = (long)billCfg.getInt("cacheInterval")))
                cachePrm.interval = _in_CFG_DFLT_CACHE_INTERVAL;
            smsc_log_info(inmanLogger, "cacheInterval: %d minutes", cachePrm.interval);
            //convert minutes to seconds
            cachePrm.interval *= 60;
        } catch (ConfigException& exc) {
            throw ConfigException("'cacheInterval' is missing");
        }
        try {
            if (!(cachePrm.RAM = (long)billCfg.getInt("cacheRAM")))
                throw ConfigException("'cacheRAM' is missing or invalid");
            smsc_log_info(inmanLogger, "cacheRAM: %d Mb", cachePrm.RAM);
        } catch (ConfigException& exc) {
            throw ConfigException("'cacheRAM' is missing or invalid");
        }
        cstr = NULL;
        try { cstr = billCfg.getString("cacheDir"); }
        catch (ConfigException& exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("'cacheDir' is missing");
        cachePrm.nmDir += cstr;
        smsc_log_info(inmanLogger, "cacheDir: %s", cstr);

        tmo = 0;
        try { tmo = (uint32_t)billCfg.getInt("cacheRecords"); }
        catch (ConfigException& exc) { }
        if (tmo)
            cachePrm.fileRcrd = (int)tmo;
        smsc_log_info(inmanLogger, "cacheRecords: %s%d", !tmo ? "default ":"", cachePrm.fileRcrd);

        /*  optional Billing parameters */

        tmo = 0;    //maxBillings
        try { tmo = (uint32_t)billCfg.getInt("maxBillings"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo > _in_CFG_MAX_BILLINGS)
                throw ConfigException(format("'maxBillings' should be less than %u",
                                             _in_CFG_MAX_BILLINGS).c_str());
            bill.maxBilling = (unsigned short)tmo;
        }
        smsc_log_info(inmanLogger, "maxBillings: %s%u per connect", !tmo ? "default ":"", bill.maxBilling);

        tmo = 0;    //maxTimeout
        try { tmo = (uint32_t)billCfg.getInt("maxTimeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if ((tmo >= 65535) || (tmo < 5))
                throw ConfigException("'maxTimeout' should fall into the range [5 ..65535] seconds");
            sock.timeout = bill.maxTimeout = (unsigned short)tmo;
        }
        smsc_log_info(inmanLogger, "maxTimeout: %s%u secs", !tmo ? "default ":"", bill.maxTimeout);

        tmo = 0;    //abtTimeout
        try { tmo = (uint32_t)billCfg.getInt("abonentTypeTimeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535)
                throw ConfigException("'abonentTypeTimeout' should fall into the range [1 ..65535] seconds");
            bill.abtTimeout = (unsigned short)tmo;
        }
        smsc_log_info(inmanLogger, "abonentTypeTimeout: %s%u secs", !tmo ? "default ":"", bill.abtTimeout);

#ifdef SMSEXTRA
        /* ********************************* *
         * SMS Extra services configuration: *
         * ********************************* */
        cstr = NULL;
        try { cstr = billCfg.getString("smsExtraConfig"); }
        catch (ConfigException& exc) { }
        if (cstr && cstr[0]) {
            smsc_log_info(inmanLogger, "'smsExtraConfig': %s", cstr);
            smsXcfg += cstr;
        }
#endif /* SMSEXTRA */

        /* ********************************* *
         * SS7 stack interaction parameters: *
         * ********************************* */
        if (bill.billMode != smsc::inman::BILL_NONE) {
            readSS7CFG(manager, bill.ss7);
        }

        /* ***************************************************************** *
         * AbonentPolicies: (IN-platforms and AbonentProviders) parameters:  *
         * ***************************************************************** */
        if (policyNm) { //default policy
            abPolicies.setPreferred(readPolicyCFG(manager, policyNm));
        } else if (bill.billMode != smsc::inman::BILL_NONE) {
            throw ConfigException("Default abonent policy is not set!");
        }
        //todo: policies address pool mask is not supported yet!
        /**/
        return;
    }

    void readXServiceParms(ConfigView * cfg, const std::string & nm_srv) throw(ConfigException)
    {
        smsc_log_info(inmanLogger, "SMS Extra service '%s' config ..", nm_srv.c_str());
        XSmsService xSrv(nm_srv);
        std::auto_ptr<ConfigView> srvCfg(cfg->getSubConfig(nm_srv.c_str()));

        try { xSrv.mask = (uint32_t)srvCfg->getInt("serviceMask"); }
        catch (ConfigException& exc) { }
        if (!xSrv.mask || (xSrv.mask & SMSX_RESERVED_MASK))
            throw ConfigException("'serviceMask' is missed or invalid or reserved bits is used");

        SmsXServiceMap::iterator xit = bill.smsXMap.find(xSrv.mask);
        if (xit != bill.smsXMap.end())
            throw ConfigException("'serviceMask' %u is shared by %s and %s",
                                  xSrv.mask, ((*xit).second).name.c_str(), nm_srv.c_str());

        try { xSrv.cdrCode = (uint32_t)srvCfg->getInt("serviceCode"); }
        catch (ConfigException& exc) { }
        if (!xSrv.cdrCode)
            throw ConfigException("'serviceCode' is missed or invalid");

        char * cstr = NULL;
        try { cstr = srvCfg->getString("serviceAdr"); } //optional param
        catch (ConfigException& exc) { }
        if (!cstr || !cstr[0])
            smsc_log_warn(inmanLogger, "  'serviceAdr' is omitted");
        else if (!xSrv.adr.fromText(cstr))
            throw ConfigException("'serviceAdr' is invalid: %s", cstr);

        bill.smsXMap.insert(SmsXServiceMap::value_type(xSrv.mask, xSrv));
        smsc_log_info(inmanLogger, "  service[0x%x]: %u, %s", xSrv.mask, xSrv.cdrCode,
                      xSrv.adr.toString().c_str());
        return;        
    }

    void readExtraConfig(Manager& manager) throw(ConfigException)
    {
        /* ********************************* *
         * SMS Extra services parameters: *
         * ********************************* */
        if (!manager.findSection("SMSExtra"))
            throw ConfigException("'SMSExtra' section is missed");
        ConfigView smsXCfg(manager, "SMSExtra");

        std::auto_ptr<CStrSet> srvPacks(smsXCfg.getShortSectionNames());
        if (srvPacks->empty())
            throw ConfigException("no SMS Extra services specified");

        CStrSet::iterator sit = srvPacks->begin();
        for (; sit != srvPacks->end(); sit++)
            readXServiceParms(&smsXCfg, *sit);
        smsc_log_info(inmanLogger, "Total SMS Extra services: %u", bill.smsXMap.size());
        return;
    }
};


} //inman
} //smsc

using smsc::inman::INManConfig;

int main(int argc, char** argv)
{
    int     rval = 0;
    char *  cfgFile = (char*)"config.xml";

    tzset();
    Logger::Init();
    inmanLogger = Logger::getInstance("smsc.inman");
    _EINSS7_logger_DFLT = Logger::getInstance("smsc.inman.inap");

    smsc_log_info(inmanLogger,"******************************");
    smsc_log_info(inmanLogger,"* SIBINCO IN MANAGER v%u.%u.%u *",
                  INMAN_VER_HIGH, INMAN_VER_LOW, INMAN_VER_FIX);
    smsc_log_info(inmanLogger,"******************************");
    if (argc > 1)
        cfgFile = argv[1];
    smsc_log_info(inmanLogger,"* Config file: %s", cfgFile);
    smsc_log_info(inmanLogger,"******************************");

    std::auto_ptr<INManConfig> pCfg(new INManConfig());
    try {
        Manager::init((const char *)cfgFile);
        Manager& manager = Manager::getInstance();
        pCfg->read(manager);
        const char * nm_xcfg = pCfg->hasExtraConfig();
        if (nm_xcfg) {
            manager.deinit();
            smsc_log_info(inmanLogger, "Reading smsExtra config %s ..", nm_xcfg);
            manager.init(nm_xcfg);
            pCfg->readExtraConfig(manager);
        }
    } catch (ConfigException& exc) {
        smsc_log_error(inmanLogger, "Config: %s", exc.what());
        smsc_log_error(inmanLogger, "Configuration invalid. Exiting.");
        exit(-1);
    }
    //INman uses the CAP3SMS application context for interaction with IN-point
    assert(
        ApplicationContextFactory::Init(ACOID::id_ac_cap3_sms_AC, initCAP3SMSComponents)/*;*/
    );

    try {
        g_pService = new Service(pCfg.release(), inmanLogger);
        assert(g_pService);
        _runService = 1;
        if (g_pService->start()) {
            //handle SIGTERM only in main thread
            sigset(SIGTERM, sighandler);
        } else {
            smsc_log_fatal(inmanLogger, "InmanSrv: startup failure. Exiting.");
            _runService = 0;
        }

        while(_runService)
            usleep(1000 * 200); //sleep 200 ms
        g_pService->stop();

    } catch(const std::exception& error) {
        smsc_log_fatal(inmanLogger, "%s", error.what() );
        fprintf( stderr, "Fatal error: %s\n", error.what() );
        rval = 1;
    }
    if (g_pService)
        delete g_pService;
    smsc_log_info(inmanLogger, "IN MANAGER shutdown complete");
    return rval;
}

