<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");%>
<h2>Remove host <i><%=host%></i>: </h2>
Service <i>Example service 1</i> stopped<br>
<a href="remove_host.jsp?host=<%=host%>">remove host</a><br>
<%@ include file="footer.jsp"%>