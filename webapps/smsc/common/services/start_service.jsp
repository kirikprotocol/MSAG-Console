<%@ include file="/common/header.jsp"%>
<% String host = request.getParameter("host");
   String service = request.getParameter("service");%>
<h2>Start service <i><%=service%></i> on host <i><%=host%></i>: </h2>
<%serviceManager.startService(service);%>
Service <i><%=service%></i> started.
<%@ include file="/common/footer.jsp"%>