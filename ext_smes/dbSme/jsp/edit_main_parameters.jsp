<jsp:useBean id="UC" class="ru.novosoft.smsc.jsp.dbSme.pl.CCEditMainParams"/>
<%@include file="header.jsp"%>
<%
int result = UC.process(request, response);
if (result == UC.RESULT_Done)
{
	response.sendRedirect("index.jsp");
} else if (result == UC.RESULT_Error) 
{
	%><div style="width:100%; font-size:150%; color:Red; text-align:center;">Error:</div><%
}
%>

<form method="post">
<div class=h1>DB SME parameters</div>
<table class=V2 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="8%">
<col nowrap align=center width="7%">
<tbody>
<tr>
	<th colspan=2 nowrap>service type</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_SvcType%>" value="<%=StringEncoderDecoder.encode(UC.getSvcType())%>"></td>
</tr>
<tr>
	<th colspan=2 nowrap>protocol id</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_ProtocolId%>" value="<%=UC.getProtocolId()%>"></td>
</tr>
<tr>
	<th rowspan=2 nowrap>thread pool</th>
	<th nowrap>init</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_ThreadPool_Init%>" value="<%=UC.getThreadPoolInfo().getInit()%>"></td>
</tr>
<tr>
	<th nowrap>max</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_ThreadPool_Max%>" value="<%=UC.getThreadPoolInfo().getMax()%>"></td>
</tr>
<tr>
	<th rowspan=5 nowrap>SMSC</th>
	<th nowrap>host</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_SMSC_Host%>" value="<%=StringEncoderDecoder.encode(UC.getSMSCInfo().getHost())%>"></td>
</tr>
<tr>
	<th nowrap>port</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_SMSC_Port%>" value="<%=UC.getSMSCInfo().getPort()%>"></td>
</tr>
<tr>
	<th nowrap>password</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_SMSC_Password%>" value="<%=StringEncoderDecoder.encode(UC.getSMSCInfo().getPassword())%>"></td>
</tr>
<tr>
	<th nowrap>sid</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_SMSC_Sid%>" value="<%=StringEncoderDecoder.encode(UC.getSMSCInfo().getSid())%>"></td>
</tr>
<tr>
	<th nowrap>timeout</th>
	<td><input class="edit" type="Text" name="<%=UC.PARAM_SMSC_Timeout%>" value="<%=UC.getSMSCInfo().getTimeout()%>"></td>
</tr>
</tbody>
</table>
<div align="right">
	<input class="button" type="Submit" name="<%=UC.BUTTON_Save%>" value="Save">
	<input class="button" type="Reset" value="Reset">
	<input class="button" type="Submit" name="<%=UC.BUTTON_Cancel%>" value="Cancel">
</div>
</form>
</body>
</html>
