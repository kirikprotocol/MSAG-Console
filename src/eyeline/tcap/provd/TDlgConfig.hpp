/* ************************************************************************** *
 * TCProvider: TCAP dialogue initialization data structures.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_TDLG_CONFIG_HPP
#ident "@(#)$Id$"
#define __EYELINE_TCAP_PROVD_TDLG_CONFIG_HPP

#include "eyeline/tcap/TDlgIndHandlerIface.hpp"
#include "eyeline/tcap/TDlgReqHandlerIface.hpp"
#include "eyeline/tcap/provd/TransactionId.hpp"
#include "eyeline/corex/timers/TMWatcherDefs.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::corex::timers::TimeoutsMonitorIface;

using eyeline::sccp::SCCPAddress;

using eyeline::tcap::TCError;
using eyeline::tcap::TDlgIndHandlerIface;
using eyeline::tcap::TDlgReqHandlerIface;
using eyeline::tcap::TDlgTimeout;


//Common TCAP Dialogue initialization data.
struct TDlgConfig {
  asn1::EncodedOID        _acOid;
  TDlgIndHandlerIface *   _indHdl;
  TDlgReqHandlerIface *   _reqHdl;
  unsigned int            _scspLink;
  TimeoutsMonitorIface *  _tmoReg;
  TDlgTimeout             _dlgTmo;
  TDlgTimeout             _dfltInvTmo;
  SCCPAddress             _ownAddr;


  TDlgConfig() : _indHdl(0), _reqHdl(0), _scspLink((unsigned)(-1))
    , _tmoReg(0), _dlgTmo(0), _dfltInvTmo(0)
  { }
  //
  TDlgConfig(const asn1::EncodedOID & ac_oid,
             TDlgIndHandlerIface * dlg_ind_hdl,
             TDlgReqHandlerIface * dlg_req_hdl,
             unsigned int scsp_link_id,
             TimeoutsMonitorIface * tmo_reg,
             TDlgTimeout dlg_exp_tmo, TDlgTimeout inv_exp_tmo,
             const SCCPAddress & own_adr)
    : _acOid(ac_oid), _indHdl(dlg_ind_hdl), _reqHdl(dlg_req_hdl)
    , _scspLink(scsp_link_id), _tmoReg(tmo_reg)
    , _dlgTmo(dlg_exp_tmo), _dfltInvTmo(inv_exp_tmo), _ownAddr(own_adr)
  { }
  ~TDlgConfig()
  { }
};

//Locally initiated TCAP Dialogue initialization data.
struct TDlgLocalConfig : public TDlgConfig {
  const TransactionId::Origin_e   _origin;

  TDlgLocalConfig() : TDlgConfig(), _origin(TransactionId::orgLocal)
  { }
  //
  TDlgLocalConfig(const asn1::EncodedOID & ac_oid,
                  TDlgIndHandlerIface * dlg_ind_hdl,
                  TDlgReqHandlerIface * dlg_req_hdl,
                  unsigned int scsp_link_id,
                  TimeoutsMonitorIface * tmo_reg,
                  TDlgTimeout dlg_exp_tmo, TDlgTimeout inv_exp_tmo,
                  const SCCPAddress & own_adr)
    : TDlgConfig(ac_oid, dlg_ind_hdl, dlg_req_hdl, scsp_link_id, tmo_reg, dlg_exp_tmo, inv_exp_tmo, own_adr)
    , _origin(TransactionId::orgLocal)
  { }
  ~TDlgLocalConfig()
  { }
};

//Remotedly initiated TCAP Dialogue initialization data.
struct TDlgRemoteConfig : public TDlgConfig {
  const TransactionId::Origin_e   _origin;

  uint32_t      _rmtTrId;
  SCCPAddress   _rmtAddr;

  TDlgRemoteConfig() : TDlgConfig(), _origin(TransactionId::orgLocal), _rmtTrId(0)
  { }
  //
  TDlgRemoteConfig(const asn1::EncodedOID & ac_oid,
                   TDlgIndHandlerIface * dlg_ind_hdl,
                   TDlgReqHandlerIface * dlg_req_hdl,
                   unsigned int scsp_link_id,
                   TimeoutsMonitorIface * tmo_reg,
                   TDlgTimeout dlg_exp_tmo, TDlgTimeout inv_exp_tmo,
                   const SCCPAddress & own_adr,
                   const SCCPAddress & rmt_adr)
    : TDlgConfig(ac_oid, dlg_ind_hdl, dlg_req_hdl, scsp_link_id, tmo_reg, dlg_exp_tmo, inv_exp_tmo, own_adr)
    , _origin(TransactionId::orgLocal), _rmtTrId(0), _rmtAddr(rmt_adr)
  { }
  ~TDlgRemoteConfig()
  { }
};

}}}

#endif /* __EYELINE_TCAP_PROVD_TDLG_CONFIG_HPP */

