#include "TCAPProvider.hpp"
#include "provd/TCAPLayer.hpp"

namespace eyeline {
namespace tcap {

void
TCAPProvider::initialize(smsc::util::config::ConfigView& tc_provider_cfg_section)
{
  provd::TCAPLayer::getInstance().initialize(tc_provider_cfg_section);
}

TCAPIface*
TCAPProvider::getTCAPIface()
{
  return provd::TCAPLayer::getInstance().getTCAPIface();
}

void
TCAPProvider::shutdown()
{
  provd::TCAPLayer::getInstance().shutdown();
}

void
TCAPProvider::registerAppCTX(const EncodedOID& ctx,
                             TDlgHandlerIfaceFactory* indHndlrsFactory,
                             unsigned int dialogueTimeout)
{
  provd::TCAPLayer::getInstance().registerAppCTX(ctx, indHndlrsFactory, dialogueTimeout);
}

}}
