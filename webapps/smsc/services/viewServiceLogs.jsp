<%@ page errorPage="error.jsp" %>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="../header.jsp"%>
<% 
	ServiceManager manager = ctx.getServiceManager();
	String logs = manager.getServiceLogs(request.getParameter("service"), 0, 100);
%>
<pre><%=logs%></pre>
<a href="index.jsp">Services</a>
<%@include file="../footer.jsp"%>
