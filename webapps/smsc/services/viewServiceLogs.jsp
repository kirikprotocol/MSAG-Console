<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="header.jsp"%>
<% 
	String name = request.getParameter("service");
	String host = request.getParameter("host");
	String logs = serviceManager.getServiceLogs(name, 0, 100);
%>
<pre><%=logs%></pre>
<%=action("Host \""+host+'"', "viewHost.jsp", "host", host)%>
<%@include file="footer.jsp"%>
