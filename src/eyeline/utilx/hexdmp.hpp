#ifndef __EYELINECOM_UTILX_HEXDMP_HPP__
# define __EYELINECOM_UTILX_HEXDMP_HPP__

# include <sys/types.h>
# include <string>

namespace eyeline {
namespace utilx {

std::string
hexdmp(const uchar_t* buf, size_t bufSz);

}}

#endif
