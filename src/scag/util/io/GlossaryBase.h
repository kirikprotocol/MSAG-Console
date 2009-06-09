#ifndef __SCAG_UTIL_STORAGE_GLOSSARYBASE_H__
#define __SCAG_UTIL_STORAGE_GLOSSARYBASE_H__

#include <string>

namespace scag { namespace util { namespace io {

class GlossaryBase {
public:
  GlossaryBase() {};
  virtual ~GlossaryBase() {};
  virtual int Add(const std::string& key) = 0;
  virtual int GetValueByKey(const std::string& key) = 0;
  virtual int GetKeyByValue(int value, std::string& key) = 0;

public:
  static const int SUCCESS		      = 0;
  static const int OPEN_ERROR	      = 1;
  static const int LOAD_ERROR	      = 2;
  static const int ALREADY_OPENED     = 3;
  static const int NO_VALUE		      = -1;
  static const int NO_KEY			  = -2;
  static const int KEY_ALREADY_EXISTS = -3;	
  static const int GLOSSARY_CLOSED	  = -4;

};

}//io
}//util
}//scag

namespace scag2 {
namespace util {
namespace io {
using scag::util::io::GlossaryBase;
}
}
}

#endif

