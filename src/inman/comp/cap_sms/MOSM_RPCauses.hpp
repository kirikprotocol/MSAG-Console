/* ************************************************************************* *
 * 3GPP TS 24.011 Annex E-2: RP-cause definition mobile originating SM-transfer.
 * ************************************************************************* */
#ifndef _SMSC_INMAN_COMP_CAPSMS_RP_CAUSES_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _SMSC_INMAN_COMP_CAPSMS_RP_CAUSES_HPP

#include "util/URCdb/URCRegistry.hpp"

namespace smsc {
namespace inman {
namespace comp {

using smsc::util::URCSpacePTR;

struct MOSM_RPCause {
    enum Codes {
        unassignedNumber = 1,   /*
This cause indicates that the destination requested by the Mobile Station
cannot be reached because, although the number is in a valid format, it is
not currently assigned (allocated). */
        operatorBarring = 8,    /*
This cause indicates that the MS has tried to send a mobile originating short
message when the MS's network operator or service provider has forbidden such
transactions. */
        callBarred = 10,        /*
This cause indicates that the outgoing call barred service applies to the
short message service for the called destination. */
        transferReject = 21,    /*
This cause indicates that the equipment sending this cause does not wish to
accept this short message, although it could have accepted the short message
since the equipment sending this cause is neither busy nor incompatible. */
        daOutOfService = 27,    /*
This cause indicates that the destination indicated by the Mobile Station
cannot be reached because the interface to the destination is not functioning
correctly. The term "not functioning correctly" indicates that a signalling
message was unable to be delivered to the remote user; e.g., a physical
layer or data link layer failure at the remote user, user equipment off-line,
etc. */
        unidentifiedSubscriber = 28, /*
This cause indicates that the subscriber is not registered in the PLMN
(i.e. IMSI not known). */
        facilityRejected = 29,      /*
This cause indicates that the facility requested by the Mobile Station
is not supported by the PLMN.  */
        unknownSubscriber = 30,     /*
This cause indicates that the subscriber is not registered in the HLR
(i.e. IMSI or directory number is not allocated to a subscriber). */
        networkOff = 38,            /*
This cause indicates that the network is not functioning correctly and
that the condition is likely to last a relatively long period of time;
e.g., immediately reattempting the short message transfer is not likely
to be successful. */
        temporaryFailure = 41,      /*
This cause indicates that the network is not functioning correctly and
that the condition is not likely to last a long period of time; e.g.,
the Mobile Station may wish to try another short message transfer
attempt almost immediately. */
        congestion = 42,            /*
This cause indicates that the short message service cannot be serviced
because of high traffic. */
        unavailableResource = 47,   /*
This cause is used to report a resource unavailable event only when no
other cause applies. */
        facilityNotSubscribed = 50, /*
This cause indicates that the requested short message service could
not be provided by the network because the user has not completed the
necessary administrative arrangements with its supporting networks. */
        facilityNotImplemented = 69,/*
This cause indicates that the network is unable to provide the requested
short message service. */
        invSMReferece = 81,         /*
This cause indicates that the equipment sending this cause has received
a message with a short message reference which is not currently in use
on the MS-network interface. */
        invMessage = 95,            /*
This cause is used to report an invalid message event only when no other
cause in the invalid message class applies. */
        invInformation = 96,        /*
This cause indicates that the equipment sending this cause has received
a message where a mandatory information element is missing and/or has a
content error (the two cases are indistinguishable). */
        invMessageType = 97,        /*
This cause indicates that the equipment sending this cause has received
a message with a message type it does not recognize either because this
is a message not defined or defined but not implemented by the equipment
sending this cause. */
        incompatibleMessage = 98,   /*
This cause indicates that the equipment sending this cause has received
a message such that the procedures do not indicate that this is a permissible
message to receive while in the short message transfer state. */
        unimplementedIE = 99,       /*
This cause indicates that the equipment sending this cause has received
a message which includes information elements not recognized because the
information element identifier is not defined or it is defined but not
implemented by the equipment sending the cause. 
However, the information element is not required to be present in the
message in order for the equipment sending the cause to process the message. */
        protocolErr = 111,          /*
This cause is used to report a protocol error event only when no other cause applies. */
        unspecified = 127           /*
This cause indicates that there has been interworking with a network which
does not provide causes for actions it takes; thus, the precise cause for a
message which is being send cannot be ascertained. */
    };
};
extern const char * rc2Txt_MOSM_RPCause(uint32_t rp_cause);
extern URCSpacePTR  _RCS_MOSM_RPCause;

#define FDECL_rc2Txt_MOSM_RPCause()    \
const char * rc2Txt_MOSM_RPCause(uint32_t rp_cause) { \
    switch (rp_cause) { \
    case MOSM_RPCause::unassignedNumber: return "Unassigned (unallocated) number"; \
    case MOSM_RPCause::operatorBarring: return "Operator determined barring"; \
    case MOSM_RPCause::callBarred: return "Call barred"; \
    case MOSM_RPCause::transferReject: return "Short message transfer rejected"; \
    case MOSM_RPCause::daOutOfService: return "Destination out of service"; \
    case MOSM_RPCause::unidentifiedSubscriber: return "Unidentified subscriber"; \
    case MOSM_RPCause::facilityRejected: return "Facility rejected"; \
    case MOSM_RPCause::unknownSubscriber: return "Unknown subscriber"; \
    case MOSM_RPCause::networkOff: return "Network out of order"; \
    case MOSM_RPCause::temporaryFailure: return "Temporary failure"; \
    case MOSM_RPCause::congestion: return "Congestion"; \
    case MOSM_RPCause::unavailableResource: return "Resources unavailable, unspecified"; \
    case MOSM_RPCause::facilityNotSubscribed: return "Requested facility not subscribed"; \
    case MOSM_RPCause::facilityNotImplemented: return "Requested facility not implemented"; \
    case MOSM_RPCause::invSMReferece: return "Invalid short message transfer reference value"; \
    case MOSM_RPCause::invMessage: return "Invalid message, unspecified"; \
    case MOSM_RPCause::invInformation: return "Invalid mandatory information"; \
    case MOSM_RPCause::invMessageType: return "Message type non-existent or not implemented"; \
    case MOSM_RPCause::incompatibleMessage: return "Message not compatible with short message protocol state"; \
    case MOSM_RPCause::unimplementedIE: return "Information element non-existent or not implemented"; \
    case MOSM_RPCause::protocolErr: return "Protocol error, unspecified"; \
    case MOSM_RPCause::unspecified: return "Interworking, unspecified"; \
    default:; } \
    return "unknown MO SM RPCause"; \
}

#define ODECL_RCS_MOSM_RPCause() FDECL_rc2Txt_MOSM_RPCause() \
URCSpacePTR  _RCS_MOSM_RPCause("moSmRPCause", 1, 127, rc2Txt_MOSM_RPCause)


/* ------------------------------------------------------------------------- *
 * Macro for all return code spaces global initialization
 * ------------------------------------------------------------------------- */
#define _RCS_MOSM_RPCauseINIT()  ODECL_RCS_MOSM_RPCause()

}//namespace comp
}//namespace inman
}//namespace smsc

#define _RCS_MOSM_RPCauseGET() smsc::inman::comp::_RCS_MOSM_RPCause.get()

#endif /* _SMSC_INMAN_COMP_CAPSMS_RP_CAUSES_HPP */

