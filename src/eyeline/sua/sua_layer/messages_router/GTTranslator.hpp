#ifndef __EYELINE_SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATOR_HPP__
# define __EYELINE_SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATOR_HPP__

# include <logger/Logger.h>
# include <eyeline/utilx/Singleton.hpp>
# include <eyeline/sua/communication/LinkId.hpp>
# include <eyeline/sua/communication/sua_messages/SuaTLV.hpp>

namespace eyeline {
namespace sua {
namespace sua_layer {
namespace messages_router {

class GTTranslator : public utilx::Singleton<GTTranslator> {
public:
  GTTranslator();
  communication::LinkId translate(const communication::sua_messages::TLV_DestinationAddress& address);
private:
  GTTranslator(const GTTranslator& rhs);
  GTTranslator& operator=(const GTTranslator& rhs);
  smsc::logger::Logger* _logger;
};

}}}}

#endif
