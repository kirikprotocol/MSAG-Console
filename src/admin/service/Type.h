#ifndef SMSC_ADMIN_SERVICE_TYPE
#define SMSC_ADMIN_SERVICE_TYPE

#include <string>

namespace smsc {
namespace admin {
namespace service {

enum Type {StringType, LongType, BooleanType, undefined};

Type cstr2Type(const char * const typestr);
const char * const type2cstr(Type type);

}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_TYPE
