<%@ page errorPage="error.jsp" %>
<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="../header.jsp"%>
<% 
	String name = request.getParameter("name");
	String host = request.getParameter("host");
	String portString = request.getParameter("port");
	if (name == null || name.equals("") || 
	    host == null || host.equals("") || 
			portString == null || portString.equals(""))
	{
		throw new Exception("Not all parameters defined");
	} else {
		int port = 0;
		try {
			port = Integer.decode(portString).intValue();
		} catch (NumberFormatException e) {
			throw new Exception("Port number misformatted: "+e.getMessage());
		}
		
		if (port != 0) {
			ServiceManager m = ctx.getServiceManager();
			m.addService(name, host, port);
			%>
			Service added sucessfully<br>
			<a href="index.jsp">Services</a>
			<%
		}
	}
%>
<%@include file="../footer.jsp"%>