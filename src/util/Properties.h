#ifndef SMSC_UTIL_PROPERTIES_HEADER
#define SMSC_UTIL_PROPERTIES_HEADER

#include <stdio.h>

#include "core/buffers/Hash.hpp"
#include "util/Exception.hpp"
#include <memory>


namespace smsc {
namespace util {

typedef const char * PropertiesValue;

class Properties : public smsc::core::buffers::Hash<PropertiesValue>
{
public:
  Properties();
  Properties(const char * const filename) throw (smsc::util::Exception);
  virtual ~Properties();

  std::auto_ptr<Properties> getSection(const char * const prefix) const;

protected:
  Properties(const Properties & copy) throw (smsc::util::Exception);
};

}
}

#endif //SMSC_UTIL_PROPERTIES_HEADER
