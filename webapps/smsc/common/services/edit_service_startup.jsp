<%@ include file="/common/header.jsp"%>
<%
String service = request.getParameter("service");
ServiceInfo s = serviceManager.getServiceInfo(service);
String host = s.getHost();
int port = s.getPort();
String cmd = s.getCmdLine();
String configFile = s.getConfigFileName();
Vector argus = s.getArgs();
String args = "";
if (argus.size() > 0)
	args = (String) argus.elementAt(0);
%>
<FORM action="edit_service_startup_2.jsp">
	<input type="Hidden" name="host" value="<%=host%>">
	<input type="Hidden" name="old_service" value="<%=service%>">
	<h2>Edit service:</h2>
	<TABLE border="1" class="list" cellspacing="0">
		<TR class="list">
			<TH class="list">Service name</TH>
			<TD class="list"><input name="service" value="<%=service%>" style="width: 100%;" ></TD></TR>
		<TR class="list">
			<TH class="list">Host</TH>
			<TD class="list"><input name="host" value="<%=host%>" style="width: 100%;" readonly></TD></TR>
		<TR class="list">
			<TH class="list">Port</TH>
			<TD class="list"><input name="port" value="<%=String.valueOf(port)%>" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Executable</TH>
			<TD class="list"><input name="config" value="<%=cmd%>" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Config file</TH>
			<TD class="list"><input name="config" value="<%=configFile%>" style="width: 100%;"></TD></TR>
			<TR class="list">
				<TH class="list">Startup Arguments</TH>
				<TD class="list"><textarea cols="" rows="5" name="args" wrap="off" style="width: 100%;"><%=args%></textarea></TD></TR>
	</TABLE>
	<INPUT type=submit value="Apply changes" name="Submit">
</FORM>
<%@ include file="/common/footer.jsp"%>