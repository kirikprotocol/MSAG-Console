<%
String host = request.getParameter("host");
String serviceId = request.getParameter("serviceId");
%>
<%@ include file="/common/header_begin.jsp"%>
<meta http-equiv="refresh" content="0;url=<%=urlPrefix+servicesPrefix+"/shutdown_service_wait.jsp?host="+URLEncoder.encode(host)+"&serviceId="+URLEncoder.encode(serviceId)%>">
<%@ include file="/common/header_end.jsp"%>
<%
serviceManager.shutdownService(serviceId);
%>
<h2>Shutdown service <i><%=serviceManager.getServiceInfo(serviceId).getName()%></i> from host <i><%=host%></i>: </h2>
Waiting for service shutdown...<br>
<%@ include file="/common/footer.jsp"%>