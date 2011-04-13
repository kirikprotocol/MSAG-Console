<jsp:useBean id="bean" scope="request" class="ru.sibinco.smsx.stats.beans.SmsxRegionsBean" />
<jsp:setProperty name="bean" property="*"/>
<%
    bean.process(request);
    bean.download(request, out);
%><%@page contentType="text/csv; charset=windows-1251"%><%
    response.setHeader("Content-Disposition", "attachment; filename=web-regions.csv");
%>