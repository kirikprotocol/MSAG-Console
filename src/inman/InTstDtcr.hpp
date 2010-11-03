/* ************************************************************************** *
 * INMan testing console: Contract determination protocol client
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TEST_DETECTOR__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TEST_DETECTOR__

#include "inman/InTstDefs.hpp"
#include "inman/interaction/msgdtcr/MsgContract.hpp"

namespace smsc  {
namespace inman {
namespace test {

using smsc::util::CAPConst;

using smsc::inman::test::TSTFacadeAC;
using smsc::inman::test::AbonentsDB;

using smsc::inman::interaction::INPSerializer;
using smsc::inman::interaction::INPCSAbntContract;
using smsc::inman::interaction::AbntContractRequest;
using smsc::inman::interaction::AbntContractResult;
using smsc::inman::interaction::AbntContractResHandlerITF;
using smsc::inman::interaction::CSAbntContractHdr_dlg;
using smsc::inman::interaction::SPckContractRequest;
using smsc::inman::interaction::SPckContractResult;


/* ************************************************************************** *
 * class DtcrDialog: INMan AbonentContract detection dialog params and result
 * ************************************************************************** */
class DtcrDialog {
public:
    enum DlgState { dIdle = 0, dRequested, dReported };

    DtcrDialog(TSTFacadeAC * use_mgr, unsigned dlg_id, unsigned ab_id, bool use_cache = true)
        : _mgr(use_mgr), dId(dlg_id), abId(ab_id), state(dIdle), useCache(use_cache)
    { 
        abInfo = AbonentsDB::getInstance()->getAbnInfo(ab_id);
    }

    unsigned getId(void) const { return dId; }
    void     setState(DlgState new_state) { state = new_state; }
    DlgState getState(void) const { return state; }
    AbonentInfo * abnInfo(void) const { return abInfo; }

    //composes and sends request
    bool sendRequest(unsigned num_bytes = 0) // 0 - forces sending whole packet
    {
        SPckContractRequest  pck;
        pck.Hdr().dlgId = dId;
        pck.Cmd().setSubscrNumber(abInfo->msIsdn.toString().c_str());
        pck.Cmd().allowCache(useCache);

        char tbuf[sizeof("%u bytes of ") + 4*3 + 1];
        int n = 0;
        if (num_bytes)
            n = snprintf(tbuf, sizeof(tbuf)-1, "%u bytes of ", num_bytes);
        tbuf[n] = 0;

        std::string msg;
        format(msg, "--> %sRequest[%u](%s) : cache(%s) ..", tbuf, dId,
               pck.Cmd().subscrAddr().c_str(), useCache ? "true" : "false");
        _mgr->Prompt(Logger::LEVEL_INFO, msg);
        if (_mgr->sendPckPart(&pck, num_bytes) > 0) {
            state = dRequested;
            return true;
        }
        msg.clear(); format(msg, "--> Request[%u]: sending failed!", dId);
        _mgr->Prompt(Logger::LEVEL_ERROR, msg);
        return false;
    }

    void onResult(AbntContractResult* res)
    {
        state = dReported;
        std::string msg;
        format(msg, "<-- Result[%u](%s): ", dId, abInfo->msIsdn.toString().c_str());
        if (res->errorCode()) {
            format(msg, "failed, error %u: %s", res->errorCode(), res->errorMsg());
        } else {
            format(msg, " %s, IMSI %s, %s, source(%s)", res->contractInfo().type2Str(),
                   res->contractInfo().imsiCStr(),
                   res->contractInfo().tdpSCF.toString().c_str(),
                   res->cacheUsed() ? "cache" : res->policyUsed());
            abInfo->setContractInfo(res->contractInfo());
        }
        _mgr->Prompt(Logger::LEVEL_INFO, msg);
    }

protected:
    TSTFacadeAC *   _mgr;
    unsigned        dId;
    unsigned        abId;   //abonent's id from AbonentsDB
    AbonentInfo *   abInfo;
    DlgState        state;
    bool            useCache;
};

/* ************************************************************************** *
 * class DtcrFacade: INMan AbonentContract detection dialogs controlling registry
 * ************************************************************************** */
class DtcrFacade : public TSTFacadeAC, AbntContractResHandlerITF {
protected:
    typedef std::map<unsigned, DtcrDialog*> DtcrDialogsMap;

    unsigned            _maxDlgId;
    DtcrDialogsMap      _Dialogs;
    AbonentsDB *        _abDB;

    inline unsigned nextDialogId(void) { return ++_maxDlgId; }

    DtcrDialog * findDialog(unsigned did)
    {
        MutexGuard grd(_sync);
        DtcrDialogsMap::const_iterator it = _Dialogs.find(did);
        return (it != _Dialogs.end()) ? (*it).second : NULL;
    }
    inline void eraseDialog(unsigned did)
    {
        MutexGuard grd(_sync);
        _Dialogs.erase(did);
    }

    DtcrDialog * initDialog(unsigned ab_id, bool use_cache = true)
    {
        MutexGuard grd(_sync);
        unsigned did = nextDialogId();
        std::auto_ptr<DtcrDialog> dlg(new DtcrDialog(this, did, ab_id, use_cache));
        if (dlg->abnInfo()) {
            _Dialogs.insert(DtcrDialogsMap::value_type(did, dlg.get()));
            return dlg.release();
        }
        Prompt(Logger::LEVEL_ERROR, format("unregistered abonent[%u]!", ab_id));
        return NULL;
    }

