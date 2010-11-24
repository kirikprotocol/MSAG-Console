/* ************************************************************************** *
 * INMan SM/USSD Billing dialog parameters
 * ************************************************************************** */
#ifndef __SMSC_INMAN_TEST_CAPSM_DLG_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_INMAN_TEST_CAPSM_DLG_HPP

#include "inman/storage/cdrutil.hpp"
#include "inman/tests/AbonentsDB.hpp"

namespace smsc  {
namespace inman {
namespace test {

using smsc::inman::cdr::CDRRecord;

/* ************************************************************************** *
 * class CapSmDialogCfg: CAP3Sms dialog params
 * ************************************************************************** */
class CapSmDialogCfg {
protected:
  AbonentsDB *        _abDB;
  unsigned            _orgAbId;  //originating abonent's id from AbonentsDB
  unsigned            _dstAbId;  //destination abonent's id from AbonentsDB
  bool                _ussdOp;
  uint32_t            _xsmsIds; //SMS Extra services id
  CDRRecord::ChargingPolicy   _chgPolicy;
  CDRRecord::ChargingType     _chgType;
  //flags
  bool                _forcedCDR;
  bool                _isOrgIMSI;
  bool                _isDstIMSI;
  bool                _isOrgMSC;
  bool                _isDstMSC;

  const AbonentInfo * _orgAb;
  const AbonentInfo * _dstAb;

  IMSIString          _orgIMSI; //replaces abonent's IMSI from DB
  IMSIString          _dstIMSI; //replaces abonent's IMSI from DB
  TonNpiAddress       _orgMSC; //replaces abonent's location MSC address from DB
  TonNpiAddress       _dstMSC; //replaces abonent's location MSC address from DB

public:
  //NOTE: PRE_ABONENTS_NUM >= 2
  explicit CapSmDialogCfg(AbonentsDB & use_db)
    : _abDB(&use_db), _orgAbId(1), _dstAbId(2), _ussdOp(false)
    , _xsmsIds(0), _chgPolicy(CDRRecord::ON_DELIVERY), _chgType(CDRRecord::MO_Charge)
    , _forcedCDR(false), _isOrgIMSI(true), _isDstIMSI(true)
    , _isOrgMSC(true), _isDstMSC(true)
  {
    if (!(_orgAb = _abDB->getAbnInfo(_orgAbId)))
      _orgAbId = 0;
    if (!(_dstAb = _abDB->getAbnInfo(_dstAbId)))
      _dstAbId = 0;
  }
  ~CapSmDialogCfg()
  { }

  void printConfig(FILE * use_stream = stdout) const;

  const AbonentInfo * getOrgAbnt(void) const { return _orgAb; }
  const AbonentInfo * getDstAbnt(void) const { return _dstAb; }

  const char * getOrgIMSI(void) const;
  const char * getDstIMSI(void) const;
  //
  const TonNpiAddress * getOrgMSC(void) const;
  const TonNpiAddress * getDstMSC(void) const;

  uint32_t getSmsXIds(void) const { return _xsmsIds; }

  bool  isChargePolicy(CDRRecord::ChargingPolicy chg_pol) const
  {
    return (_chgPolicy == chg_pol);
  }
  bool  isChargeType(CDRRecord::ChargingType chg_type) const
  {
    return (_chgType == chg_type);
  }
  bool  isUssdOp(void) const    { return _ussdOp; }
  bool  isForcedCDR(void) const { return _forcedCDR; }


  void  setChargePolicy(CDRRecord::ChargingPolicy chg_pol)  { _chgPolicy = chg_pol; }
  void  setChargeType(CDRRecord::ChargingType chg_type)     { _chgType = chg_type; }

  void  setUssdOp(bool is_ussd = true)        { _ussdOp = is_ussd; }
  void  setForcedCDR(bool forced_cdr = true)  { _forcedCDR = forced_cdr; }
  void  setSmsXIds(uint32_t srv_ids)          { _xsmsIds = srv_ids; }

  void  setOrgIMSI(bool imsi_on, const IMSIString * use_imsi = NULL);
  void  setDstIMSI(bool imsi_on, const IMSIString * use_imsi = NULL);
  //
  void  setOrgMSC(bool msc_on, const TonNpiAddress * use_msc = NULL);
  void  setDstMSC(bool msc_on, const TonNpiAddress * use_msc = NULL);
  //
  bool  setOrgAbnt(unsigned ab_id)
  {
    if (!(_orgAb = _abDB->getAbnInfo(_orgAbId = ab_id)))
      _orgAbId = 0;
    return (_orgAb != 0);
  }
  //
  bool  setDstAbnt(unsigned ab_id)
  {
    if (!(_dstAb = _abDB->getAbnInfo(_dstAbId = ab_id)))
      _dstAbId = 0;
    return (_dstAb != 0);
  }

  //Sets abonent next to _orgAbnt in AbonentsDB as _orgAbnt
  void nextOrgAbnt(void) { setOrgAbnt(_abDB->nextId(_orgAbId)); }
  //Sets abonent next to _dstAbnt in AbonentsDB as _dstAbnt
  void nextDstAbnt(void) { setDstAbnt(_abDB->nextId(_dstAbId)); }
};

class CapSmDialog : public CapSmDialogCfg {
public:
  enum DlgState_e { dIdle = 0, dCharged = 1, dApproved, dReported };

protected:
  unsigned int  _dId;
  DlgState_e    _state;
  bool          _batchMode; //autorespond with _dlvrRes
  uint32_t      _dlvrRes;   //failure by default

public:
  CapSmDialog(AbonentsDB & use_db, unsigned int dlg_id, bool batch_mode = false,
           uint32_t dlvr_res = 1016)
    : CapSmDialogCfg(use_db), _dId(dlg_id), _state(dIdle), _batchMode(batch_mode)
    , _dlvrRes(dlvr_res)
  { }
  CapSmDialog(const CapSmDialogCfg & use_cfg, unsigned int dlg_id,  bool batch_mode = false,
           uint32_t dlvr_res = 1016)
      : CapSmDialogCfg(use_cfg), _dId(dlg_id), _state(dIdle), _batchMode(batch_mode)
      , _dlvrRes(dlvr_res)
  { }
  ~CapSmDialog()
  { }

  void    setState(DlgState_e new_state) { _state = new_state; }
  CapSmDialogCfg & getConfig(void) { return *this; }

  bool        isState(DlgState_e cmp_state) const { return (_state == cmp_state); }
  DlgState_e  getState(void) const { return _state; }
  uint32_t    getDlvrResult(void) const { return _dlvrRes; }
  bool        isBatchMode(void) const { return _batchMode; }

  const CapSmDialogCfg & getConfig(void) const { return *this; }
};

} // namespace test
} // namespace inman
} // namespace smsc
#endif /* __SMSC_INMAN_TEST_CAPSM_DLG_HPP */

