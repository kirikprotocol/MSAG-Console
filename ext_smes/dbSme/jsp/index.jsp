<jsp:useBean id="UC" class="ru.novosoft.smsc.jsp.dbSme.pl.CCShowConfig"/>
<%@include file="header.jsp"%>
<%UC.process(request, response);%>
<%@page import="ru.novosoft.smsc.jsp.dbSme.pl.CCEditProvider"%>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~ DB SME parameters ~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>DB SME parameters</td>
	<td class=h1b><a href="edit_main_parameters.jsp">edit</a></td>
</tr>
</table>

<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<tr>
	<th>service type</th>
	<td><%=StringEncoderDecoder.encode(UC.getSvcType())%></td>
</tr>
<tr>
	<th>protocol id</th>
	<td><%=UC.getProtocolId()%></td>
</tr>
</tbody></table>
<br>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~ Global messages ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Global messages</td>
	<td class=h1b><a href="edit_messages.jsp">edit</a></td>
</tr>
</table>

<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<%
MessageSet dbSmeMessages = UC.getDbSmeMessages();
String possibleMsgs[] = UC.getPossibleMessageTypes();
boolean something_printed = false;
for (int i = 0; i<possibleMsgs.length; i++)
{
	String name = possibleMsgs[i];
	String value = (String) dbSmeMessages.getMessages().get(name);
	if (value != null && value.length() > 0)
	{
		%><tr>
			<th><%=StringEncoderDecoder.encode(name)%></th>
			<td><%=StringEncoderDecoder.encode(value)%></td>
		</tr><%
		something_printed = true;
	}
}
if (!something_printed)
{
	%><tr><th colspan=2>&lt;empty&gt;</th></tr><%
}
%>
</tbody></table>
<br>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~ Data source drivers ~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Data source drivers</td>
	<td class=h1b><a href="edit_data_source_drivers.jsp">edit</a></td>
</tr>
</table>

<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<%
for (Iterator i = UC.getDataSourceDrivers().iterator(); i.hasNext(); )
{
	DataSourceDriverInfo info = (DataSourceDriverInfo) i.next();
	%><tr>
		<td><%=StringEncoderDecoder.encode(info.getType())%></td>
		<td><%=StringEncoderDecoder.encode(info.getLoadup())%></td>
	</tr><%
}
%>
</table>
<br>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Data providers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
List providers = UC.getProviders();
for (Iterator i = providers.iterator(); i.hasNext(); )
{
	DataProviderInfo provider = (DataProviderInfo) i.next();
	DataSourceInfo dataSource = provider.getDataSourceInfo();
	%>
<table width="100%" cellpadding=0 cellspacing=0 border=0>
<tr>
	<td width="100%" class=h1>Data provider: <%=StringEncoderDecoder.encode(provider.getName())%></td>
	<td class=h1b><a href="edit_provider.jsp?<%=CCEditProvider.PARAM_Name%>=<%=URLEncoder.encode(provider.getName())%>">edit</a></td>
</tr>
</table>

	<%--~~~~~~~~~~~~~~~~~~~~~~~~~~ Data provider ~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<tr>
	<th>type</th>
	<td><%=StringEncoderDecoder.encode(dataSource.getType())%></td>
</tr>
<tr>
	<th>address</th>
	<td><%=StringEncoderDecoder.encode(provider.getAddress())%></td>
</tr>
<%
Collection jobs = provider.getJobs();
boolean isFirstRow = true;
for (Iterator j = jobs.iterator(); j.hasNext(); )
{
	JobInfo job = (JobInfo) j.next();
	%>
<tr>
	<%if (isFirstRow) {
		%><th rowspan="<%=jobs.size()%>">jobs</th><%
		isFirstRow = false;
	}%>
	<td><%=StringEncoderDecoder.encode(job.getName())%></td>
</tr>
<%}%>
</tbody></table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%}%>

</body>
</html>
