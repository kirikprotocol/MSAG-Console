<%@ include file="/common/header.jsp"%>
<%
String host = request.getParameter("host");
String serviceId = request.getParameter("serviceId");
serviceManager.refreshService(serviceId);
if (serviceManager.getServiceInfo(serviceId).getPid() != 0)
{%>
<h2>Shutdown service <i><%=serviceManager.getServiceInfo(serviceId).getName()%></i> from host <i><%=host%></i>: </h2>
<font color=red>Service was not stopped in 10 seconds<br>
Do you want to 
<a href="kill_service.jsp?host=<%=URLEncoder.encode(host)%>&serviceId=<%=URLEncoder.encode(serviceId)%>">Kill now</a>
or
<a href="shutdown_service_wait.jsp?host=<%=URLEncoder.encode(host)%>&serviceId=<%=URLEncoder.encode(serviceId)%>">Wait more 10 seconds</a>
</font>
<%} else {%>
<h2>Shutdown service <i><%=serviceManager.getServiceInfo(serviceId).getName()%></i> from host <i><%=host%></i>: </h2>
Service stopped
<%}%>
<%@ include file="/common/footer.jsp"%>