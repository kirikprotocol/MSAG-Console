<%@ include file="/common/header.jsp"%>
<%
String host = request.getParameter("host");
String name = request.getParameter("serviceName");
String serviceId = request.getParameter("serviceId");
boolean isRunning = serviceManager.getServiceInfo(serviceId).getPid() != 0;
%>
<h2>Edit service <i><%=old_service%></i>:</h2>
<% if (isRunning) { %>
New startup parameters will be applied on next service start.
<a href="restart_service.jsp?service=<%=serviceId%>&host=<%=host%>">Restart service <I><%=name%></I> now?</a>
<% } else { %>
New startup parameters applied.
<% }%>
<%@ include file="/common/footer.jsp"%>
<% 
if (true) throw new Exception("Not Yet Implemented");
%>