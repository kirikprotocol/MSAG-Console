<%
String host = request.getParameter("host");
String service = request.getParameter("service");
%>
<%@ include file="/common/header_begin.jsp"%>
<meta http-equiv="refresh" content="0;url=<%=urlPrefix+servicesPrefix+"/shutdown_and_remove_service_wait.jsp?host="+URLEncoder.encode(host)+"&service="+URLEncoder.encode(service)%>">
<%@ include file="/common/header_end.jsp"%>
<%
serviceManager.shutdownService(service);
%>
<h2>Shutdown and Remove service <i><%=service%></i> from host <i><%=host%></i>: </h2>
Waiting for service shutdown...<br>
<%@ include file="/common/footer.jsp"%>