#ifndef __SMSC_LICENSE_CHECK_LICENSE_HPP__
#define __SMSC_LICENSE_CHECK_LICENSE_HPP__

#include "core/buffers/Hash.hpp"

namespace smsc{
namespace license{
namespace check{
using smsc::core::buffers::Hash;
bool CheckLicense(const char* lf,const char* sig,Hash<string>& lic);
}//namespace check
}//namespace license
}//namespace smsc

#endif
