#ifndef __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_SSN_HPP__
# define __EYELINE_SS7NA_SUAGW_SUASTACK_MESSAGES_SSN_HPP__

namespace eyeline {
namespace ss7na {
namespace sua_gw {
namespace sua_stack {
namespace messages {

typedef enum { SSN_NOT_KNOWN = 0x00, SSN_SCCP_MANAGEMENT = 0x01, SSN_ISDN_USER_PART = 0x03,
               SSN_OMAP = 0x04, SSN_MAP = 0x05, SSN_HLR = 0x06, SSN_VLR = 0x07, SSN_MSC = 0x08,
               SSN_EIC = 0x09, SSN_AUC = 0x0A, SSN_ISDN_SUP_SERVICE = 0x0B, SSN_BROADBAND_ISDN = 0x0D,
               SSN_TC_TEST_RESPOPNDER = 0x0E, SSN_RESERVED = 0xFF} ssn_t;

class SSN {
public:
  static const char* getDescription(ssn_t ssn);
};

}}}}}

#endif
