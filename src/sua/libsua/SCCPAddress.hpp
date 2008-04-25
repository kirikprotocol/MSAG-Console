#ifndef __SUA_LIBSUA_SCCPADDRESS_HPP__
# define __SUA_LIBSUA_SCCPADDRESS_HPP__ 1

# include <sua/communication/sua_messages/GlobalTitle.hpp>

namespace libsua {

struct SCCPAddress
{
  explicit SCCPAddress(const sua_messages::GlobalTitle& gt);

  SCCPAddress(const sua_messages::GlobalTitle& gt, uint8_t ssn);

  sua_messages::GlobalTitle gt;

  uint8_t ssn;

  uint32_t fieldsMask;

  typedef enum { SET_GT = 0x01, SET_SSN = 0x02 } field_mask_t;
};

}

#endif /* SCCPADDRESS_H_HEADER_INCLUDED_B7F159FE */
