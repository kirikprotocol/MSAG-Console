<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");
   String service = request.getParameter("service");%>
<h2>Edit service <i><%=service%></i>:</h2>
<% if (request.getParameter("running") != null && request.getParameter("running").equals("true")) { %>
New startup parameters will be applied on next service start.
<a href="restart_service.jsp?service=<%=service%>&host=<%=host%>">Restart service <I><%=service%></I> now?</a>
<% } else { %>
New startup parameters applied.
<% }%>
<%@ include file="footer.jsp"%>