<%@ include file="header.jsp"%>
<%
String host = request.getParameter("host");
String serviceId = request.getParameter("serviceId");
serviceManager.killService(serviceId);
serviceManager.refreshService(serviceId);
%>
<h2>Kill service <i><%=StringEncoderDecoder.encode(serviceId)%></i> from host <i><%=StringEncoderDecoder.encode(host)%></i>: </h2>
Service <i><%=StringEncoderDecoder.encode(serviceId)%></i> killed.<br>
<%@ include file="footer.jsp"%>