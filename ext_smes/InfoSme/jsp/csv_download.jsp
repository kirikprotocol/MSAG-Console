<%@ page import="ru.novosoft.smsc.jsp.PageBean, ru.novosoft.smsc.infosme.beans.InfoSmeBean,
                 ru.novosoft.smsc.jsp.SMSCJspException,
                 ru.novosoft.smsc.jsp.SMSCErrors,
                 java.text.DateFormat,
                 java.text.SimpleDateFormat,
                 java.util.Date"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.TasksStatistics" /><jsp:setProperty name="bean" property="*"/><%

    ru.novosoft.smsc.infosme.beans.TasksStatistics sbean = bean;
    int result = sbean.process(request);
    if (result == PageBean.RESULT_OK)
        result = sbean.exportStatistics(out);

    switch(result)
    {
    case PageBean.RESULT_ERROR:
    case InfoSmeBean.RESULT_DLSTAT:
        response.sendRedirect("stat.jsp?csv=true");
        return;
    case InfoSmeBean.RESULT_DONE:
        response.sendRedirect("deliveries.jsp");
        return;
    default:
        break;
    }

%><%@page contentType="text/csv; charset=windows-1251" %><%
    DateFormat dateFormat = new SimpleDateFormat("'stat'_yyyy-MM-dd_HH-mm-ss.'csv'");
    response.setHeader("Content-Disposition", "attachment; filename="+dateFormat.format(new Date()));
%>