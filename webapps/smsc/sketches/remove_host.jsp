<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");%>
<h2>Remove host <i><%=host%></i>: </h2>
Host <i><%=host%></i> removed.
<%@ include file="footer.jsp"%>