<jsp:useBean id="UC" class="ru.novosoft.smsc.jsp.dbSme.pl.CCEditDataSources"/>
<%@include file="header.jsp"%>
<%
int result = UC.process(request, response);
if (result == UC.RESULT_Done)
{
	response.sendRedirect("edit_data_source_drivers.jsp");
} 
else if (result == UC.RESULT_Error) 
{
	%><div style="width:100%; font-size:150%; color:Red; text-align:center;">Error:</div><%
} 
else if (result == UC.RESULT_NotAllParams)
{
	%><div style="width:100%; font-size:150%; color:Red; text-align:center;">Not all parameters defined</div><%
}
%>
<div class=h1>Data source drivers</div>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col width="49%">
<col width="49%">
<col width="1%">
<col width="1%">
<tr>
	<th>Type</th>
	<th>Loadup</th>
	<th colspan="2">&nbsp;</th>
</tr>
<%
for (Iterator i = UC.getDataSourceDrivers().iterator(); i.hasNext(); )
{
	DataSourceDriverInfo info = (DataSourceDriverInfo) i.next();
	%><tr><form>
		<input type="Hidden" name="<%=UC.PARAM_OldName%>" value="<%=StringEncoderDecoder.encode(info.getName())%>">
		<td><input class="edit" type="Text" name="<%=UC.PARAM_Type%>" value="<%=StringEncoderDecoder.encode(info.getType())%>"></td>
		<td><input class="edit" type="Text" name="<%=UC.PARAM_Loadup%>" value="<%=StringEncoderDecoder.encode(info.getLoadup())%>"></td>
		<td><input class="button" type="Submit" name="<%=UC.BUTTON_Save%>" value="Apply"></td>
		<td><input class="button" type="Submit" name="<%=UC.BUTTON_Remove%>" value="Delete"></td>
	</form></tr><%
}
%><tr><form method="post">
		<td><input class="edit" type="Text" name="<%=UC.PARAM_Type%>" value=""></td>
		<td><input class="edit" type="Text" name="<%=UC.PARAM_Loadup%>" value=""></td>
		<td colspan=2><input class="button" style="width:100%" type="Submit" name="<%=UC.BUTTON_Add%>" value="Add"></td>
	</form></tr>
</table>
</body>
</html>
