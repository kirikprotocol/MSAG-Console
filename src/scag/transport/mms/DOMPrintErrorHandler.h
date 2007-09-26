#ifndef __SCAG_DOM_PRINT_ERROR_HANDLER_H__
#define __SCAG_DOM_PRINT_ERROR_HANDLER_H__

#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/dom/DOMWriter.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>


#include "logger/Logger.h"
#include "sms/sms.h"

#include "util.h"

namespace scag {
namespace transport {
namespace mms {

XERCES_CPP_NAMESPACE_USE
using smsc::logger::Logger;

class DOMPrintErrorHandler : public DOMErrorHandler
{
public:
  DOMPrintErrorHandler(){
    logger = Logger::getInstance("mms.dom");
  };
  ~DOMPrintErrorHandler(){};
  bool handleError(const DOMError& dom_error) {
    StrX msg(dom_error.getMessage());
    if (dom_error.getSeverity() == DOMError::DOM_SEVERITY_WARNING) {
      smsc_log_warn(logger, "Warning : %s", msg.localForm());
      return true;
    }
    if (dom_error.getSeverity() == DOMError::DOM_SEVERITY_ERROR) {
      smsc_log_error(logger, "Error : %s", msg.localForm());
      return true;
    }    
    smsc_log_error(logger, "Fatal Error : %s", msg.localForm());
    return true;
  }      
  void resetErrors(){};
private:
    /* Unimplemented constructors and operators */
  DOMPrintErrorHandler(const DOMErrorHandler&);
  void operator=(const DOMErrorHandler&);

  Logger* logger;
};  

}//mms
}//transport
}//scag

#endif
