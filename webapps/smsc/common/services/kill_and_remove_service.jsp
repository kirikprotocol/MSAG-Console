<%@ include file="/common/header.jsp"%>
<%
String host = request.getParameter("host");
String serviceId = request.getParameter("serviceId");
serviceManager.killService(serviceId);
serviceManager.refreshService(serviceId);
%>
<h2>Kill and Remove service <i><%=StringEncoderDecoder.encode(serviceId)%></i> from host <i><%=StringEncoderDecoder.encode(host)%></i>: </h2>
Service <i><%=StringEncoderDecoder.encode(serviceId)%></i> killed.<br>
<a href="remove_service.jsp?host=<%=StringEncoderDecoder.encode(host)%>&serviceId=<%=StringEncoderDecoder.encode(serviceId)%>">Remove it</a>
<%@ include file="/common/footer.jsp"%>