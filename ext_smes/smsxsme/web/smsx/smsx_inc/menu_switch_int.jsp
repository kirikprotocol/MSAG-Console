<%@ page import="ru.novosoft.smsc.jsp.PageBean,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 ru.sibinco.smsx.stats.beans.StatsBean" %>
<%
  boolean defaultError = false;
  switch(beanResult)
  {
    case PageBean.RESULT_OK:
    case PageBean.RESULT_ERROR:
      break;
    case StatsBean.VIEW_SMSX_USERS:
      response.sendRedirect("smsx_users.jsp?requestId="+requestId);
      return;
    case StatsBean.VIEW_WEB_DAILY:
      response.sendRedirect("smsx_web_daily.jsp?requestId="+requestId);
      return;
    case StatsBean.VIEW_WEB_REGIONS:
      response.sendRedirect("smsx_web_regions.jsp?requestId="+requestId);
      return;
    case StatsBean.VIEW_TRAFFIC:
      response.sendRedirect("smsx_traffic.jsp?requestId="+requestId);
      return;
    default:
      defaultError = true;
  }
%>