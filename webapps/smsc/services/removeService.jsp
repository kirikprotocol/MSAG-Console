<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="header.jsp"%>
<% 
	String name = request.getParameter("service");
	String host = request.getParameter("host");
	if (name == null || name.equals("") ||
	    host == null || host.equals(""))
	{
		throw new Exception("Service or host name not defined");
	} 

	serviceManager.removeService(name);
	%>
	Service removed sucessfully<br>
	<%=action("hosts", "index.jsp")%><br>
	<%=action("Services on \"" + host + "\"", "viewHost.jsp?host="+host)%>
<%@include file="footer.jsp"%>