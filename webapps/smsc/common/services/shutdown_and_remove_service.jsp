<%
String host = request.getParameter("host");
String serviceId = request.getParameter("serviceId");
%>
<%@ include file="/common/header_begin.jsp"%>
<meta http-equiv="refresh" content="0;url=<%=urlPrefix+servicesPrefix+"/shutdown_and_remove_service_wait.jsp?host="+URLEncoder.encode(host)+"&serviceId="+URLEncoder.encode(serviceId)%>">
<%@ include file="/common/header_end.jsp"%>
<%
serviceManager.shutdownService(serviceId);
%>
<h2>Shutdown and Remove service <i><%=StringEncoderDecoder.encode(serviceId)%></i> from host <i><%=StringEncoderDecoder.encode(host)%></i>: </h2>
Waiting for service shutdown...<br>
<%@ include file="/common/footer.jsp"%>