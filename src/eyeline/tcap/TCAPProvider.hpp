#ifndef __EYELINE_TCAP_TCAPPROVIDER_HPP__
# ident "@(#)$Id$"
# define __EYELINE_TCAP_TCAPPROVIDER_HPP__

# include <sys/time.h>

# include "core/threads/ThreadPool.hpp"
# include "util/config/ConfigView.h"

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/asn1/EncodedOID.hpp"
# include "eyeline/tcap/TCAPIface.hpp"
# include "eyeline/tcap/TDlgHandlerIfaceFactory.hpp"

namespace eyeline {
namespace tcap {

class TCAPProvider {
public:
  static void initialize(smsc::util::config::ConfigView& tc_provider_cfg_section);
  static TCAPIface* getTCAPIface();
  static void shutdown();

  static void registerAppCTX(const EncodedOID& ctx,
                             TDlgHandlerIfaceFactory* indHndlrsFactory,
                             unsigned int dialogueTimeout);
};

}}

#endif
