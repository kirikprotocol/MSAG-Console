<table class="list" cellspacing="0">
	<TR class="list">
		<TH class="list">Service name</TH>
		<TH class="list">Host name</TH>
		<TH class="list">Status</TH>
		<TH class="list" colSpan=3>Actions</TH>
	</TR>
	<TR class="list">
		<TD class="list"><A href="view_service.jsp">Example service 1</a></TD>
		<TD class="list"><A href="view_host.jsp">smsc</a></TD>
		<TD class="list">running</TD>
		<TD class="list"><A href="shutdown_service.jsp?host=smsc&service=Example+service+1">shutdown</A></TD>
		<TD class="list"><A href="edit_service.jsp?host=smsc&service=Example+service+1&running=true">edit startup parameters</A></TD>
		<TD class="list"><A href="shutdown_and_remove_service.jsp?host=smsc&service=Example+service+1">shutdown and remove</a></TD>
	</TR>
	<%
	if (isFull)
	{%>
		<TR class="list">
			<TD class="list"><A href="view_service.jsp">Second example service</a></TD>
			<TD class="list"><A href="view_host.jsp">smsc.again</a></TD>
			<TD class="list">stopped</TD>
			<TD class="list"><A href="start_service.jsp?host=smsc.again&service=Second+example+service">start</a></TD>
			<TD class="list"><A href="edit_service.jsp?host=smsc.again&service=Second+example+service">edit startup parameters</a></TD>
			<TD class="list"><A href="remove_service.jsp?host=smsc.again&service=Second+example+service">remove</a></TD>
		</TR>
	<%}%>
	<TR class="list">
		<TD class="list"><A href="view_service.jsp">Third example service</a></TD>
		<TD class="list"><A href="view_host.jsp">smsc</a></TD>
		<TD class="list">stopped</TD>
		<TD class="list"><A href="start_service.jsp?host=smsc&service=Third+example+service">start</a></TD>
		<TD class="list"><A href="edit_service.jsp?host=smsc&service=Third+example+service">edit startup parameters</a></TD>
		<TD class="list"><A href="remove_service.jsp?host=smsc&service=Third+example+service">remove</a></TD>
	</TR>
</TABLE>
