<%@ page import="ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.smsc.emailsme.beans.SmeBean"
%><%
  switch (beanResult) {
    case PageBean.RESULT_OK:
      break;
    case PageBean.RESULT_DONE:
      response.sendRedirect("index.jsp");
      return;
    case PageBean.RESULT_ERROR:
      break;
    case SmeBean.RESULT_INDEX:
      response.sendRedirect("index.jsp");
      return;
    case SmeBean.RESULT_OPTIONS:
      response.sendRedirect("options.jsp");
      return;
    case SmeBean.RESULT_DRIVERS:
      response.sendRedirect("drivers.jsp");
      return;
    case SmeBean.RESULT_PROFILES:
      response.sendRedirect("profiles.jsp");
      return;
    default:
      break;
  }
%>