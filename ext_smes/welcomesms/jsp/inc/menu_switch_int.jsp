<%@ page import="ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.novosoft.smsc.jsp.PageBean"%>
<%@ page import="mobi.eyeline.welcomesms.beans.WelcomeSMSBean"%>
<%
  boolean defaultError = false;
  switch(beanResult)
  {
    case WelcomeSMSBean.RESULT_OK:
    case WelcomeSMSBean.RESULT_ERROR:
      break;
    case WelcomeSMSBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case WelcomeSMSBean.RESULT_OPTIONS:
      response.sendRedirect("options.jsp");
      return;
    case WelcomeSMSBean.RESULT_NETWORKS:
      response.sendRedirect("networks.jsp");
      return;
    case WelcomeSMSBean.RESULT_ZONES:
      response.sendRedirect("zones.jsp");
      return;
    case WelcomeSMSBean.RESULT_UPLOAD:
      response.sendRedirect("upload.jsp");
      return;
    default:
      defaultError = true;
  }
%>