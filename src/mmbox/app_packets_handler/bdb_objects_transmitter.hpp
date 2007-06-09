#ifndef __BDB_AGENT_BDB_OBJECT_TRANSMITTER_HPP__
# define __BDB_AGENT_BDB_OBJECT_TRANSMITTER_HPP__ 1

# include <mmbox/app_protocol/ApplicationPackets.hpp>
# include <util/comm_comp/CommunicationComponent.hpp>
# include <util/comm_comp/RawTransportPacket.hpp>

namespace mmbox {
namespace app_pck_handler {

typedef smsc::util::comm_comp::ObjectReaderWriter<smsc::util::comm_comp::RawTransportPacket, mmbox::app_protocol::RequestApplicationPacket, mmbox::app_protocol::ResponseApplicationPacket> bdb_objects_transmitter_t;

}}

#endif
