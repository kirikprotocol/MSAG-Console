<%@ include file="/common/header.jsp"%>
<% 
	String service = request.getParameter("service");
	String host = request.getParameter("host");
	if (service == null || service.equals("") ||
	    host == null || host.equals(""))
	{
		throw new Exception("Service or host name not defined");
	} 

	serviceManager.removeService(service);
%>
<h2>Remove service <i><%=service%></i> from host <i><%=host%></i>: </h2>
Service <i><%=service%></i> removed from host <i><%=host%></i>.
<%@ include file="/common/footer.jsp"%>