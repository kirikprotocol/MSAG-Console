<%@ include file="/common/header.jsp"%>
<% 
	String serviceId = request.getParameter("serviceId");
	String host = request.getParameter("host");
	if (serviceId == null || serviceId.equals("") ||
	    host == null || host.equals(""))
	{
		throw new Exception("Service or host name not defined");
	} 
%>
<h2>Remove service <i><%=serviceId%></i> from host <i><%=host%></i>: </h2>
<%serviceManager.removeService(serviceId);%>
Service <i><%=serviceId%></i> removed from host <i><%=host%></i>.<br>
<a href="<%=urlPrefix+smscPrefix%>/apply_routes.jsp">Restart SMSC to apply new sme's config now</a> or <a href="<%=urlPrefix%>/">restart SMSC later</a>.
<%@ include file="/common/footer.jsp"%>