<jsp:useBean id="UC" class="ru.novosoft.smsc.jsp.dbSme.pl.CCEditDatasource"/>
<%@include file="header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.dbSme.pl.CCEditMessages"%>
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
else
{
%>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Data source ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<form method="post" action="edit_datasource.jsp">
<input type="Hidden" name="<%=UC.PARAM_OldProviderName%>" value="<%=StringEncoderDecoder.encode(UC.getProviderName())%>">
<input type="Hidden" name="<%=UC.PARAM_Creating%>" value="<%=UC.isCreating()%>">
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Data provider</td>
</tr>
</table>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<tr>
	<th>name</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_ProviderName%>" value="<%=StringEncoderDecoder.encode(UC.getProviderName())%>"></td>
</tr>
<tr>
	<th>address</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_Address%>" value="<%=UC.getAddress()%>"></td>
</tr>
</tbody></table>

<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Data source</td>
</tr>
</table>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<tr>
	<th>type:</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_Type%>" value="<%=StringEncoderDecoder.encode(UC.getType())%>"></td>
</tr>
<tr>
	<th>connections:</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_Connections%>" value="<%=UC.getConnections()%>"></td>
</tr>
<tr>
	<th>dbInstance:</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_DbInstance%>" value="<%=StringEncoderDecoder.encode(UC.getDbInstance())%>"></td>
</tr>
<tr>
	<th>dbUserName:</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_DbUserName%>" value="<%=StringEncoderDecoder.encode(UC.getDbUserName())%>"></td>
</tr>
<tr>
	<th>dbUserPassword:</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_DbUserPassword%>" value="<%=StringEncoderDecoder.encode(UC.getDbUserPassword())%>"></td>
</tr>
<tr>
	<th><label for="<%=UC.PARAM_watchdog%>">watchdog:</label></th>
	<td><input class="edit" type="Checkbox" id="<%=UC.PARAM_watchdog%>" name="<%=UC.PARAM_watchdog%>" value="true" <%=UC.isWatchdog() ? " checked" : ""%>></td>
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
