<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@include file="header.jsp"%>
<% 
	String name = request.getParameter("service");
	String host = request.getParameter("host");
	String portString = request.getParameter("port");
	String cmdLine = request.getParameter("cmdLine");
	String configFileName = request.getParameter("config");
	if (name == null       || name.equals("")        ||
	    host == null       || host.equals("")        ||
			portString == null || portString.equals("") ||
			cmdLine == null    || cmdLine.equals(""))
	{
		throw new Exception("Not all parameters defined");
	} 
	
	int port = 0;
	try {
		port = Integer.decode(portString).intValue();
	} catch (NumberFormatException e) {
		throw new Exception("Port number misformatted: "+e.getMessage());
	}
	if (port == 0) 
		throw new Exception("Port number not defined");
	
	serviceManager.addService(new ServiceInfo(name, host, port, cmdLine, configFileName, new Vector()));
	%>
	Service added sucessfully<br>
	<%=action("Host \""+host+'"', "viewHost.jsp", "host", host)%>
<%@include file="footer.jsp"%>