<%@ page errorPage="error.jsp" %>
<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="../header.jsp"%>
<% 
	ServiceManager manager = ctx.getServiceManager();
	String name = request.getParameter("service");
	if (manager.shutdownService(name)) {
		%><h1 align="center">Service <%=name%> shutted down</h1><%
	} else {
		%><h1 align="center">Service <%=name%> was not shutdown properly</h1><%
	}
%>
<a href="index.jsp">Services</a>
<%@include file="../footer.jsp"%>
