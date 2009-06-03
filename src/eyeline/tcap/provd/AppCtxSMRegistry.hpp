#ifndef __EYELINE_TCAP_PROVD_APPCTXSMREGISTRY_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_PROVD_APPCTXSMREGISTRY_HPP__

# include <map>

# include "logger/Logger.h"
# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/tcap/TDlgHandlerIface.hpp"
# include "eyeline/tcap/TDlgHandlerIfaceFactory.hpp"

namespace eyeline {
namespace tcap {
namespace provd {

class AppCtxSMRegistry : public utilx::Singleton<AppCtxSMRegistry> {
public:
  AppCtxSMRegistry()
    : _logger(smsc::logger::Logger::getInstance("tcap.provd"))
  {}

  struct RegistryEntry {
    RegistryEntry(TDlgHandlerIface* aDialogueHandler,
                  unsigned int aDialogueTimeout)
      : dialogueHandler(aDialogueHandler), dialogueTimeout(aDialogueTimeout)
    {}

    TDlgHandlerIface* dialogueHandler;
    unsigned int dialogueTimeout;
  };
  // Not MT-safe
  void registerDialogueHandlerFactory(const asn1::EncodedOID& ctx,
                                      TDlgHandlerIfaceFactory* indHndlrsFactory,
                                      unsigned int dialogueTimeout);

  RegistryEntry getDialogueHandler(const asn1::EncodedOID& ctx);

  // Not MT-safe
  void unregisterDialogueHandlerFactory(const asn1::EncodedOID& ctx);
private:
  smsc::logger::Logger* _logger;
  struct RegistryEntry_impl {
    RegistryEntry_impl(TDlgHandlerIfaceFactory* aDialogueHandlerFactory,
                       unsigned int aDialogueTimeout)
      : dialogueHandlerFactory(aDialogueHandlerFactory), dialogueTimeout(aDialogueTimeout)
    {}

    TDlgHandlerIfaceFactory* dialogueHandlerFactory;
    unsigned int dialogueTimeout;
  };
  typedef std::map<asn1::EncodedOID, RegistryEntry_impl> registered_app_ctxs_t;
  registered_app_ctxs_t _registeredAppCtxs;
};

}}}

#endif
