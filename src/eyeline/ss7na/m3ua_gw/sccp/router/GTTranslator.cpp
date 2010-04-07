#include "eyeline/ss7na/common/types.hpp"
#include "eyeline/ss7na/common/Exception.hpp"
#include "GTTranslator.hpp"
#include "GTTranslationTablesRegistry.hpp"
#include "GTTranslationTable.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace router {

GTTranslator::GTTranslator()
  : _logger(smsc::logger::Logger::getInstance("sccp_rout"))
{}

GTTranslationResult
GTTranslator::translate(const eyeline::sccp::SCCPAddress& address)
{
  char gtAddressFamilyPrefix[32];

  smsc_log_debug(_logger, "GTTranslator::translate::: do gt translation for address=[%s]",
                address.toString().c_str());
  const eyeline::sccp::GlobalTitle& globalTitle = address.getGT();
  const eyeline::sccp::GlobalTitle::GTIndicator& gtInd = globalTitle.getGTIndicator();
  // form address string in .GTI.TON.NPI.NATUREOFADDR.digits format
  snprintf(gtAddressFamilyPrefix, sizeof(gtAddressFamilyPrefix), ".%d.%d.%d.%d",
           gtInd.kind, gtInd.getTrTypeValue(), gtInd.getNPi(), gtInd.getNoAValue());
  smsc_log_debug(_logger, "GTTranslator::translate::: try get GTTranslationTable for gtAddrFamilyPrefix='%s'",
                 gtAddressFamilyPrefix);
  GTTranslationTable* translationTable = GTTranslationTablesRegistry::getInstance().getGTTranslationTable(gtAddressFamilyPrefix);
  if ( !translationTable )
    throw common::TranslationFailure(common::NO_ADDR_TRANSLATION_FOR_SUCH_NATURE, "GTTranslator::translate::: translation table not found for address=[%s]", address.toString().c_str());

  return translationTable->translate(address);
}

}}}}}
