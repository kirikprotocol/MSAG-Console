<%@ page import="ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors"%>

<%@ include file="menu_switch_int.jsp"%><%
  if (defaultError)
      errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction,
                                             SMSCJspException.ERROR_CLASS_ERROR));
%>