<%@ include file="/common/header.jsp"%>
<%
String host = "";
if (request.getParameter("host") != null)
	host = request.getParameter("host");
%>
<FORM action="add_service_2.jsp" method="post">
	<h2>Add service:</h2>
	<TABLE border="1" class="list" cellspacing="0">
		<TR class="list">
			<TH class="list">Service name</TH>
			<TD class="list"><input name="service" value="" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Host</TH>
			<TD class="list"><INPUT name="host" value="<%=host%>" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Port</TH>
			<TD class="list"><input name="port" value="" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Executable</TH>
			<TD class="list"><input name="cmd_line" value="" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Config file</TH>
			<TD class="list"><input name="config" value="" style="width: 100%;"></TD></TR>
		<TR class="list">
			<TH class="list">Startup Arguments</TH>
			<TD class="list"><textarea cols="" rows="5" name="args" wrap="off" style="width: 100%;"></textarea></TD></TR>
	</TABLE>
	<INPUT type=submit value="Add service" name="Submit">
</FORM>
<%@ include file="/common/footer.jsp"%>