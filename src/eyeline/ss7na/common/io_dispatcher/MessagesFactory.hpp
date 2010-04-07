#ifndef __EYELINE_SS7NA_COMMON_IODISPATCHER_MESSAGESFACTORY_HPP__
# define __EYELINE_SS7NA_COMMON_IODISPATCHER_MESSAGESFACTORY_HPP__

# include "eyeline/ss7na/common/Message.hpp"
# include "eyeline/utilx/MessagesFactory.hpp"
# include "eyeline/utilx/Singleton.hpp"

namespace eyeline {
namespace ss7na {
namespace common {
namespace io_dispatcher {

const unsigned int MAX_NUM_OF_PROTOCOLS = 2; // m3ua/sua, libsccp
const unsigned int MAX_NUM_OF_MESSAGES = 512;

class MessagesFactory : public utilx::MessagesFactoryTmpl<Message, MAX_NUM_OF_MESSAGES, MAX_NUM_OF_PROTOCOLS>,
                        public utilx::Singleton<MessagesFactory> {
public:
};

}}}}

#endif
