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

class DOMPrintErrorHandler : public DOMErrorHandler
{
public:
  DOMPrintErrorHandler(){};
  ~DOMPrintErrorHandler(){};
  bool handleError(const DOMError& dom_error) {
    StrX msg(dom_error.getMessage());
    if (dom_error.getSeverity() == DOMError::DOM_SEVERITY_WARNING) {
      __trace2__("DOM Print Warning Message : %s", msg.localForm());
      return true;
    }
    if (dom_error.getSeverity() == DOMError::DOM_SEVERITY_ERROR) {
      __trace2__("DOM Print Error Message : %s", msg.localForm());
      return true;
    }    
    __trace2__("DOM Print Fatal Message : %s", msg.localForm());
    return true;
  }      
  void resetErrors(){};
private:
    /* Unimplemented constructors and operators */
  DOMPrintErrorHandler(const DOMErrorHandler&);
  void operator=(const DOMErrorHandler&);

};  

}//mms
}//transport
}//scag

#endif
