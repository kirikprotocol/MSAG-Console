<%@ include file="/common/header.jsp"%>
<% String host = request.getParameter("host");
   String serviceId = request.getParameter("serviceId");
   //String serviceName = hostsManager.getServiceInfo(serviceId).getName();
%>
<h2>Start service <i><%=StringEncoderDecoder.encode(serviceId)%></i> on host <i><%=StringEncoderDecoder.encode(host)%></i>: </h2>
<%serviceManager.startService(serviceId);%>
Service <i><%=StringEncoderDecoder.encode(serviceId)%></i> started.
<%@ include file="/common/footer.jsp"%>