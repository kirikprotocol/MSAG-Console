#ifndef __EYELINE_SUA_COMMUNICATION_TYPES_HPP__
# define __EYELINE_SUA_COMMUNICATION_TYPES_HPP__

namespace eyeline {
namespace sua {
namespace communication {

typedef enum { RETURN_CAUSE = 0x01, REFUSAL_CAUSE = 0x02, RELEASE_CAUSE = 0x03, RESET_CAUSE = 0x04,
               ERROR_CAUSE = 0x05 } return_cause_type_t;

typedef enum { NO_ADDR_TRANSLATION_FOR_SUCH_NATURE = 0, NO_ADDR_TRANSLATION_FOR_SPECIFIC_ADDR = 0x01,
               SUBSYSTEM_CONGESTION = 0x02,SUBSYSTEM_FAILURE = 0x03, UNEQUIPPED_USER = 0x04,
               MTP_FAILURE = 0x05, NETWORK_CONGESTION = 0x06, UNQUALIFIED = 0x07,
               ERROR_IN_MESSAGE_TRANSPORT = 0x08, ERROR_IN_LOCAL_PROCESSING = 0x09,
               DESTINATION_CANNOT_PERFORM_REASSEMBLY = 0x0A, SCCP_FAILURE = 0x0B, HOP_COUNTER_VIOLATION = 0x0C,
               SEGMENTATION_NOT_SUPPORTED = 0x0D, SEGMENTATION_FAILURE = 0x0E } return_cause_value_t;

}}}

#endif
