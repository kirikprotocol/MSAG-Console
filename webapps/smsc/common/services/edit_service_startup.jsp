<%@ include file="/common/header.jsp"%>
<%
String serviceId = request.getParameter("serviceId");
ServiceInfo s = serviceManager.getServiceInfo(serviceId);
String name = s.getName();
String host = s.getHost();
int    port = s.getPort();
String args = s.getArgs();
%>
<FORM action="edit_service_startup_2.jsp">
	<input type="Hidden" name="host" value="<%=host%>">
	<h2>Edit service:</h2>
	<TABLE border="1" class="list" cellspacing="0">
		<TR class="list">
			<TH class="list">Service name</TH>
			<TD class="list"><input name="serviceName" value="<%=name%>" style="width: 100%;" ></TD></TR>
		<TR class="list">
			<TH class="list">Service Id</TH>
			<TD class="list"><input type="text" name="serviceId" value="<%=serviceId%>" readonly style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Host</TH>
			<TD class="list"><input type="text" name="host" value="<%=host%>" readonly style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Port</TH>
			<TD class="list"><input name="port" value="<%=String.valueOf(port)%>" style="width: 100%;"></TD></TR>
    <TR class="list">
      <TH class="list">Startup Arguments</TH>
      <TD class="list"><textarea cols="" rows="5" name="args" wrap="off" style="width: 100%;"><%=args%></textarea></TD></TR>
	</TABLE>
	<INPUT type=submit value="Apply changes" name="Submit">
</FORM>
<%@ include file="/common/footer.jsp"%>