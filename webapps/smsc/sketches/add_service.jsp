<%@ include file="header.jsp"%>
<FORM action="add_service_2.jsp">
	<h2>Add service:</h2>
	<TABLE border="1" class="list" cellspacing="0">
		<TR class="list">
			<TH class="list">Service name</TH>
			<TD class="list"><input type="text" name="name" value="Dumb service 3" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Host</TH>
			<TD class="list"><INPUT name="host" value="smsc" style="width: 100%;"></TD></TR>
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