#ifndef SMSC_ADMIN_SERVICE_PARAMETER
#define SMSC_ADMIN_SERVICE_PARAMETER

#include <admin/service/Type.h>
#include <core/buffers/Hash.hpp>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::util::cStringCopy;
using smsc::core::buffers::Hash;

class Parameter
{
private:
  char * name;
  Type type;

public:
  Parameter(const char * const parameterName, Type parameterType)
  {
    name = cStringCopy(parameterName);
    type = parameterType;
  }

  Parameter(const Parameter &copy)
  {
    name = cStringCopy(copy.name);
    type = copy.type;
  }

  Parameter()
  {
    name = 0;
    type = undefined;
  }

  Parameter & operator =(const Parameter & copy)
  {
    if (name != 0)
      delete [] name;
    name = cStringCopy(copy.name);
    type = copy.type;
    return *this;
  }

  virtual ~Parameter()
  {
    if (name != 0)
      delete [] name;
  }

  const char * const getName() const
  {
    return name;
  }

  const Type getType() const
  {
    return type;
  }

  bool operator ==(const Parameter & param) const
  {
    return name != 0 && param.name != 0 && type == param.type && strcmp(name, param.name) == 0;
  }

  bool operator !=(const Parameter & param) const
  {
    return !(*this == param);
  }
};

typedef smsc::core::buffers::Hash<Parameter> Parameters;

}
}
}
#endif

