<jsp:useBean id="UC" class="ru.novosoft.smsc.jsp.dbSme.pl.CCEditProvider"/>
<%@include file="header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.dbSme.pl.CCEditMessages,
                ru.novosoft.smsc.jsp.dbSme.pl.CCEditDatasource,
				ru.novosoft.smsc.jsp.dbSme.pl.CCEditJob"%>
<%
int result = UC.process(request, response);
if (result == UC.RESULT_Done)
{
	response.sendRedirect("index.jsp");
}
else if (result == UC.RESULT_NotAllParams)
{
	%><div style="width:100%; font-size:150%; color:Red; text-align:center;">Error: Not all required params defined</div><%
}
else if (result == UC.RESULT_Error)
{
	%><div style="width:100%; font-size:150%; color:Red; text-align:center;">Error:</div><%
}

DataProviderInfo provider = UC.getProvider();
if (provider == null)
{%><div style="font-size: 150%; color:red; align:center">Error: Unknown provider</div><%
}
else
{
%>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Provider ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
DataSourceInfo dataSource = provider.getDataSourceInfo();
%>
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Data provider</td>
	<td class=h1b><!--a href="edit_provider.jsp?<%=UC.PARAM_Name%>=<%=StringEncoderDecoder.encode(provider.getName())%>">edit</a--></td>
</tr>
</table>

<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<tr>
	<th>name</th>
	<td><%=StringEncoderDecoder.encode(provider.getName())%></td>
</tr>
<tr>
	<th>address</th>
	<td><%=StringEncoderDecoder.encode(provider.getAddress())%></td>
</tr>
</tbody></table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Data source ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Data source: <%=StringEncoderDecoder.encode(dataSource.getType())%></td>
	<td class=h1b><a href="edit_datasource.jsp?<%=CCEditDatasource.PARAM_ProviderName%>=<%=URLEncoder.encode(provider.getName())%>">edit</a></td>
</tr>
</table>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<tr>
	<th>type</th>
	<td><%=StringEncoderDecoder.encode(dataSource.getType())%></td>
</tr>
<tr>
	<th>connections</th>
	<td><%=dataSource.getConnections()%></td>
</tr>
<tr>
	<th>dbInstance</th>
	<td><%=StringEncoderDecoder.encode(dataSource.getDbInstance())%></td>
</tr>
<tr>
	<th>dbUserName</th>
	<td><%=StringEncoderDecoder.encode(dataSource.getDbUserName())%></td>
</tr>
<tr>
	<th>dbUserPassword</th>
	<td><%=StringEncoderDecoder.encode(dataSource.getDbUserPassword())%></td>
</tr>
</tbody></table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Messages ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Messages</td>
	<td class=h1b><a href="edit_messages.jsp?<%=CCEditMessages.PARAM_ProviderName%>=<%=URLEncoder.encode(provider.getName())%>">edit</a></td>
</tr>
</table>

<table class=V2 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<%
MessageSet datasourceMessages = provider.getMessages();
for (Iterator j = datasourceMessages.getMessages().keySet().iterator(); j.hasNext(); )
{
	String name = (String) j.next();
	String value = (String) datasourceMessages.getMessages().get(name);
	%><tr>
		<th><%=StringEncoderDecoder.encode(name)%></th>
		<td><%=StringEncoderDecoder.encode(value)%></td>
	</tr><%
}
%>
</tbody>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Jobs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Jobs</td>
</tr>
</table>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col align=left width="96%">
<col width="1%">
<col width="1%">
<col width="1%">
<col width="1%">
<thead>
<tr>
	<th align=center>name</th>
	<th>type</th>
	<th>&nbsp;</th>
</tr>
</thead>
<tbody>
<%
Collection jobs = provider.getJobs();
for (Iterator j = jobs.iterator(); j.hasNext(); )
{
	JobInfo job = (JobInfo) j.next();
	String href = "edit_job.jsp?" + CCEditJob.PARAM_ProviderName + '=' + URLEncoder.encode(provider.getName())
				  + '&' + CCEditJob.PARAM_Name + '=' + URLEncoder.encode(job.getName());
	%>
	<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Job ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr>
	<td><a href="<%=href%>"><%=StringEncoderDecoder.encode(job.getName())%></a></td>
	<td nowrap><%=StringEncoderDecoder.encode(job.getType())%></td>
	<td class=h1b><a href="edit_messages.jsp?<%=CCEditMessages.PARAM_ProviderName%>=<%=
	URLEncoder.encode(provider.getName())%>&<%=CCEditMessages.PARAM_JobName%>=<%=
	URLEncoder.encode(job.getName())%>">messages</a></td>
	<td class=h1b><a href="<%=href + '&' + CCEditJob.BUTTON_DeleteJob + "=del"%>">delete</a></td>
</tr>
<%}%>
<tr>
	<td></td>
	<td></td>
	<td></td>
	<td class=h1b><a href="edit_job.jsp?<%=CCEditJob.PARAM_ProviderName + '=' + URLEncoder.encode(provider.getName()) + '&' + CCEditJob.BUTTON_CreateNewJob + "=new"%>">create</a></td>
</tr>
</tbody></table>
<%}%>
</body>
</html>
