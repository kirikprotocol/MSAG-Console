<%@ include file="/common/header.jsp"%>
<%
String host = request.getParameter("host");
String service = request.getParameter("service");
serviceManager.refreshService(service);
if (serviceManager.getServiceInfo(service).getPid() != 0)
{%>
<h2>Shutdown service <i><%=service%></i> from host <i><%=host%></i>: </h2>
<font color=red>Service was not stopped in 10 seconds<br>
Do you want to 
<a href="kill_service.jsp?host=<%=URLEncoder.encode(host)%>&service=<%=URLEncoder.encode(service)%>">Kill now</a>
or
<a href="shutdown_service_wait.jsp?host=<%=URLEncoder.encode(host)%>&service=<%=URLEncoder.encode(service)%>">Wait more 10 seconds</a>
</font>
<%} else {%>
<h2>Shutdown service <i><%=service%></i> from host <i><%=host%></i>: </h2>
Service stopped
<%}%>
<%@ include file="/common/footer.jsp"%>