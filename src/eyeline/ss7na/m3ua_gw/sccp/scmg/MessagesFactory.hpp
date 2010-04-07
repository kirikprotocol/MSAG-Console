#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGESFACTORY_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_SCMG_MESSAGESFACTORY_HPP__

# include "eyeline/utilx/Singleton.hpp"
# include "eyeline/utilx/MessagesFactory.hpp"
# include "eyeline/ss7na/m3ua_gw/sccp/scmg/messages/SCMGMessage.hpp"

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace scmg {

const unsigned int MAX_NUM_OF_MESSAGES = 16;

class MessagesFactory : public utilx::MessagesFactoryTmpl<messages::SCMGMessage, MAX_NUM_OF_MESSAGES>,
                        public utilx::Singleton<MessagesFactory> {
public:
};

}}}}}

#endif
