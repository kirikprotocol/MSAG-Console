<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="header.jsp"%>
<% 
	String name = request.getParameter("service");
	String host = request.getParameter("host");
	serviceManager.killService(name);
%>
<h1 align="center">Service <%=name%> killed</h1>
<%=action("Host \""+host+'"', "viewHost.jsp", "host", host)%>
<%@include file="footer.jsp"%>
