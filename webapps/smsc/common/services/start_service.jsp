<%@ include file="/common/header.jsp"%>
<% String host = request.getParameter("host");
   String serviceId = request.getParameter("serviceId");
   String serviceName = serviceManager.getServiceInfo(serviceId).getName();
%>
<h2>Start service <i><%=serviceName%></i> on host <i><%=host%></i>: </h2>
<%serviceManager.startService(serviceId);%>
Service <i><%=serviceName%></i> started.
<%@ include file="/common/footer.jsp"%>