#ifndef __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_CODECUTILITY_HPP__
# define __EYELINE_LOADBALANCER_PROTOCOLS_SMPP_CODECUTILITY_HPP__

# include <sys/types.h>
# include "eyeline/load_balancer/io_subsystem/Packet.hpp"

namespace eyeline {
namespace load_balancer {
namespace protocols {
namespace smpp {

size_t
extractVariableCOctetString(const io_subsystem::Packet* packet, size_t offset,
                            char* value, size_t maxValueSize);

size_t
extractFixedCOctetString(const io_subsystem::Packet* packet, size_t offset,
                         char* value, size_t expectedValueSize);

size_t
addCOctetString(io_subsystem::Packet* packet, const char* value, size_t maxValueSize);

}}}}

#endif
