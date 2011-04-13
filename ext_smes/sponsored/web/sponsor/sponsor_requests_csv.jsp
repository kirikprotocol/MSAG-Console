<jsp:useBean id="bean" scope="request" class="ru.sibinco.sponsored.stats.beans.SponsoredRequestsBean" />
<jsp:setProperty name="bean" property="*"/>
<%@page contentType="text/csv; charset=windows-1251"%><%
    response.setHeader("Content-Disposition", "attachment; filename=sponsor_results.csv");
%>
<%
    bean.process(request);
    bean.download(out);
%>