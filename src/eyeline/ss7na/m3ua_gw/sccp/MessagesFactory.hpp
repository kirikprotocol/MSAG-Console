#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGESFACTORY_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGESFACTORY_HPP__

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/MessagesFactory.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/messages/SCCPMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {

const unsigned int MAX_NUM_OF_MESSAGES = 32;

class MessagesFactory : public utilx::MessagesFactoryTmpl<messages::SCCPMessage, MAX_NUM_OF_MESSAGES>,
                        public utilx::Singleton<MessagesFactory> {
public:
};

}}}}

#endif
