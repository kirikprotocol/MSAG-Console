<%@ include file="/common/header.jsp"%>
<%
String service = request.getParameter("service");
String host = request.getParameter("host");
String portString = request.getParameter("port");
String cmdLine = request.getParameter("cmd_line");
String configFileName = request.getParameter("config");
String args = request.getParameter("args");
if (service == null       || service.equals("")        ||
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

if (args == null)
	args = "";
	
serviceManager.addService(new ServiceInfo(service, host, port, cmdLine, configFileName, args));
%>
<h2>Add service:</h2>
Service <i><%=service%></i> added sucessfully.
<%@ include file="/common/footer.jsp"%>