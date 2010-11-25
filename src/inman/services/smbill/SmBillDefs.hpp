/* ************************************************************************* *
 * SMS/USSD messages billing service types and helpers definitions
 * ************************************************************************* */
#ifndef __INMAN_SMBILLING_DEFS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_SMBILLING_DEFS_HPP

#include <map>

#include "util/TonNpiAddress.hpp"
#include "inman/inap/TCUsrDefs.hpp"
#include "inman/services/ICSrvIDs.hpp"

namespace smsc    {
namespace inman   {
namespace smbill  {

using smsc::util::TonNpiAddress;
using smsc::inman::inap::TCAPUsr_CFG;

//_smsXSrvs bits to mask, i.e. exclude from processing logic
#define SMSX_NOCHARGE_SRV  0x80000000   //SMSX Extra service free of charge
#define SMSX_INCHARGE_SRV  0x40000000   //Charge SMS via IN point despite of
                                        //billMode setting
#define SMSX_RESERVED_MASK  (SMSX_NOCHARGE_SRV|SMSX_INCHARGE_SRV)

struct XSmsService {
    std::string     name;
    uint32_t        mask;
    uint32_t        svcCode; //actually it's a serviceId
    TonNpiAddress   adr;
    bool            chargeBearer;

    explicit XSmsService(const char * use_name = NULL, uint32_t use_mask = 0)
        : name(use_name), mask(use_mask), svcCode(0), chargeBearer(false)
    { }
};
typedef std::map<uint32_t, XSmsService> SmsXServiceMap;

extern const char * const _CDRmodes[];
extern const char * const _BILLmodes[];
extern const char * const _MSGtypes[];

struct ChargeParm {
    enum MSG_TYPE  { msgUnknown = 0, msgSMS = 1, msgUSSD = 2, msgXSMS = 3 };
    enum BILL_MODE { billOFF = 0, bill2CDR = 1, bill2IN = 2 };
    enum CDR_MODE { cdrNONE = 0, cdrBILLMODE = 1, cdrALL = 2};
    enum ContractReqMode { reqOnDemand = 0, reqAlways };

    static const char * cdrModeStr(CDR_MODE mode_id)
    {
        return _CDRmodes[mode_id];
    }
    static const char * msgTypeStr(MSG_TYPE msg_id)
    {
        return _MSGtypes[msg_id];
    }
    static const char * billModeStr(BILL_MODE mode_id)
    {
        return _BILLmodes[mode_id];
    }
};

//Billing modes priority setting: 
// primary billing mode and secondary, that is used in case
// of failure while processing first one
typedef std::pair<ChargeParm::BILL_MODE, ChargeParm::BILL_MODE> BModesPrio;

class BillModes {
protected:
    typedef std::map<ChargeParm::MSG_TYPE, BModesPrio> BModesMAP;
    BModesMAP   bmMap;
    BModesMAP::const_iterator  dflt;
public:
    BillModes()
    {
        std::pair<BModesMAP::iterator, bool> res =
            bmMap.insert(BModesMAP::value_type(ChargeParm::msgUnknown,
                 BModesPrio(ChargeParm::billOFF, ChargeParm::billOFF)));
        dflt = res.first;
    }

    ~BillModes()
    { }

    bool assign(ChargeParm::MSG_TYPE msg_type, ChargeParm::BILL_MODE mode_1st,
                        ChargeParm::BILL_MODE mode_2nd = ChargeParm::billOFF)
    {
        std::pair<BModesMAP::iterator, bool> res =
        bmMap.insert(BModesMAP::value_type(msg_type, BModesPrio(mode_1st, mode_2nd)));
        return res.second;
    }

    bool allOFF(void) const
    {
        BModesMAP::const_iterator it = bmMap.begin();
        for (; it != bmMap.end(); ++it) {
            if ((it->second).first != ChargeParm::billOFF)
                return false;
        }
        return true;
    }

    bool useIN(void) const
    {
        BModesMAP::const_iterator it = bmMap.begin();
        for (; it != bmMap.end(); ++it) {
            if ((it->second).first == ChargeParm::bill2IN)
                return true;
        }
        return false;
    }

    bool isAssigned(ChargeParm::MSG_TYPE msg_type) const
    {
        BModesMAP::const_iterator it = bmMap.find(msg_type);
        return (it == bmMap.end()) ? false : true;
    }

    const BModesPrio * modeFor(ChargeParm::MSG_TYPE msg_type) const
    {
        BModesMAP::const_iterator it = bmMap.find(msg_type);
        return (it == bmMap.end()) ? &(dflt->second) : &(it->second);
    }
};

//core SM billing parameters
struct SmBillParams : public ChargeParm {
    BillModes       mo_billMode;    //Billing modes for MobileOriginating messages
    BillModes       mt_billMode;    //Billing modes for MobileTerminating messages
    ContractReqMode cntrReq;
    CDR_MODE        cdrMode;
    uint16_t        maxBilling;     //maximum number of Billings per connect
    bool            useCache;       //use abonents contract data cache
    uint32_t        cacheTmo;       //abonent cache data expiration timeout in secs
    std::string     cdrDir;         //location to store CDR files
    long            cdrInterval;    //rolling interval for CDR files
    std::auto_ptr<SmsXServiceMap>
                    smsXMap;        //SMS Extra services configuration
    std::auto_ptr<TCAPUsr_CFG>
                    capSms;         //CAP3Sms (SM charging) configuration

    SmBillParams()
        : cntrReq(reqOnDemand), cdrMode(cdrBILLMODE), maxBilling(0)
        , cacheTmo(0), cdrInterval(0)
    { }

    bool needIAProvider(void) const
    {
      return ((cntrReq == ChargeParm::reqAlways)
              || mo_billMode.useIN() || mt_billMode.useIN());
    }
};

//Billing service configuration parameters (from config.xml)
struct SmBillingXCFG {
    std::auto_ptr<SmBillParams> prm; //core SM billing parameters
    std::string     policyNm;       //name of default AbonenPolicy
    uint16_t        maxTimeout;     //maximum timeout for TCP operations,
                                    //billing aborts on its expiration
    uint16_t        abtTimeout;     //maximum timeout on abonent type requets,
                                    //(HLR & DB interaction), on expiration billing
                                    //continues in CDR mode 
    ICSIdsSet       deps;           //ICServices Billing depends on

    SmBillingXCFG() : prm(new SmBillParams())
        , maxTimeout(0), abtTimeout(0)
    { }
};

} //smbill
} //inman
} //smsc
#endif /* __INMAN_SMBILLING_DEFS_HPP */

