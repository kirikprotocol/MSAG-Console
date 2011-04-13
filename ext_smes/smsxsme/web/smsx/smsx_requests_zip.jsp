<%@page contentType="application/octet-stream; charset=windows-1251"%><%
    response.setHeader("Content-Disposition", "attachment; filename=results.zip");
%>
<jsp:useBean id="bean" scope="request" class="ru.sibinco.smsx.stats.beans.SmsxRequestsBean" />
<jsp:setProperty name="bean" property="*"/>
<%
    bean.process(request);
    bean.download(request, response);
%>