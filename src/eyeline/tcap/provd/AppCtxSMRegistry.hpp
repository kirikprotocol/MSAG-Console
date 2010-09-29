/* ************************************************************************** *
 * TCProvider: TCAP dialogue handler factories registry
 * ************************************************************************** */
#ifndef __EYELINE_TCAP_PROVD_APPCTXSMREGISTRY_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif /* __GNUC__ */
#define __EYELINE_TCAP_PROVD_APPCTXSMREGISTRY_HPP__

#include <map>

#include "core/synchronization/Mutex.hpp"

#include "eyeline/asn1/EncodedOID.hpp"
#include "eyeline/tcap/TDlgHandlerFactoryIface.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using eyeline::tcap::TDlgTimeout;
using eyeline::tcap::TDlgIndHandlerIface;
using eyeline::tcap::TDlgHandlerFactoryIface;

struct TDlgHandlerInfo {
  TDlgIndHandlerIface * dlgHdl;
  TDlgTimeout           dlgTimeout;

  TDlgHandlerInfo() : dlgHdl(0), dlgTimeout(0)
  { }
  TDlgHandlerInfo(TDlgIndHandlerIface * use_dlg_handler, TDlgTimeout use_dlg_timeout = 0)
    : dlgHdl(use_dlg_handler), dlgTimeout(use_dlg_timeout)
  { }

  bool empty(void) const { return dlgHdl == 0; }
};

class AppCtxSMRegistry {
public:
  AppCtxSMRegistry()
  { }
  ~AppCtxSMRegistry()
  { }

  //Returns false in case of app_ctx has factory already registered
  bool registerDlgHandlerFactory(const asn1::EncodedOID & ctx_oid,
                                  TDlgHandlerFactoryIface * dlg_hndls_factory,
                                  TDlgTimeout dlg_exp_tmo = 0) /*throw()*/;
  //Returns empty TDlgHandlerInfo if app_ctx is unknown
  //NOTE: init_opcode is mandatory only in case of _ac_contextless_ops !!!
  TDlgHandlerInfo getDlgHandler(const asn1::EncodedOID & ctx_oid,
                              ros::LocalOpCode init_opcode = 0) const /*throw()*/;
  //
  void unregisterDlgHandlerFactory(const asn1::EncodedOID & ctx_oid);

private:
  struct RegistryEntry {
    TDlgHandlerFactoryIface * dlgHdlFactory;
    unsigned int              dlgTimeout;

    RegistryEntry() : dlgHdlFactory(0), dlgTimeout(0)
    { }
    RegistryEntry(TDlgHandlerFactoryIface * use_dlg_hdl_factory,
                                            unsigned int use_dlg_timeout)
      : dlgHdlFactory(use_dlg_hdl_factory), dlgTimeout(use_dlg_timeout)
    { }
    ~RegistryEntry()
    { }

    //NOTE: init_opcode is mandatory only in case of _ac_contextless_ops !!!
    TDlgHandlerInfo createDlgHandler(ros::LocalOpCode init_opcode = 0) const
    {
      return TDlgHandlerInfo(dlgHdlFactory->createTDlgHandler(init_opcode), dlgTimeout);
    }
  };
  typedef std::map<asn1::EncodedOID, RegistryEntry> registered_app_ctxs_t;

  mutable smsc::core::synchronization::Mutex  _sync;
  registered_app_ctxs_t                       _registeredAppCtxs;
};

}}}

#endif /* __EYELINE_TCAP_PROVD_APPCTXSMREGISTRY_HPP__ */

