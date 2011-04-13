<jsp:useBean id="bean" scope="request" class="ru.sibinco.sponsored.stats.beans.SponsorResultsBean" />
<jsp:setProperty name="bean" property="*"/>
<%
    bean.process(request);
    bean.download(out);
%><%@page contentType="text/csv; charset=windows-1251"%><%
    response.setHeader("Content-Disposition", "attachment; filename=sponsor_results.csv");
%>