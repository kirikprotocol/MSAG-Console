<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");
   String service = request.getParameter("service");%>
<h2>Start service <i><%=service%></i> on host <i><%=host%></i>: </h2>
Service <i><%=service%></i> started.
<%@ include file="footer.jsp"%>