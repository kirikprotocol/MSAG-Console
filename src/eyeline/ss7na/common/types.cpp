#include "types.hpp"

namespace eyeline {
namespace ss7na {
namespace common {

const char* returnCauseValueDesc[] = {
                                  "no translation for an address of such nature",
                                  "no translation for this specific address",
                                  "subsystem congestion",
                                  "subsystem failure",
                                  "unequipped user",
                                  "MTP failure",
                                  "network congestion",
                                  "unqualified",
                                  "error in message transport",
                                  "error in local processing",
                                  "destination cannot perform reassembly",
                                  "SCCP failure",
                                  "hop counter violation",
                                  "segmentation not supported",
                                  "segmentation failure"
};

}}}