    bool numericIncrement(char * buf, unsigned len)
    {
        if (!len)
            return false;
        char nbuf[CAPConst::MAX_SMS_AddressValueLength + 1];
        unsigned i = CAPConst::MAX_SMS_AddressValueLength;
        nbuf[i] = 0;

        bool shift = true;
        do {
            nbuf[--i] = buf[--len];
            if ((nbuf[i] >= 0x30) && (nbuf[i] <= 0x39)) {
                if (shift) {
                    if (++(nbuf[i]) > 0x39)
                        nbuf[i] = 0x30;
                    else
                        shift = false;
                }
            } else
                return false;
        } while (len);
        strcpy(buf, nbuf + i);
        return true;
    }

public:
    DtcrFacade(ConnectSrv * conn_srv, Logger * use_log = NULL)
        : TSTFacadeAC(conn_srv, use_log), _maxDlgId(0)
        , _abDB(AbonentsDB::getInstance())
    {
        strcpy(_logId, "TFDtcr");
        INPSerializer::getInstance()->registerCmdSet(INPCSAbntContract::getInstance());
    }

    virtual ~DtcrFacade()
    { 
        MutexGuard grd(_sync);
        do_disconnect();
        if (!_Dialogs.empty()) {
            DtcrDialogsMap::const_iterator it;
            for (it = _Dialogs.begin(); it != _Dialogs.end(); it++) {
                DtcrDialog * dlg = (*it).second;
                delete dlg;
            }
            _Dialogs.clear();
        }
    }

    bool detectAbn(unsigned ab_id, bool use_cache = true)
    {
        DtcrDialog * dlg = initDialog(ab_id, use_cache);
        return dlg ? dlg->sendRequest() : false;
    }
    bool detectAbn(const TonNpiAddress & sbscr, bool use_cache = true)
    {
        unsigned ab_id = _abDB->searchAbn(sbscr);
        if (!ab_id) {
            AbonentInfo ab_info(&sbscr);
            ab_id = _abDB->setAbnInfo(ab_info);
        }
        return ab_id ? detectAbn(ab_id, use_cache) : false;
    }

    //detects contract for number of abonents from DB starting from given abId
    unsigned detectAbnMlt(unsigned ab_id, unsigned range, bool use_cache = true)
    {
        unsigned rval = 0;
        for (; range > 0; range--, ab_id++) {
            DtcrDialog * dlg = initDialog(ab_id, use_cache);
            if (!dlg)
                break;
            if (detectAbn(ab_id, use_cache))
                rval++;
        }
        return rval;
    }
    //detects contract for number of abonents starting from given ISDN address
    unsigned detectAbnMlt(const TonNpiAddress & sbscr, unsigned range, bool use_cache = true)
    {
        unsigned rval = 0;
        TonNpiAddress msIsdn = sbscr;
        for (; range > 0; range--) {
            unsigned ab_id = _abDB->searchAbn(msIsdn);
            if (!ab_id) {
                AbonentInfo ab_info(&msIsdn);
                ab_id = _abDB->setAbnInfo(ab_info);
            }
            if (detectAbn(ab_id, use_cache))
                rval++;
            if (!numericIncrement(msIsdn.signals, msIsdn.length))
                break;
        }
        return rval;
    }


    // -- ConnectListenerITF methods
    void onPacketReceived(Connect* conn, std::auto_ptr<SerializablePacketAC>& recv_cmd)
    /*throw(std::exception) */
    {
        //check service header
        INPPacketAC* pck = static_cast<INPPacketAC*>(recv_cmd.get());
        //check for header
        if (!pck->pHdr() || ((pck->pHdr())->Id() != INPCSAbntContract::HDR_DIALOG)) {
            std::string msg;
            format(msg, "ERR: unknown cmd header: %u", (pck->pHdr())->Id());
            Prompt(Logger::LEVEL_ERROR, msg);
        } else {
            if ((pck->pCmd())->Id() == INPCSAbntContract::ABNT_CONTRACT_RESULT_TAG) {
                AbntContractResult * cmd = static_cast<AbntContractResult*>(pck->pCmd());
                CSAbntContractHdr_dlg * hdr = static_cast<CSAbntContractHdr_dlg*>(pck->pHdr());

                bool goon = true;
                try { cmd->loadDataBuf(); }
                catch (SerializerException& exc) {
                    std::string msg;
                    format(msg, "ERR: corrupted cmd %u (dlgId: %u): %s",
                            cmd->Id(), hdr->Id(), exc.what());
                    Prompt(Logger::LEVEL_ERROR, msg);
                    goon = false;
                }
                if (goon)
                    onContractResult(cmd, hdr->dlgId);
            } else {
                std::string msg;
                format(msg, "ERR: unknown command recieved: %u", (pck->pCmd())->Id());
                Prompt(Logger::LEVEL_ERROR, msg);
            }
        }
    }

protected:
    // -- AbntContractResHandlerITF methods
    void onContractResult(AbntContractResult* res, uint32_t req_id)
    {
        DtcrDialog * pDlg = findDialog(req_id);
        if (!pDlg) {
            std::string msg;
            format(msg, "ERR: Dialog[%u] is inknown!", req_id);
            Prompt(Logger::LEVEL_ERROR, msg);
        } else {
            pDlg->onResult(res);
            eraseDialog(req_id);
        }
    }

};

} // namespace test
} // namespace inman
} // namespace smsc
#endif /* __SMSC_INMAN_TEST_DETECTOR__ */

