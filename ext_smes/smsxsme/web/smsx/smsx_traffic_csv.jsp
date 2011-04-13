<jsp:useBean id="bean" scope="request" class="ru.sibinco.smsx.stats.beans.SmsxTrafficBean" />
<jsp:setProperty name="bean" property="*"/>
<%
    bean.process(request);
    bean.download(request, out);
%><%@page contentType="text/csv; charset=windows-1251"%><%
    response.setHeader("Content-Disposition", "attachment; filename=traffic.csv");
%>