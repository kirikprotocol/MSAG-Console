<%@include file="header.jsp"%>
<% 
	String host = request.getParameter("host");
	if (host == null || host.equals(""))
	{
		throw new Exception("Not all parameters defined");
	} else {
		serviceManager.removeDaemon(host);
	}
%>
<h2>Host "<%=host%>" removed sucessfully</h2>
<%@include file="footer.jsp"%>