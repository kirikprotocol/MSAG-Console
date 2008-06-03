#include <sua/utilx/Exception.hpp>
#include "GTTranslator.hpp"
#include "GTTranslationTablesRegistry.hpp"
#include "GTTranslationTable.hpp"

namespace messages_router {

GTTranslator::GTTranslator()
  : _logger(smsc::logger::Logger::getInstance("msg_rout"))
{}

communication::LinkId
GTTranslator::translate(const sua_messages::TLV_DestinationAddress& address)
{
  char gtAddressFamilyPrefix[32];

  smsc_log_info(_logger, "GTTranslator::translate::: do gt translation for address=[%s]", address.toString().c_str());
  const sua_messages::GlobalTitle& globalTitle = address.getGlobalTitle().getGlobalTitleValue();
  snprintf(gtAddressFamilyPrefix, sizeof(gtAddressFamilyPrefix), ".%d.%d.%d.%d", globalTitle.getGTI(), globalTitle.getTranslationType(), globalTitle.getNumberingPlan(), globalTitle.getNatureOfAddress());

  GTTranslationTable* translationTable = GTTranslationTablesRegistry::getInstance().getGTTranslationTable(gtAddressFamilyPrefix);
  if ( !translationTable )
    throw utilx::TranslationFailure(communication::NO_ADDR_TRANSLATION_FOR_SUCH_NATURE, "GTTranslator::translate::: translation table not found for address=[%s]", address.toString().c_str());

  return translationTable->translate(address);
}

}
