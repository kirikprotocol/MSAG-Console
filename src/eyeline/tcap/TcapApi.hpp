/* ************************************************************************** *
 * TCAP API: structured TCAP dialogue API.
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_TDLG_API_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_TDLG_API_HPP

#include "eyeline/tcap/TDlgIndHandlerIface.hpp"
#include "eyeline/tcap/TDlgReqHandlerIface.hpp"

namespace eyeline {
namespace tcap {

struct TDlgProperties {
  TDialogueId             dlgId;  //unique dialogue Id.
  TDlgIndHandlerIface *   indHdl; //dialogue indications handler
  TDlgReqHandlerIface *   reqHdl; //dialogue requests handler

  TDlgProperties() : dlgId(0), indHdl(0), reqHdl(0)
  { }
  TDlgProperties(TDialogueId use_id, TDlgIndHandlerIface * ind_hdl, TDlgReqHandlerIface * req_hdl)
    : dlgId(use_id), indHdl(ind_hdl), reqHdl(req_hdl)
  { }

  bool empty(void) const { return (!indHdl && !reqHdl); }
};

}}

#endif /* __EYELINE_TCAP_TDLG_API_HPP */

