<%@ include file="header.jsp"%>
<h2>Add host:</h2>
<TABLE border="1" class="list" cellspacing="0">
	<TR class="list">
		<TH class="list">host name</TH>
		<TD class="list"><input type="text" name="name" value="smsc_3" style="width: 100%;"></TD></TR>
	<TR class="list">
		<TH class="list">Port</TH>
		<TD class="list"><input name="port" value="1234" style="width: 100%;"></TD></TR>
</TABLE>
<FORM action="add_host_2.jsp"><INPUT type=submit value=Submit name="add host"></FORM>
<%@ include file="footer.jsp"%>