#include "AppCtxSMRegistry.hpp"
#include "eyeline/utilx/Exception.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

void
AppCtxSMRegistry::registerDialogueHandlerFactory(const asn1::EncodedOID& ctx,
                                                 TDlgHandlerIfaceFactory* indHndlrsFactory,
                                                 unsigned int dialogueTimeout)
{
  if ( _registeredAppCtxs.find(ctx) == _registeredAppCtxs.end() )
    _registeredAppCtxs.insert(std::make_pair(ctx, RegistryEntry_impl(indHndlrsFactory, dialogueTimeout)));
}

AppCtxSMRegistry::RegistryEntry
AppCtxSMRegistry::getDialogueHandler(const asn1::EncodedOID& ctx)
{
  registered_app_ctxs_t::iterator iter = _registeredAppCtxs.find(ctx);
  if ( iter == _registeredAppCtxs.end() )
    throw utilx::RegistryKeyNotFound("AppCtxSMRegistry::getDialogueHandler::: no entry for key='%s'", ctx.asnValue().c_str());
  return RegistryEntry(iter->second.dialogueHandlerFactory->createTDlgHandlerIface(),
                       iter->second.dialogueTimeout);
}

void
AppCtxSMRegistry::unregisterDialogueHandlerFactory(const asn1::EncodedOID& ctx)
{
  registered_app_ctxs_t::iterator iter = _registeredAppCtxs.find(ctx);
  if ( iter != _registeredAppCtxs.end() ) {
    delete iter->second.dialogueHandlerFactory;
    _registeredAppCtxs.erase(iter);
  }
}

}}}
