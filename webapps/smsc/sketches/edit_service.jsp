<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");
   String service = request.getParameter("service");%>
<FORM action="edit_service_2.jsp">
	<input type="Hidden" name="host" value="<%=host%>">
	<input type="Hidden" name="service" value="<%=service%>">
	<input type="Hidden" name="running" value="<%=request.getParameter("running")%>">
	<h2>Edit service:</h2>
	<TABLE border="1" class="list" cellspacing="0">
		<TR class="list">
			<TH class="list">Service name</TH>
			<TD class="list"><input type="text" name="name" style="width: 100%;" value="<%=service%>"></TD></TR>
		<TR class="list">
			<TH class="list">Host</TH>
			<TD class="list"><input type="text" name="host" value="<%=host%>" readonly style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Port</TH>
			<TD class="list"><input name="port" value="1234" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Executable</TH>
			<TD class="list"><input name="config" value="/etc/smsc/dumb3/service" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Config file</TH>
			<TD class="list"><input name="config" value="/etc/smsc/dumb3/service_config.xml" style="width: 100%;"></TD></TR>
			<TR class="list">
				<TH class="list">Startup Arguments</TH>
				<TD class="list"><textarea cols="" rows="5" name="args" wrap="off" style="width: 100%;">something first
something else</textarea></TD></TR>
	</TABLE>
	<INPUT type=submit value=Submit name="add service">
</FORM>
<%@ include file="footer.jsp"%>