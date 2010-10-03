#include "MEPrivateExtensionList.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

void
MEPrivateExtensionList::setValue(const PrivateExtensionList& private_ext_list)
{
  unsigned idx= 0;
  for(PrivateExtensionList::const_iterator iter= private_ext_list.begin(), end_iter= private_ext_list.end();
      iter != end_iter; ++iter ) {
    if (idx == maxNumOfPrivateExtensions)
      throw smsc::util::Exception("MEPrivateExtensionList::MEPrivateExtensionList::: too long PrivateExtensionList");
    ++idx;
    addValue(*iter);
  }
}

}}}}
