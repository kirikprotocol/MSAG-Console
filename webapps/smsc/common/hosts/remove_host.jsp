<%@ include file="/common/header.jsp"%>
<% 
	String host = request.getParameter("host");
	if (host == null || host.equals(""))
	{
		throw new Exception("Not all parameters defined");
	} else {
		serviceManager.removeDaemon(host);
	}
%>
<h2>Remove host <i><%=host%></i>:</h2>
Host <i><%=host%></i> removed.
<%@ include file="/common/footer.jsp"%>