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
<table width="100%" cellpadding=0 cellspacing=0 border=0>
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
	<td><input readonly class="edit" type="Text" name="<%=UC.PARAM_Type%>" value="<%=StringEncoderDecoder.encode(UC.getType())%>"></td>
</tr>
<tr>
	<th><label for="<%=UC.PARAM_Query%>">Query</label></th>
	<td><input class="edit" type="Checkbox" id="<%=UC.PARAM_Query%>" name="<%=UC.PARAM_Query%>" value="true"<%=UC.isQuery() ? " checked" : ""%>></td>
</tr>
<tr>
	<th>Address</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_Address%>" value="<%=StringEncoderDecoder.encode(UC.getAddress())%>"></td>
</tr>
<tr>
	<th>Alias</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_Alias%>" value="<%=StringEncoderDecoder.encode(UC.getAlias())%>"></td>
</tr>
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
</body>
</html>
