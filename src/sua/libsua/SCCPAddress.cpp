#include "SCCPAddress.hpp"

namespace libsua {

SCCPAddress::SCCPAddress(const sua_messages::GlobalTitle& aGt)
  : gt(aGt), ssn(0), fieldsMask(SET_GT)
{}

SCCPAddress::SCCPAddress(const sua_messages::GlobalTitle& aGt, uint8_t aSsn)
  : gt(aGt), ssn(aSsn), fieldsMask(SET_GT|SET_SSN)
{}

}
