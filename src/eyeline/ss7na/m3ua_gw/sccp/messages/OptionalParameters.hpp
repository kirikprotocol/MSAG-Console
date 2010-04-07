#ifndef __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_OPTIONALPARAMETERS_HPP__
# define __EYELINE_SS7NA_M3UAGW_SCCP_MESSAGES_OPTIONALPARAMETERS_HPP__

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {
namespace sccp {
namespace messages {

enum param_codes_e {
  END_OPTIONAL_PARAM_TAG = 0, SEGMENTATION_PARAM_TAG = 0x10, IMPORTANCE_PARAM_TAG = 0x12
};

enum length_ind_values_e {
  SEGMENTATION_VALUE_SIZE = 4, IMPORTANCE_VALUE_SIZE = 1
};

}}}}}

#endif
