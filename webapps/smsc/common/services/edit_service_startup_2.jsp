<%@ include file="/common/header.jsp"%>
<%
String host = request.getParameter("host");
String service = request.getParameter("service");
String old_service = request.getParameter("old_service");
boolean isRunning = serviceManager.getServiceInfo(old_service).getPid() != 0;
%>
<h2>Edit service <i><%=old_service%></i>:</h2>
<% if (isRunning) { %>
New startup parameters will be applied on next service start.
<a href="restart_service.jsp?service=<%=service%>&host=<%=host%>">Restart service <I><%=service%></I> now?</a>
<% } else { %>
New startup parameters applied.
<% }%>
<%@ include file="/common/footer.jsp"%>
<% 
if (true) throw new Exception("Not Yet Implemented");
%>