/* ************************************************************************** *
 * Types used in definition of ROS PDUs
  * ************************************************************************** */
#ifndef __ROS_TYPES_DEFS_HPP
#ident "@(#)$Id$"
#define __ROS_TYPES_DEFS_HPP

#include <inttypes.h>

namespace eyeline {
namespace ros {

typedef uint8_t InvokeId;
typedef uint8_t LocalOpCode;

struct LinkedId {
  bool      _present;
  InvokeId  _invId;

  LinkedId()
    : _present(false), _invId(0)
  { }
  LinkedId(uint8_t inv_id)
    : _present(true), _invId(inv_id)
  { }
};

} //ros
} //eyeline

#endif /* __ROS_TYPES_DEFS_HPP */

