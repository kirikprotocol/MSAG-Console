#ifndef __EYELINE_SS7NA_SUAGW_MESSAGESROUTER_GTTRANSLATOR_HPP__
# define __EYELINE_SS7NA_SUAGW_MESSAGESROUTER_GTTRANSLATOR_HPP__

# include "logger/Logger.h"
# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/ss7na/common/LinkId.hpp"
# include "eyeline/ss7na/sua_gw/sua_stack/messages/SuaTLV.hpp"

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace messages_router {

class GTTranslator : public utilx::Singleton<GTTranslator> {
public:
  GTTranslator();
  common::LinkId translate(const sua_stack::messages::TLV_DestinationAddress& address);
private:
  GTTranslator(const GTTranslator& rhs);
  GTTranslator& operator=(const GTTranslator& rhs);
  smsc::logger::Logger* _logger;
};

}}}}

#endif
