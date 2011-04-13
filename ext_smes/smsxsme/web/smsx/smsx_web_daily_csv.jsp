<jsp:useBean id="bean" scope="request" class="ru.sibinco.smsx.stats.beans.SmsxDailyBean" />
<jsp:setProperty name="bean" property="*"/>
<%@page contentType="text/csv; charset=windows-1251"%><%
    response.setHeader("Content-Disposition", "attachment; filename=web-daily.csv");
    bean.process(request);
    bean.download(request, out);
%>