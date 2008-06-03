#ifndef __SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATOR_HPP__
# define __SUA_SUALAYER_MESSAGESROUTER_GTTRANSLATOR_HPP__

# include <logger/Logger.h>
# include <sua/utilx/Singleton.hpp>
# include <sua/communication/LinkId.hpp>
# include <sua/communication/sua_messages/SuaTLV.hpp>

namespace messages_router {

class GTTranslator : public utilx::Singleton<GTTranslator> {
public:
  GTTranslator();
  communication::LinkId translate(const sua_messages::TLV_DestinationAddress& address);
private:
  GTTranslator(const GTTranslator& rhs);
  GTTranslator& operator=(const GTTranslator& rhs);
  smsc::logger::Logger* _logger;
};

}

#endif
