#pragma ident "$Id$"
/* ************************************************************************** *
 * INMan service config file parsing.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_CONFIG_PARSING__
#define __SMSC_INMAN_CONFIG_PARSING__

#include <stdio.h>
#include <memory>
#include <string.h>
#include <string>

#include "inman/inman.hpp"
using smsc::inman::ChargeObj;

#include "util/config/ConfigView.h"
#include "util/mirrorfile/mirrorfile.h"
using smsc::util::config::ConfigView;
using smsc::util::config::CStrSet;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;

#include "inman/abprov/IAPLoader.hpp"
using smsc::inman::iaprvd::IAProviderLoader;
using smsc::inman::iaprvd::IAProviderCreatorITF;
using smsc::inman::iaprvd::_IAPTypes;
using smsc::inman::iaprvd::_IAPAbilities;

#include "inman/service.hpp"
using smsc::inman::BillModes;

#include "inman/common/CSVList.hpp"
using smsc::util::CSVList;

namespace smsc {
namespace inman {


class INScfsRegistry {
protected:
    typedef std::map<TDPCategory::Id, const char *> TDPNames;

    TDPNames    tdpNames;
    INScfsMAP   scfMap;
    Logger *    logger;

    TDPCategory::Id str2tdp(const char * str)
    {
        for (TDPNames::const_iterator it = tdpNames.begin(); it != tdpNames.end(); ++it) {
            if (!strcmp(str, it->second))
                return it->first;
        }
        return TDPCategory::dpUnknown;
    }

    bool str2UInt(uint32_t * p_val, const std::string & str)
    {
        if (str.empty())
            return false;
        *p_val = atoi(str.c_str());
        if (!*p_val) { //check for all zeroes
            if (strspn(str.c_str(), "0") != str.size())
                return false;
        }
        return true;
    }

    SKAlgorithmAC * readSkeyVal(TDPCategory::Id tdp_type, std::string & str)
    {
        uint32_t skey = 0;
        if (str2UInt(&skey, str))
            return new SKAlgorithm_SKVal(tdp_type, skey);
        return NULL;
    }

    bool readSKeyMap(SKAlgorithm_SKMap * alg, ConfigView * xlt_cfg)
    {
        std::auto_ptr<CStrSet> subs(xlt_cfg->getIntParamNames());
        for (CStrSet::const_iterator sit = subs->begin(); sit != subs->end(); ++sit) {
            uint32_t argKey = 0, resKey = 0;
            //check paramName
            if (str2UInt(&argKey, *sit)) {
                try {
                    resKey = (uint32_t)xlt_cfg->getInt(sit->c_str());
                    alg->insert(argKey, resKey);
                } catch (const ConfigException & exc) {
                    return false;
                }
            } else
                return false;
        }
        return true;
    }

    //"[algId :] algArg [: algParams] "
    SKAlgorithmAC * readSkeyAlg(ConfigView * scf_cfg, TDPCategory::Id tdp_type, const char * str)
    {
        CSVList algStr(str, ':');
        CSVList::size_type n = algStr.size();
        if (!n)
            return NULL;
        if (n == 1) //just a value
            return readSkeyVal(tdp_type, algStr[0]);
        // n >= 2
        if (!strcmp("val", algStr[0].c_str())) //just a value
            return readSkeyVal(tdp_type, algStr[1]);

        if (!strcmp("map", algStr[0].c_str())) {
            if (n < 3)
                return NULL;

            TDPCategory::Id argTdp = str2tdp(algStr[1].c_str());
            if (argTdp == TDPCategory::dpUnknown)
                return NULL;
            if (!scf_cfg->findSubSection(algStr[2].c_str()))
                return NULL;
            std::auto_ptr<SKAlgorithm_SKMap> alg(new SKAlgorithm_SKMap(tdp_type, argTdp));
            std::auto_ptr<ConfigView> xltCfg(scf_cfg->getSubConfig(algStr[2].c_str()));
            bool res = readSKeyMap(alg.get(), xltCfg.get());
            return (res && alg->size()) ? alg.release() : NULL;
        }
    }

    unsigned readSrvKeys(ConfigView * scf_cfg, SKAlgorithmMAP & sk_alg) throw(ConfigException)
    {
        std::auto_ptr<ConfigView> skeyCfg(scf_cfg->getSubConfig("ServiceKeys"));
        std::auto_ptr<CStrSet> subs(skeyCfg->getStrParamNames());
        for (CStrSet::iterator sit = subs->begin(); sit != subs->end(); ++sit) {
            TDPCategory::Id tdpType = str2tdp(sit->c_str());
            if (tdpType != TDPCategory::dpUnknown) {
                char *          cstr = skeyCfg->getString(sit->c_str());
                SKAlgorithmAC * alg = readSkeyAlg(scf_cfg, tdpType, cstr);
                if (alg) {
                    sk_alg[tdpType] = alg;
                    smsc_log_info(logger, "  skey %s %s", sit->c_str(),
                                                    alg->toString().c_str());
                } else
                    throw ConfigException("  %s service key value/algorithm is invalid",
                                            TDPCategory::Name(tdpType));
            } else
                smsc_log_warn(logger, " %s service key is unknown/unsupported", sit->c_str());
        }
        return scfMap.size();
    }

public:
    INScfsRegistry()
    { 
        tdpNames[TDPCategory::dpMO_BC] = TDPCategory::Name(TDPCategory::dpMO_BC);
        tdpNames[TDPCategory::dpMO_SM] = TDPCategory::Name(TDPCategory::dpMO_SM);
    }
    ~INScfsRegistry()
    {
        for (INScfsMAP::iterator sit = scfMap.begin(); sit != scfMap.end(); sit++)
            delete (*sit).second;
    }

    void Init(Logger * use_log) { logger = use_log; }

    inline void copyAll(INScfsMAP & cp_map)
    {
        cp_map.insert(scfMap.begin(), scfMap.end());
    }

    //NOTE: the 'nm_scf' subsection presence must be previously checked !!!
    const INScfCFG * readSCF(ConfigView * scf_sec, const char * nm_scf, bool warn = false) throw(ConfigException)
    {   // according to INScfCFG::IDPLocationAddr
        static const char * const _IDPLIAddr[] = { "MSC", "SMSC", "SSF" };
        static const char * const _IDPReqMode[] = { "MT", "SEQ" };

        std::auto_ptr<ConfigView> scfCfg(scf_sec->getSubConfig(nm_scf));

        char * cstr = NULL;
        cstr = scfCfg->getString("scfAddress");
        if (!cstr[0])
            throw ConfigException("%s.scfAddress missing", nm_scf);

        TonNpiAddress scfAdr;
        if (!scfAdr.fromText(cstr) || !scfAdr.fixISDN())
            throw ConfigException("%s.scfAddress is invalid: %s", nm_scf, cstr);

        //check uniqueness
        INScfsMAP::const_iterator sit = scfMap.find(scfAdr.toString());
        if (sit != scfMap.end()) {
            if (warn)
                smsc_log_info(logger, "IN-platform '%s' already known ..", nm_scf);
            return (*sit).second;
        }

        //read configuration
        std::auto_ptr<INScfCFG> pin;

        cstr = NULL;
        try { cstr = scfCfg->getString("aliasFor", 0, false); }
        catch (const ConfigException & exc) { }

        if (cstr && cstr[0]) {
            pin.reset((INScfCFG*)readSCF(scf_sec, cstr)); //throws
            smsc_log_info(logger, "IN-platform '%s' config ..", nm_scf);
            smsc_log_info(logger, "  ISDN: %s", scfAdr.toString().c_str());
            smsc_log_info(logger, "  aliasFor: %s", cstr);
        } else {
            pin.reset(new INScfCFG(nm_scf));
            pin->scfAdr = scfAdr;
            smsc_log_info(logger, "IN-platform '%s' config ..", nm_scf);
            smsc_log_info(logger, "  ISDN: %s", scfAdr.toString().c_str());

            //Read service keys
            if (!scfCfg->findSubSection("ServiceKeys"))
                throw ConfigException("'ServiceKeys' section is missed");
            readSrvKeys(scfCfg.get(), pin->skAlg);

            // -- OPTIONAL parameters --//

            //list of RP causes forcing charging denial because of low balance
            cstr = NULL; std::string cppStr = "  RPCList_reject: ";
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
            smsc_log_info(logger, cppStr.c_str());

            //list of RP causes indicating that IN point should be
            //interacted again a bit later
            cstr = NULL; cppStr = "  RPCList_retry: ";
            try { cstr = scfCfg->getString("RPCList_retry"); }
            catch (ConfigException& exc) { }
            if (cstr) {
                try { pin->retryRPC.init(cstr); }
                catch (std::exception& exc) {
                    throw ConfigException("RPCList_retry: %s", exc.what());
                }
            }
            //adjust default attempt setings for given RPCauses
            for (RPCListATT::iterator it = pin->retryRPC.begin();
                                    it != pin->retryRPC.end(); ++it) {
                if (!it->second)
                    it->second++;
            }
            if (!pin->retryRPC.print(cppStr))
                cppStr += "unsupported";
            smsc_log_info(logger, cppStr.c_str());

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
            smsc_log_info(logger, "  IDPLocationInfo: %s", cstr);

            cstr = NULL; cppStr = "IDPReqMode: ";
            try { cstr = scfCfg->getString("IDPReqMode");}
            catch (ConfigException& exc) { }

            if (cstr && cstr[0]) {
                if (!strcmp(cstr, _IDPReqMode[INScfCFG::idpReqSEQ]))
                    pin->idpReqMode = INScfCFG::idpReqSEQ;
                else if (strcmp(cstr, _IDPReqMode[INScfCFG::idpReqMT]))
                    throw ConfigException("IDPReqMode: invalid value");
            } else
                cstr = (char*)_IDPReqMode[INScfCFG::idpReqMT];
            smsc_log_info(logger, "  IDPReqMode: %s", cstr);
        }
        scfMap.insert(INScfsMAP::value_type(scfAdr.toString(), pin.get()));
        return pin.release();
    }

    unsigned readSCFs(ConfigView * scf_sec) throw(ConfigException)
    {
        std::auto_ptr<CStrSet> subs(scf_sec->getShortSectionNames());
        CStrSet::iterator sit = subs->begin();
        for (; sit != subs->end(); sit++)
            readSCF(scf_sec, sit->c_str(), true);
        return scfMap.size();
    }
};

class INManConfig : public InService_CFG {
public:
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

//protected members:
    INScfsRegistry      scfMap;
    std::string         smsXcfg; //SMSExtra services config file
    Logger *            logger;

//protected methods:

    AbonentPolicy * readPolicyCFG(Manager & manager, const char * nm_pol) throw(ConfigException)
    {
        AbonentPolicyXCFG   polXCFG(nm_pol);

        if (!manager.findSection("AbonentPolicies"))
            throw ConfigException("'AbonentPolicies' section is missed");

        ConfigView  polSec(manager, "AbonentPolicies");
        if (!polSec.findSubSection(polXCFG.ident))
            throw ConfigException("'%s' policy is not specified", polXCFG.ident);

        std::auto_ptr<AbonentPolicy> policyCFG(new AbonentPolicy(nm_pol));

        std::auto_ptr<ConfigView> polCfg(polSec.getSubConfig(polXCFG.ident));
        //parse policy cfg
        smsc_log_info(logger, "Reading %s policy config ..", polXCFG.ident);

        char * cstr = NULL;
        cstr = polCfg->getString("policy");
        if (!cstr || !cstr[0])
            throw ConfigException("'%s' policy value is not specified", polXCFG.ident);
        if (!polXCFG.init(cstr))
            throw ConfigException("'%s' policy value is not valid: %s", polXCFG.ident, cstr);
        {
            std::string pstr;
            polXCFG.print(pstr);
            smsc_log_info(logger, "policy is: %s", pstr.c_str());
        }

        //lookup IN platforms configs
        if (polXCFG.scfNms.size()) {
            if (!manager.findSection("IN-platforms"))
                throw ConfigException("'IN-platforms' section is missed");
            ConfigView  scfSec(manager, "IN-platforms");

            if (!strcmp((char*)"*", *(polXCFG.scfNms.begin()))) { //include all SCFs defined
                if (!scfMap.readSCFs(&scfSec))
                    throw ConfigException("IN-platforms is not defined");
                scfMap.copyAll(policyCFG->scfMap);
            } else {
                for (CPTRList::iterator sit = polXCFG.scfNms.begin();
                                        sit != polXCFG.scfNms.end(); sit++) {
                    if (!scfSec.findSubSection(*sit))
                        throw ConfigException("IN-platform '%s'is not defined", *sit);
                    const INScfCFG * pin = scfMap.readSCF(&scfSec, *sit);
                    policyCFG->scfMap.insert(
                        INScfsMAP::value_type(pin->scfAdr.toString(), pin));
                }
            }
        }
        //lookup Abonent Provider config and load it
        if (polXCFG.prvdNm) {
            if (!manager.findSection("AbonentProviders"))
                throw ConfigException("'AbonentProviders' section is missed");
            ConfigView  prvdSec(manager, "AbonentProviders");
            if (!prvdSec.findSubSection(polXCFG.prvdNm))
                throw ConfigException("'%s' abonent provider is missed", polXCFG.prvdNm);

            std::auto_ptr<ConfigView> provCfg(prvdSec.getSubConfig(polXCFG.prvdNm));
            smsc_log_info(logger, "Loading AbonentProvider '%s'", polXCFG.prvdNm);
            policyCFG->provAllc = IAProviderLoader::LoadIAP(provCfg.get(), logger); //throws
            smsc_log_info(logger, "AbonentProvider '%s': type %s, ident: %s, ability: %s",
                          polXCFG.prvdNm, _IAPTypes[policyCFG->provAllc->type()],
                          policyCFG->provAllc->ident(), 
                          _IAPAbilities[policyCFG->provAllc->ability()]);
            policyCFG->provAllc->logConfig(logger);
        }

        return policyCFG.release();
    }

    void readSS7CFG(Manager & manager, SS7_CFG & ss7) throw(ConfigException)
    {
        if (!manager.findSection("SS7"))
            throw ConfigException("'SS7' section is missed");

        smsc_log_info(logger, "Reading SS7 configuration ..");
        uint32_t tmo;
        char *   cstr;
        ConfigView ss7Cfg(manager, "SS7");

        cstr = NULL;
        try { cstr = ss7Cfg.getString("ssfAddress"); }
        catch (ConfigException& exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("SSF address is missing");
        if (!ss7.ssf_addr.fromText(cstr) || !ss7.ssf_addr.fixISDN())
            throw ConfigException("SSF address is invalid: %s", cstr);

        ss7.own_ssn = ss7Cfg.getInt("ssn"); //throws
        smsc_log_info(logger, "  SSF: %u:%s", ss7.own_ssn,
                        ss7.ssf_addr.toString().c_str());

        /*  optional SS7 interaction parameters */
        ss7.userId = _in_CFG_DFLT_SS7_USER_ID;
        ss7.capTimeout = _in_CFG_DFLT_CAP_TIMEOUT;
        ss7.maxDlgId = _in_CFG_DFLT_TCAP_DIALOGS;

        tmo = 0;    //ss7UserId
        try { tmo = (uint32_t)ss7Cfg.getInt("ss7UserId"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (!tmo || (tmo > 20))
                throw ConfigException("'ss7UserId' should fall into the range [1..20]");
            ss7.userId = (unsigned char)tmo;
        }
        smsc_log_info(logger, "  ss7UserId: %u%s", ss7.userId, !tmo ? " (default)":"");

        tmo = 0;    //maxTimeout
        try { tmo = (uint32_t)ss7Cfg.getInt("maxTimeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535)
                throw ConfigException("'maxTimeout' should be less than 65535 seconds");
            ss7.capTimeout = (unsigned short)tmo;
        }
        smsc_log_info(logger, "  maxTimeout: %u secs%s", ss7.capTimeout, !tmo ? " (default)":"");

        tmo = 0;    //maxDialogs
        try { tmo = (uint32_t)ss7Cfg.getInt("maxDialogs"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if ((tmo >= 65530) || (tmo < 2))
                throw ConfigException("'maxDialogs' should fall into the range [2..65530]");
            ss7.maxDlgId = (unsigned short)tmo;
        }
        smsc_log_info(logger, "  maxDialogs: %u%s", ss7.maxDlgId, !tmo ? " (default)":"");
        return;
    }

    void str2BillMode(const char * m_str, ChargeObj::BILL_MODE (& pbm)[2]) throw(ConfigException)
    {
        if (!m_str || !m_str[0])
            throw ConfigException("Invalid billMode");

        CSVList bmList(m_str);
        if (bmList.empty() || (bmList.size() > 2))
            throw ConfigException("Invalid billMode '%s'", m_str);

        pbm[0] = pbm[1] = ChargeObj::billOFF;
        CSVList::size_type i = 0;
        do {
            if (!strcmp(_BILLmodes[ChargeObj::bill2IN], bmList[i].c_str()))
                pbm[i] = ChargeObj::bill2IN;
            else if (!strcmp(_BILLmodes[ChargeObj::bill2CDR], bmList[i].c_str())) {
                pbm[i] = ChargeObj::bill2CDR; // no need to check next value
                pbm[++i] = ChargeObj::bill2CDR; 
            } else if (!strcmp(_BILLmodes[ChargeObj::billOFF], bmList[i].c_str()))
                ++i; // no need to check next value
            else
                throw ConfigException("Invalid billMode '%s'", bmList[i].c_str());
        } while (++i < bmList.size());

        //check bill2IN setting ..
        if ((pbm[0] == ChargeObj::bill2IN) && (pbm[0] == pbm[1]))
            throw ConfigException("Invalid billMode '%s'", m_str);
    }

    void readBillMode(ChargeObj::MSG_TYPE msg_type, const char * mode, bool mt_bill)
            throw(ConfigException)
    {
        BillModes * b_map = mt_bill ? &bill.mt_billMode : &bill.mo_billMode;
        if (b_map->isAssigned(msg_type))
            throw ConfigException("Multiple settings for '%s'", _MSGtypes[msg_type]);

        ChargeObj::BILL_MODE    pbm[2];
        str2BillMode(mode, pbm);
        b_map->assign(msg_type, pbm[0], pbm[1]);
        smsc_log_info(logger, "    %s -> %s, %s", mt_bill ? "MT" : "MO",
            bill.billModeStr(pbm[0]), bill.billModeStr(pbm[1]));
    }

    void readModesFor(ChargeObj::MSG_TYPE msg_type, ConfigView * m_cfg)
            throw(ConfigException)
    {
        char * mode = NULL;
        smsc_log_info(logger, "  %s ..", bill.msgTypeStr(msg_type));

        try { mode = m_cfg->getString("MO"); }
        catch (const ConfigException& exc) { }
        if (!mode || !mode[0])
            throw ConfigException("%s parameter 'MO' is invalid or missing!",
                                    bill.msgTypeStr(msg_type));
        readBillMode(msg_type, mode, false);

        try { mode = m_cfg->getString("MT"); }
        catch (const ConfigException& exc) { }
        if (!mode || !mode[0])
            throw ConfigException("%s parameter 'MT' is invalid or missing!",
                                    bill.msgTypeStr(msg_type));
        readBillMode(msg_type, mode, true);
    }

    /* Reads BillingModes subsection */
    void readBillingModes(ConfigView & cfg)  throw(ConfigException)
    {
        if (!cfg.findSubSection("BillingModes"))
            throw ConfigException("'BillingModes' subsection is missed");

        std::auto_ptr<ConfigView>   bmCfg(cfg.getSubConfig("BillingModes"));
        std::auto_ptr<CStrSet>      msgs(bmCfg->getShortSectionNames());
        if (msgs->empty())
            throw ConfigException("no billing modes set");

        uint32_t tmo = 0;
        char * cstr = NULL;
        for (CStrSet::iterator sit = msgs->begin(); sit != msgs->end(); ++sit) {
            std::auto_ptr<ConfigView> curMsg(bmCfg->getSubConfig(sit->c_str()));
            if (!strcmp(_MSGtypes[ChargeObj::msgSMS], sit->c_str()))
                readModesFor(ChargeObj::msgSMS, curMsg.get());
            else if (!strcmp(_MSGtypes[ChargeObj::msgUSSD], sit->c_str()))
                readModesFor(ChargeObj::msgUSSD, curMsg.get());
            else if (!strcmp(_MSGtypes[ChargeObj::msgXSMS], sit->c_str()))
                readModesFor(ChargeObj::msgXSMS, curMsg.get());
            else
                throw ConfigException("Illegal section for messageType %s", sit->c_str());
        }
    }

    //returns default policy name
    const char * readBillCFG(Manager & manager) throw(ConfigException)
    {
        //according to BillingCFG::ContractReqMode
        static const char * _abReq[] = { "onDemand", "always" };
        uint32_t tmo = 0;
        char * cstr = NULL;
        char * policyNm = NULL; //abonent contract determination policy

        if (!manager.findSection("Billing"))
            throw ConfigException("'Billing' section is missed");
        ConfigView billCfg(manager, "Billing");
        smsc_log_info(logger, "Reading Billing settings ..");

        //read BillingModes subsection
        readBillingModes(billCfg);

        cstr = NULL;
        bill.cntrReq = BillingCFG::reqOnDemand;
        try { cstr = billCfg.getString("abonentTypeRequest");
        } catch (ConfigException& exc) { }
        if (cstr && cstr[0]) {
            if (!strcmp(_abReq[BillingCFG::reqAlways], cstr)) {
                bill.cntrReq = BillingCFG::reqAlways; 
            } else if (strcmp(_abReq[BillingCFG::reqOnDemand], cstr))
                throw ConfigException("illegal 'abonentTypeRequest' value");
        } else
            cstr = NULL;
        smsc_log_info(logger, "abonentTypeRequest: %s%s", _abReq[bill.cntrReq],
                      !cstr ? " (default)":"");

        if ((bill.cntrReq == BillingCFG::reqAlways)
            || bill.mo_billMode.useIN() || bill.mt_billMode.useIN()) {
            //abonent contract determination policy is required
            cstr = NULL;
            try { policyNm = billCfg.getString("abonentPolicy");
            } catch (ConfigException& exc) { }

            if (!policyNm || !policyNm[0])
                throw ConfigException("abonent contract determination policy is not set!");
        } else
            smsc_log_info(logger, "abonent contract determination policy is not used");

        tmo = 0;    //abtTimeout
        try { tmo = (uint32_t)billCfg.getInt("abonentTypeTimeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535)
                throw ConfigException("'abonentTypeTimeout' should fall into the range [1 ..65535] seconds");
            bill.abtTimeout = (unsigned short)tmo;
        }
        smsc_log_info(logger, "abonentTypeTimeout: %u secs%s", (unsigned)bill.abtTimeout.Value(),
                      !tmo ? " (default)":"");

        cstr = NULL;
        try { cstr = billCfg.getString("cdrMode");
        } catch (ConfigException& exc) {
            throw ConfigException("'cdrMode' is unknown or missing");
        }
        if (!strcmp(cstr, _CDRmodes[BillingCFG::cdrALL]))
            bill.cdrMode = BillingCFG::cdrALL;
        else if (!strcmp(cstr, _CDRmodes[BillingCFG::cdrNONE]))
            bill.cdrMode = BillingCFG::cdrNONE;
        else if (strcmp(cstr, _CDRmodes[BillingCFG::cdrBILLMODE]))
            throw ConfigException("'cdrMode' is unknown or missing");
        smsc_log_info(logger, "cdrMode: %s [%d]", cstr, bill.cdrMode);

        if (bill.cdrMode != BillingCFG::cdrNONE) {
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
            smsc_log_info(logger, "cdrDir: %s", cstr);
            smsc_log_info(logger, "cdrInterval: %d secs", bill.cdrInterval);
        }
        //cache parameters
        try {
            if (!(cachePrm.interval = (long)billCfg.getInt("cacheInterval")))
                cachePrm.interval = _in_CFG_DFLT_CACHE_INTERVAL;
            smsc_log_info(logger, "cacheInterval: %d minutes", cachePrm.interval);
            //convert minutes to seconds
            cachePrm.interval *= 60;
        } catch (ConfigException& exc) {
            throw ConfigException("'cacheInterval' is missing");
        }
        try {
            if (!(cachePrm.RAM = (long)billCfg.getInt("cacheRAM")))
                throw ConfigException("'cacheRAM' is missing or invalid");
            smsc_log_info(logger, "cacheRAM: %d Mb", cachePrm.RAM);
        } catch (ConfigException& exc) {
            throw ConfigException("'cacheRAM' is missing or invalid");
        }
        cstr = NULL;
        try { cstr = billCfg.getString("cacheDir"); }
        catch (ConfigException& exc) { }
        if (!cstr || !cstr[0])
            throw ConfigException("'cacheDir' is missing");
        cachePrm.nmDir += cstr;
        smsc_log_info(logger, "cacheDir: %s", cstr);

        tmo = 0;
        try { tmo = (uint32_t)billCfg.getInt("cacheRecords"); }
        catch (ConfigException& exc) { }
        if (tmo)
            cachePrm.fileRcrd = (int)tmo;
        smsc_log_info(logger, "cacheRecords: %d%s", cachePrm.fileRcrd,
                      !tmo ? " (default)":"");

        tmo = 0;    //maxBillings
        try { tmo = (uint32_t)billCfg.getInt("maxBillings"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo > _in_CFG_MAX_BILLINGS)
                throw ConfigException(format("'maxBillings' should be less than %u",
                                             _in_CFG_MAX_BILLINGS).c_str());
            bill.maxBilling = (unsigned short)tmo;
        }
        smsc_log_info(logger, "maxBillings: %u per connect%s", bill.maxBilling,
                      !tmo ? " (default)":"");

        tmo = 0;    //maxTimeout
        try { tmo = (uint32_t)billCfg.getInt("maxTimeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if ((tmo >= 65535) || (tmo < 5))
                throw ConfigException("'maxTimeout' should fall into the range [5 ..65535] seconds");
            sock.timeout = (unsigned short)tmo;
            bill.maxTimeout = (unsigned short)tmo;
        }
        smsc_log_info(logger, "maxTimeout: %u secs%s", (unsigned)bill.maxTimeout.Value(),
                      !tmo ? " (default)":"");

#ifdef SMSEXTRA
        /* ********************************* *
         * SMS Extra services configuration: *
         * ********************************* */
        cstr = NULL;
        try { cstr = billCfg.getString("smsExtraConfig"); }
        catch (ConfigException& exc) { }
        if (cstr && cstr[0]) {
            smsc_log_info(logger, "'smsExtraConfig': %s", cstr);
            smsXcfg += cstr;
        }
#endif /* SMSEXTRA */
        return policyNm;
    }

    void readXServiceParms(ConfigView * cfg, const std::string & nm_srv) throw(ConfigException)
    {
        smsc_log_info(logger, "SMS Extra service '%s' config ..", nm_srv.c_str());
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
            smsc_log_warn(logger, "  'serviceAdr' is omitted");
        else if (!xSrv.adr.fromText(cstr))
            throw ConfigException("'serviceAdr' is invalid: %s", cstr);

        try { xSrv.chargeBearer = srvCfg->getBool("chargeBearer"); } //optional param
        catch (ConfigException& exc) { }

        bill.smsXMap.insert(SmsXServiceMap::value_type(xSrv.mask, xSrv));
        smsc_log_info(logger, "  service[0x%x]: %u, %s%s", xSrv.mask, xSrv.cdrCode,
                      xSrv.adr.toString().c_str(), xSrv.chargeBearer ? ", chargeBearer" : "");
        return;        
    }

public:

    INManConfig(Logger * use_log) : InService_CFG(), logger(use_log)
    {
        //OPTIONAL PARAMETERS:
        sock.maxConn = _in_CFG_DFLT_CLIENT_CONNS;
        bill.maxBilling = _in_CFG_DFLT_BILLINGS;
        sock.timeout = _in_CFG_DFLT_BILL_TIMEOUT;
        bill.maxTimeout = _in_CFG_DFLT_BILL_TIMEOUT;
        bill.abtTimeout = _in_CFG_DFLT_ABTYPE_TIMEOUT;
        cachePrm.fileRcrd = _in_CFG_DFLT_CACHE_RECORDS;
        scfMap.Init(logger);
    }

    ~INManConfig()
    { }

    const char * hasExtraConfig(void) const { return smsXcfg.length() ? smsXcfg.c_str() : NULL; }


    void read(Manager& manager) throw(ConfigException)
    {
        uint32_t tmo = 0;
        char *   cstr = NULL;

        /* ********************* *
         * InService parameters: *
         * ********************* */
        try { cstr = manager.getString("version");
        } catch (ConfigException& exc) { }
        if (!cstr || !cstr[0])
            smsc_log_warn(logger, "Config version is not set");
        else
            smsc_log_info(logger, "Config version: %s", cstr);

        try {
            cstr = manager.getString("host");
            sock.port = manager.getInt("port");
            smsc_log_info(logger, "INMan: %s:%d", cstr, sock.port);
        } catch (ConfigException& exc) {
            throw ConfigException("INMan host or port missing");
        }
        sock.host += cstr;

        tmo = 0;
        try { tmo = (uint32_t)manager.getInt("maxClients"); }
        catch (ConfigException& exc) { }
        if (tmo)
            sock.maxConn = (unsigned short)tmo;
        smsc_log_info(logger, "maxClients: %u%s", sock.maxConn, !tmo ? " (default)":"");

        /* ******************** *
         * Billing parameters:  *
         * ******************** */
        const char * policyNm = readBillCFG(manager);

        /* ***************************************************************** *
         * AbonentPolicies: (IN-platforms and AbonentProviders) parameters:  *
         * ***************************************************************** */
        if (policyNm) { //default policy
            abPolicies.setPreferred(readPolicyCFG(manager, policyNm));
        }
        //todo: policies address pool mask is not supported yet!

        /* ********************************* *
         * SS7 stack interaction parameters: *
         * ********************************* */
        if (bill.mo_billMode.useIN() || bill.mt_billMode.useIN() || abPolicies.useSS7())
            readSS7CFG(manager, bill.ss7);
        else
            smsc_log_info(logger, "SS7 stack not in use!");

        /**/
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
        smsc_log_info(logger, "Total SMS Extra services: %u", bill.smsXMap.size());
        return;
    }
};

} //inman
} //smsc
#endif /* __SMSC_INMAN_CONFIG_PARSING__ */

