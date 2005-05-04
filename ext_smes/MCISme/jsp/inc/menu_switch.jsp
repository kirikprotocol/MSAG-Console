<%@ page import="ru.novosoft.smsc.mcisme.beans.MCISmeBean,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.PageBean"%>
<%
  switch(beanResult)
  {
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
      break;
    case MCISmeBean.RESULT_APPLY:
      response.sendRedirect("index.jsp");
      return;
    case MCISmeBean.RESULT_STAT:
      response.sendRedirect("stat.jsp");
      return;
    case MCISmeBean.RESULT_STATUSES:
      response.sendRedirect("statuses.jsp");
      return;
    case MCISmeBean.RESULT_OPTIONS:
      response.sendRedirect("options.jsp");
      return;
    case MCISmeBean.RESULT_CIRCUITS:
      response.sendRedirect("circuits.jsp");
      return;
    case MCISmeBean.RESULT_TEMPLATES:
      response.sendRedirect("templates.jsp");
      return;
    case MCISmeBean.RESULT_RULES:
      response.sendRedirect("rules.jsp");
      return;
    case MCISmeBean.RESULT_DRIVERS:
      response.sendRedirect("drivers.jsp");
      return;
    case MCISmeBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    default:
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction, SMSCJspException.ERROR_CLASS_ERROR));
  }
%>