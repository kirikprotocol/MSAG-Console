<%@ include file="/common/header.jsp"%>
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
		} else
			throw new Exception("Port number not defined");
	}
%>
<h2>Add host:</h2>
Host <i><%=host%></i> added sucessfully
<%@ include file="/common/footer.jsp"%>