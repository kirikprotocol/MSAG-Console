#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/tcap/provd/AppCtxSMRegistry.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

using smsc::core::synchronization::MutexGuard;

bool AppCtxSMRegistry::registerDlgHandlerFactory(const asn1::EncodedOID & ctx_oid,
                                                 TDlgHandlerFactoryIface * dlg_hndls_factory,
                                                 TDlgTimeout dlg_exp_tmo/* = 0*/)
{
  MutexGuard  grd(_sync);
  if ( _registeredAppCtxs.find(ctx_oid) == _registeredAppCtxs.end() ) {
    _registeredAppCtxs.insert(std::make_pair(ctx_oid, RegistryEntry(dlg_hndls_factory, dlg_exp_tmo)));
    return true;
  }
  return false;
}

TDlgHandlerInfo
  AppCtxSMRegistry::getDlgHandler(const asn1::EncodedOID & ctx_oid,
                                       ros::LocalOpCode init_opcode/* = 0*/) const /*throw()*/
{
  MutexGuard  grd(_sync);
  registered_app_ctxs_t::const_iterator iter = _registeredAppCtxs.find(ctx_oid);
  return (iter == _registeredAppCtxs.end()) ? 
            TDlgHandlerInfo() : iter->second.createDlgHandler(init_opcode);
}

void AppCtxSMRegistry::unregisterDlgHandlerFactory(const asn1::EncodedOID & ctx_oid) /*throw()*/
{
  MutexGuard  grd(_sync);
  registered_app_ctxs_t::iterator iter = _registeredAppCtxs.find(ctx_oid);
  if ( iter != _registeredAppCtxs.end() ) {
    _registeredAppCtxs.erase(iter);
  }
}

}}}
