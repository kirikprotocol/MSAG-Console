<%@ page import="ru.novosoft.smsc.infosme.beans.InfoSmeBean,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.PageBean"%>

<%@ include file="menu_switch_int.jsp"%><%
  if (defaultError)
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction,
                                             SMSCJspException.ERROR_CLASS_ERROR));
%>