<%@ include file="/common/header.jsp"%>
<%
String host = request.getParameter("host");
//String name = request.getParameter("serviceName");
String serviceId = request.getParameter("serviceId");
String portString = request.getParameter("port");
String args = request.getParameter("args");

int port = 0;
try {
  port = Integer.decode(portString).intValue();
} catch (NumberFormatException e) {
  throw new Exception("Port number misformatted: "+e.getMessage());
}
if (port == 0)
  throw new Exception("Port number not defined");

ServiceInfo info = serviceManager.getServiceInfo(serviceId);
boolean isRunning = info.getPid() != 0;
//String old_service = info.getName();

serviceManager.setStartupParameters(serviceId, host, /*name, */port, args);
%>
<h2>Edit service <i><%=StringEncoderDecoder.encode(serviceId)%></i>:</h2>
<% if (isRunning) { %>
New startup parameters will be applied on next service start.
<a href="restart_service.jsp?service=<%=URLEncoder.encode(serviceId)%>&host=<%=URLEncoder.encode(host)%>">Restart service <I><%=StringEncoderDecoder.encode(serviceId)%></I> now?</a>
<% } else { %>
New startup parameters applied.
<% }%>
<%@ include file="/common/footer.jsp"%>