<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");
   String service = request.getParameter("service");%>
<h2>Remove service <i><%=service%></i> from host <i><%=host%></i>: </h2>
Service <i><%=service%></i> removed from host <i><%=host%></i>.
<%@ include file="footer.jsp"%>