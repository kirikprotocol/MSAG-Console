<%@ include file="/sketches/header.jsp"%>
<TABLE>
	<TR>
		<TD><A href="../routes/index.jsp"><h2>Routes</h2></A></TD>
		<TD><A href="aliases.jsp"><h2>Aliases</h2></A></TD>
	</TR>
</TABLE>
<h2>Aliases</h2>
<h3>Add alias:</h3>
Alias "<%=request.getParameter("from")%>&nbsp;-&gt;&nbsp;<%=request.getParameter("to")%>" <%=request.getParameter("is_add") != null ? "added" : "edited"%> sucessfully.
<%@ include file="/sketches/footer.jsp"%>