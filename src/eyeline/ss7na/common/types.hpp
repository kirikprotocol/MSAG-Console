#ifndef __EYELINE_SS7NA_COMMON_TYPES_HPP__
# define __EYELINE_SS7NA_COMMON_TYPES_HPP__

# include <sys/types.h>

namespace eyeline {
namespace ss7na {
namespace common {

typedef enum { RETURN_CAUSE = 0x01, REFUSAL_CAUSE = 0x02, RELEASE_CAUSE = 0x03, RESET_CAUSE = 0x04,
               ERROR_CAUSE = 0x05 } return_cause_type_t;

typedef enum { NO_ADDR_TRANSLATION_FOR_SUCH_NATURE = 0, NO_ADDR_TRANSLATION_FOR_SPECIFIC_ADDR = 0x01,
               SUBSYSTEM_CONGESTION = 0x02, SUBSYSTEM_FAILURE = 0x03, UNEQUIPPED_USER = 0x04,
               MTP_FAILURE = 0x05, NETWORK_CONGESTION = 0x06, UNQUALIFIED = 0x07,
               ERROR_IN_MESSAGE_TRANSPORT = 0x08, ERROR_IN_LOCAL_PROCESSING = 0x09,
               DESTINATION_CANNOT_PERFORM_REASSEMBLY = 0x0A, SCCP_FAILURE = 0x0B, HOP_COUNTER_VIOLATION = 0x0C,
               SEGMENTATION_NOT_SUPPORTED = 0x0D, SEGMENTATION_FAILURE = 0x0E, MAX_CAUSE_VALUE = 0x0E } return_cause_value_t;

extern const char* returnCauseValueDesc[];

typedef uint32_t point_code_t;

enum service_indicator_e {
  SNM = 0, SNTM = 1, SCCP = 3, TUP = 4, ISUP = 5, DUP_CALL = 6, DUP_CANCEL = 7,
  MTP_TestUserPart = 8, BroadBandISDNUserPart = 9, SatelliteISDNUserPart = 10
};

}}}

#endif
