#ifndef SMSC_UTIL_FINDCONFIGFILE_HEADER
#include <admin/AdminException.h>

namespace smsc {
namespace util {
extern const char * const findConfigFile(const char * const file_to_find) throw (smsc::admin::AdminException);
}
}
#endif //SMSC_UTIL_FINDCONFIGFILE_HEADER

