<%@ include file="header.jsp"%>
<% String host = request.getParameter("host");%>
<FORM method="post" action="edit_host_2.jsp">
<input type="Hidden" name="host" value="<%=host%>">
<h2>Edit host <i><%=host%></i>:</h2>
<TABLE border="1" class="list" cellspacing="0">
	<TR class="list">
		<TH class="list">home directory</TH>
		<TD class="list"><input name="home directory" value="/export/home/igork/smsc_bin_daemon" style="width: 100%;"></TD></TR>
	<TR class="list">
		<TH class="list">Port</TH>
		<TD class="list"><input name="port" value="6680" style="width: 100%;"></TD></TR>
	<TR class="list">
		<TH class="list">stderr output file</TH>
		<TD class="list"><input name="stderr" value="logs/daemon.err" style="width: 100%;"></TD></TR>
	<TR class="list">
		<TH class="list">log configuration file</TH>
		<TD class="list"><input name="log_config" value="daemon.logger.init" style="width: 100%;"></TD></TR>
</TABLE>
<INPUT type=submit value="Set new parameters" name="submit"></FORM>
<%@ include file="footer.jsp"%>