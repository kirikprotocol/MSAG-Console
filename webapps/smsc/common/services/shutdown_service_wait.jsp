<%
String host = request.getParameter("host");
String service = request.getParameter("service");
%><%@ include file="/common/header_begin.jsp"%><meta http-equiv="refresh" content="10;url=<%=urlPrefix+servicesPrefix+"/shutdown_service_check.jsp?host="+URLEncoder.encode(host)+"&service="+URLEncoder.encode(service)%>"><%@ include file="/common/header_end.jsp"%>
<h2>Shutdown service <i><%=service%></i> from host <i><%=host%></i>: </h2>
Waiting for service shutdown...<br>
<%@ include file="/common/footer.jsp"%>