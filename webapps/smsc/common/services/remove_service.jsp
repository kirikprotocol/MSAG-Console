<%@ include file="/common/header.jsp"%>
<% 
	String serviceId = request.getParameter("serviceId");
	String host = request.getParameter("host");
	if (serviceId == null || serviceId.equals("") ||
	    host == null || host.equals(""))
	{
		throw new Exception("Service or host name not defined");
	} 

  String serviceName = serviceManager.getServiceInfo(serviceId).getName();
	serviceManager.removeService(serviceId);
%>
<h2>Remove service <i><%=serviceName%></i> from host <i><%=host%></i>: </h2>
Service <i><%=serviceName%></i> removed from host <i><%=host%></i>.
<%@ include file="/common/footer.jsp"%>