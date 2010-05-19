/* ************************************************************************** *
 * SMS/USSD Billing service config file parsing.
 * ************************************************************************** */
#ifndef __SMSC_INMAN_CONFIG_PARSING__
#ident "@(#)$Id$"
#define __SMSC_INMAN_CONFIG_PARSING__

#include "inman/services/ICSCfgReader.hpp"
using smsc::inman::ICSIdent;
using smsc::inman::ICSrvCfgReaderAC_T;

#include "inman/services/smbill/SmBillDefs.hpp"

namespace smsc   {
namespace inman  {
namespace smbill {

#ifdef SMSEXTRA
// SMS Extra services configuration reader
class ICSXSMSCfgReader : public ICSMultiSectionCfgReaderAC_T<SmsXServiceMap> {
protected:
    CfgState parseSection(XConfigView * cfg_sec,
                        const std::string & nm_sec, void * opaque_arg = NULL)
        throw(ConfigException);

public:
    ICSXSMSCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
        : ICSMultiSectionCfgReaderAC_T<SmsXServiceMap>(root_sec, use_log,
                                                       ics_sec ? ics_sec : "SMSExtra")
    { }
    ~ICSXSMSCfgReader()
    { }
};
#endif /* SMSEXTRA */


class ICSSmBillingCfgReader : public ICSrvCfgReaderAC_T<SmBillingXCFG> {
private:
    void str2BillMode(const char * m_str, ChargeParm::BILL_MODE (& pbm)[2])
        throw(ConfigException);
    //
    void readBillMode(ChargeParm::MSG_TYPE msg_type, const char * mode, bool mt_bill)
        throw(ConfigException);
    //
    void readModesFor(ChargeParm::MSG_TYPE msg_type, XConfigView * m_cfg)
        throw(ConfigException);
    /* Reads BillingModes subsection */
    void readBillingModes(XConfigView & cfg) throw(ConfigException);
    //Returns true if service depends on other ones
    TCAPUsr_CFG * readCAP3Sms(XConfigView & cfg) throw(ConfigException);

protected:
    static const uint32_t   _MIN_CDR_ROLL_INTERVAL = 10;    //units: seconds
    static const uint32_t   _DFLT_CDR_ROLL_INTERVAL = 60;   //units: seconds
    static const uint32_t   _MAX_BILLINGS_NUM = 0xFFFF;
    static const uint32_t   _DFLT_BILLINGS_NUM = 1000;
    static const uint16_t   _MAX_ABTYPE_TIMEOUT = 0xFFFF;   //units: seconds
    static const uint16_t   _DFLT_ABTYPE_TIMEOUT = 8;       //units: seconds
    static const uint16_t   _MAX_DELAY_SECS = 0xFFFF;       //units: seconds
    static const uint16_t   _DFLT_BILL_TIMEOUT = 120;       // >= 3*30 (Tssf)

    //Parses XML configuration entry section, updates dependencies.
    //Returns status of config parsing, 
    CfgState parseConfig(void * opaque_obj = NULL) throw(ConfigException);

public:
    ICSSmBillingCfgReader(Config & root_sec, Logger * use_log, const char * ics_sec = NULL)
        : ICSrvCfgReaderAC_T<SmBillingXCFG>(root_sec, use_log, ics_sec ? ics_sec : "Billing")
    {
        icsDeps.insert(ICSIdent::icsIdTCPServer);
        icsDeps.insert(ICSIdent::icsIdTimeWatcher);
    }
    ~ICSSmBillingCfgReader()
    { }

    SmBillingXCFG * rlseConfig(void)
    {
        icsDeps.exportDeps(icsCfg->deps);
        return icsCfg.release(); 
    }

};

} //smbill
} //inman
} //smsc
#endif /* __SMSC_INMAN_CONFIG_PARSING__ */

