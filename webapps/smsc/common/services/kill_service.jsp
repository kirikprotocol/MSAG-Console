<%@ include file="header.jsp"%>
<%
String host = request.getParameter("host");
String service = request.getParameter("service");
serviceManager.killService(service);
serviceManager.refreshService(service);
%>
<h2>Kill service <i><%=service%></i> from host <i><%=host%></i>: </h2>
Service <i><%=service%></i> killed.<br>
<%@ include file="footer.jsp"%>