#ifndef __EYELINE_SS7NA_SUAGW_TYPES_HPP__
# define __EYELINE_SS7NA_SUAGW_TYPES_HPP__

# include <sys/types.h>

namespace eyeline {
namespace ss7na {
namespace sua_gw {

enum protocol_class_e {
  PROTOCOL_SUA = 0, PROTOCOL_LIBSCCP = 1
};

}}}

#endif
