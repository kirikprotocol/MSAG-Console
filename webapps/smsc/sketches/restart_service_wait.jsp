<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");
   String service = request.getParameter("service");%>
<h2>Restart service <i><%=service%></i> from host <i><%=host%></i>: </h2>
Waiting again for service shutdown...<br>
<%
out.flush();
//Thread.currentThread().sleep(10000);
%>
<font color=red>Service was not stopped in 10 seconds<br>
Do you want to 
<a href="restart_service_kill.jsp?host=<%=host%>&service=<%=service%>">Kill now</a>
or
<a href="restart_service_wait.jsp?host=<%=host%>&service=<%=service%>">Wait more 10 seconds</a>
</font>
<%@ include file="footer.jsp"%>