<%@ page import="ru.novosoft.smsc.jsp.PageBean, ru.novosoft.smsc.infosme.beans.InfoSmeBean,
                 java.text.DateFormat, java.text.SimpleDateFormat, java.util.Date"%>
<jsp:useBean id="bean" scope="page" class="ru.novosoft.smsc.infosme.beans.TasksStatistics"/><jsp:setProperty name="bean" property="*"/><%

    bean.process(request);
    bean.exportStatistics(request, out);    

%><%@page contentType="text/csv; charset=windows-1251"%><%
    DateFormat dateFormat = new SimpleDateFormat("'stat'_yyyy-MM-dd_HH-mm-ss.'csv'");
    response.setHeader("Content-Disposition", "attachment; filename="+dateFormat.format(new Date()));
%>