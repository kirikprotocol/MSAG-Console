#ifndef __EYELINE_SS7NA_M3UAGW_TYPES_HPP__
# define __EYELINE_SS7NA_M3UAGW_TYPES_HPP__

# include <sys/types.h>

namespace eyeline {
namespace ss7na {
namespace m3ua_gw {

enum mtp_user_identity_e {
  Reserved_0 = 0, Reserved_1, Reserved_2,
  SCCP = 3, TUP = 4, ISUP = 5, Reserved_6, Reserved_7, Reserved_8,
  Broadband_ISUP = 9, Satellite_ISUP = 10, Reserved_11,
  AAL_type2_Signalling = 12, BICC = 13, GatewayControlProtocol = 14,
  Reserved_15
};

enum network_indicator_e {
  INTERNATIONAL_NETWORK = 0, RESERVE_INTERNATIONAL_NETWORK = 1,
  NATIONAL_NETWORK = 2, RESERVE_NATIONAL_NETWORK = 3
};

enum mtp_status_cause_e {
  Unknown = 0, SignallingNetworkCongestion = 1 , UnequippedRemoteUser = 2,
  InaccessibleRemoteUser = 3
};

enum mtp_congestion_level_e {
  NoCongestion = 0, CongestionLevel1 = 1, CongestionLevel2 = 2, CongestionLevel3 = 3
};

enum protocol_class_e {
  PROTOCOL_M3UA = 0, PROTOCOL_LIBSCCP = 1
};

}}}

#endif
