<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");
   String service = request.getParameter("service");%>
<h2>Kill and Remove service <i><%=service%></i> from host <i><%=host%></i>: </h2>
Service <i><%=service%></i> killed.<br>
<a href="remove_service.jsp?host=<%=host%>&service=<%=service%>">Remove it</a>
<%@ include file="footer.jsp"%>