<%
String host = request.getParameter("host");
String serviceId = request.getParameter("serviceId");
%><%@ include file="/common/header_begin.jsp"%><meta http-equiv="refresh" content="10;url=<%=urlPrefix+servicesPrefix+"/shutdown_service_check.jsp?host="+URLEncoder.encode(host)+"&serviceId="+URLEncoder.encode(serviceId)%>"><%@ include file="/common/header_end.jsp"%>
<h2>Shutdown service <i><%=StringEncoderDecoder.encode(serviceId)%></i> from host <i><%=StringEncoderDecoder.encode(host)%></i>: </h2>
Waiting for service shutdown...<br>
<%@ include file="/common/footer.jsp"%>