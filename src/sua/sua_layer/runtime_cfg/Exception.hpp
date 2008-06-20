#ifndef __SUA_SUALAYER_RUNTIMECFG_EXCEPTION_HPP__
# define __SUA_SUALAYER_RUNTIMECFG_EXCEPTION_HPP__

# include <util/Exception.hpp>
# include <sua/communication/types.hpp>

namespace runtime_cfg {

class InconsistentConfigCommandException : public smsc::util::Exception {
public:
  InconsistentConfigCommandException(const std::string& errDiagnosticMsg, const char * fmt, ...)
    : Exception(), _errorDiagnosticMsg(errDiagnosticMsg), _popupCurrentCommandInterpreter(false)
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

}

#endif
