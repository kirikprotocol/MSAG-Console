<%@ include file="header.jsp"%>
<h2>Hosts:</h2>
<TABLE class="list" cellspacing="0">
	<TR class="list">
		<TH class="list">Host name</TH>
		<th class="list" width="10%">all services</th>
		<th class="list" width="10%">running services</th>
		<TH class="list" colspan="2">&nbsp;</TH>
	</TR>
	<TR class="list">
		<TD class="list"><A href="view_host.jsp">smsc</A></TD>
		<TD class="list">2</TD>
		<TD class="list">1</TD>
		<TD class="list"><a href="edit_host.jsp?host=smsc">Edit host startup parameters</a></TD>
	<TD class="list"><A href="remove_host_with_services.jsp?host=smsc">stop services and remove host</A></TD>
	</TR>
	<TR class="list">
		<TD class="list"><A href="view_host.jsp">smsc_2</A></TD>
		<TD class="list">0</TD>
		<TD class="list">0</TD>
		<TD class="list"><a href="edit_host.jsp?host=smsc_2">Edit host startup parameters</a></TD>
		<TD class="list"><A href="remove_host.jsp?host=smsc_2">remove</A></TD>
	</TR>
	<TR class="list">
		<TD class="list"><A href="view_host.jsp">smsc.again</A></TD>
		<TD class="list">1</TD>
		<TD class="list">0</TD>
		<TD class="list"><a href="edit_host.jsp?host=smsc.again">Edit host startup parameters</a></TD>
		<TD class="list"><A href="remove_host.jsp?host=smsc.again">remove</A></TD>
	</TR>
</TABLE>
<a href="add_host.jsp">Add host</a>
<%@ include file="footer.jsp"%>
