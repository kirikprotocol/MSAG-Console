<jsp:useBean id="UC" class="ru.novosoft.smsc.jsp.dbSme.pl.CCEditJob"/>
<%@include file="header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.dbSme.pl.CCEditProvider"%>
<%
int result = UC.process(request, response);
if (result == UC.RESULT_Done)
{
	response.sendRedirect("edit_provider.jsp?" + CCEditProvider.PARAM_Name + '=' + URLEncoder.encode(UC.getProviderName()));
}
else if (result == UC.RESULT_NotAllParams)
{
	%><div style="width:100%; font-size:150%; color:Red; text-align:center;">Error: Not all required params defined</div><%
}
else if (result == UC.RESULT_Error)
{
	%><div style="width:100%; font-size:150%; color:Red; text-align:center;">Error:</div><%
}
else if (result == UC.RESULT_Ok)
{
%>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Job ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<form method="get" action="edit_job.jsp">
<input type="Hidden" name="<%=UC.PARAM_ProviderName%>" value="<%=StringEncoderDecoder.encode(UC.getProviderName())%>">
<%
if (UC.isCreating())
{
%><input type=hidden name="<%=UC.PARAM_IsCreating%>" value=new><%
} else {
%><input type="Hidden" name="<%=UC.PARAM_OldName%>" value="<%=StringEncoderDecoder.encode(UC.getName())%>"><%
}%>
<table width="100%" cellpadding=0 cellspacing=0 border=0 id="main_table_id_01">
<tr>
	<td width="100%" class=h1>Job</td>
</tr>
</table>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<col nowrap align=left>
<tbody>
<tr>
	<th>Name</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_Name%>" value="<%=StringEncoderDecoder.encode(UC.getName())%>"></td>
</tr>
<tr>
	<th>Type</th>
	<td><%
		if (UC.isCreating()) {
		%><select onchange="showSelectedParams(value)" name="<%=UC.PARAM_Type%>">
			<option value="<%=SqlJobInfo  .JOB_TYPE_Sql  %>" <%=SqlJobInfo  .JOB_TYPE_Sql.equals  (UC.getType()) ? "selected" : ""%>>SQL job</option>
			<option value="<%=PlSqlJobInfo.JOB_TYPE_PlSql%>" <%=PlSqlJobInfo.JOB_TYPE_PlSql.equals(UC.getType()) ? "selected" : ""%>>PL SQL job</option>
		</select><%
		} else {
			%><input readonly class="edit" type="Text" name="<%=UC.PARAM_Type%>" value="<%=StringEncoderDecoder.encode(UC.getType())%>"><%
		}%>
	</td>
</tr>
<tr id="row_<%=UC.PARAM_Address%>">
	<th>Address</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_Address%>" value="<%=StringEncoderDecoder.encode(UC.getAddress())%>"></td>
</tr>
<tr id="row_<%=UC.PARAM_Alias%>">
	<th>Alias</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_Alias%>" value="<%=StringEncoderDecoder.encode(UC.getAlias())%>"></td>
</tr>
<tr>
	<th><label for="<%=UC.PARAM_timeout%>">Timeout</label></th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_timeout%>" value="<%=UC.getTimeout()%>"></td>
</tr>
</table>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0" id=PARAMS_ROWS_TABLE_SQL_JOB>
<col nowrap align=center width="15%">
<col nowrap align=left>
<tbody>
<tr id="row_<%=UC.PARAM_Query%>" name="row_<%=UC.PARAM_Query%>">
	<th><label for="<%=UC.PARAM_Query%>">Query</label></th>
	<td><input class="edit" type="Checkbox" id="<%=UC.PARAM_Query%>" name="<%=UC.PARAM_Query%>" value="true"<%=UC.isQuery() ? " checked" : ""%>></td>
</tr>
</table>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0" id=PARAMS_ROWS_TABLE_PL_SQL_JOB>
<col nowrap align=center width="15%">
<col nowrap align=left>
<tbody>
<tr id="row_<%=UC.PARAM_Commit%>">
	<th><label for="<%=UC.PARAM_Commit%>">Commit</label></th>
	<td><input class="edit" type="Checkbox" id="<%=UC.PARAM_Commit%>" name="<%=UC.PARAM_Commit%>" value="true" <%=UC.isCommit() ? " checked" : ""%>></td>
</tr>
<tr id="row_<%=UC.PARAM_Function%>">
	<th><label for="<%=UC.PARAM_Function%>">Function</label></th>
	<td><input class="edit" type="Checkbox" id="<%=UC.PARAM_Function%>" name="<%=UC.PARAM_Function%>" value="true" <%=UC.isFunction() ? " checked" : ""%>></td>
</tr>
</table>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<col nowrap align=left>
<tbody>
<tr>
	<th>Sql</th>
	<td><textarea class="edit" cols="80" rows="5" name="<%=UC.PARAM_Sql%>"><%=StringEncoderDecoder.encode(UC.getSql())%></textarea>
</tr>
<tr>
	<th>Input</th>
	<td><textarea class="edit" cols="80" rows="5" name="<%=UC.PARAM_Input%>"><%=StringEncoderDecoder.encode(UC.getInput())%></textarea>
</tr>
<tr>
	<th>Output</th>
	<td><textarea class="edit" cols="80" rows="5" name="<%=UC.PARAM_Output%>"><%=StringEncoderDecoder.encode(UC.getOutput())%></textarea>
</tr>
</tbody></table>
<div align="right">
	<input class="button" type="Submit" name="<%=UC.BUTTON_Save%>" value="Save">
	<input class="button" type="Reset" value="Reset">
	<input class="button" type="Submit" name="<%=UC.BUTTON_Cancel%>" value="Cancel">
</div>
</form>
<%}%>
<script>
function showSelectedParams(selection_value)
{
	try {
		if (selection_value == "<%=SqlJobInfo.JOB_TYPE_Sql%>")
		{
			document.all.PARAMS_ROWS_TABLE_PL_SQL_JOB.runtimeStyle.display = "none";
			document.all.PARAMS_ROWS_TABLE_SQL_JOB   .runtimeStyle.display = "block";
		} else {
			document.all.PARAMS_ROWS_TABLE_PL_SQL_JOB.runtimeStyle.display = "block";
			document.all.PARAMS_ROWS_TABLE_SQL_JOB   .runtimeStyle.display = "none";
		}
	} catch (e)
	{
	}
}
showSelectedParams("<%=UC.getType()%>");
</script>
</body>
</html>
