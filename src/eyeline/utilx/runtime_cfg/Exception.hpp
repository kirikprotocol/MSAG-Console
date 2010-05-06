#ifndef __EYELINE_UTILX_RUNTIMECFG_EXCEPTION_HPP__
# define __EYELINE_UTILX_RUNTIMECFG_EXCEPTION_HPP__

# include "util/Exception.hpp"

namespace eyeline {
namespace utilx {
namespace runtime_cfg {

class InconsistentConfigCommandException : public smsc::util::Exception {
public:
  InconsistentConfigCommandException(const std::string& msg_for_user)
    : Exception(), _errorDiagnosticMsg(msg_for_user), _popupCurrentCommandInterpreter(false)
  {}

  InconsistentConfigCommandException(const std::string& msg_for_user, const char * fmt, ...)
    : Exception(), _errorDiagnosticMsg(msg_for_user), _popupCurrentCommandInterpreter(false)
  {
    SMSC_UTIL_EX_FILL(fmt);
  }

  virtual ~InconsistentConfigCommandException() throw () {}

  const std::string& getErrorDignostic() const {
    return _errorDiagnosticMsg;
  }

  void forcePopUpCurrentCommandInterpreter() { _popupCurrentCommandInterpreter = true; }
  bool needPopUpCurrentCommandInterpreter() const { return _popupCurrentCommandInterpreter; }
private:
  std::string _errorDiagnosticMsg;
  bool _popupCurrentCommandInterpreter;
};

}}}

#endif
