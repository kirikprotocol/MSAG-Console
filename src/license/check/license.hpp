#ifndef __SMSC_LICENSE_CHECK_LICENSE_HPP__
#define __SMSC_LICENSE_CHECK_LICENSE_HPP__

#include "core/buffers/Hash.hpp"
#include <string>

namespace smsc{
namespace license{
namespace check{
using smsc::core::buffers::Hash;
bool CheckLicense(const char* lf,const char* sig,Hash<std::string>& lic,const char* keys[],size_t keysCount);
}//namespace check
}//namespace license
}//namespace smsc

#endif
