<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");%>
<h2>Edit host <i><%=host%></i>:</h2>
Parameters will be applied on next host reboot.
<%@ include file="footer.jsp"%>