<%@ page import="ru.novosoft.smsc.infosme.beans.InfoSmeBean,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors"%>
<%
  switch(beanResult)
  {
    case InfoSmeBean.RESULT_APPLY:
      response.sendRedirect("index.jsp");
      return;
    case InfoSmeBean.RESULT_STAT:
      response.sendRedirect("stat.jsp");
      return;
    case InfoSmeBean.RESULT_OPTIONS:
      response.sendRedirect("options.jsp");
      return;
    case InfoSmeBean.RESULT_DRIVERS:
      response.sendRedirect("drivers.jsp");
      return;
    case InfoSmeBean.RESULT_PROVIDERS:
      response.sendRedirect("providers.jsp");
      return;
    case InfoSmeBean.RESULT_TASKS:
      response.sendRedirect("tasks.jsp");
      return;
    case InfoSmeBean.RESULT_SHEDULES:
      response.sendRedirect("shedules.jsp");
      return;
    case InfoSmeBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case InfoSmeBean.RESULT_OK:
      STATUS.append("Ok");
      break;
    case InfoSmeBean.RESULT_ERROR:
      STATUS.append("<span class=CF00>Error</span>");
      break;
    default:
      STATUS.append("<span class=CF00>Error "+beanResult+"</span>");
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>