<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="header.jsp"%>
<% 
	String host = request.getParameter("host");
	String portString = request.getParameter("port");
	if (host == null || host.equals("") || 
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
			serviceManager.addDaemon(host, port);
			%>
			Service added sucessfully<br>
			<%=action("Hosts", "index.jsp")%>
			<%
		} else
			throw new Exception("Port number not defined");
	}
%>
<%@include file="footer.jsp"%>