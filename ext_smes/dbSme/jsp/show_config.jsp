<jsp:useBean id="UC" class="ru.novosoft.smsc.jsp.dbSme.pl.CCShowConfig"/>
<%UC.process(request, response);%>
<%@include file="header.jsp"%>

<%--~~~~~~~~~~~~~~~~~~~~~~~~ DB SME parameters: ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=h1>DB SME parameters</div>
<table class=V2 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="8%">
<col nowrap align=center width="7%">
<tbody>
<tr>
	<th colspan=2 nowrap>service type</th>
	<td><%=StringEncoderDecoder.encode(UC.getSvcType())%></td>
</tr>
<tr>
	<th colspan=2 nowrap>protocol id</th>
	<td><%=UC.getProtocolId()%></td>
</tr>
<tr>
	<th rowspan=2 nowrap>thread pool</th>
	<th nowrap>init</th>
	<td><%=UC.getThreadPoolInfo().getInit()%></td>
</tr>
<tr>
	<th nowrap>max</th>
	<td><%=UC.getThreadPoolInfo().getMax()%>&nbsp;</td>
</tr>
<tr>
	<th rowspan=5 nowrap>SMSC</th>
	<th nowrap>host</th>
	<td><%=StringEncoderDecoder.encode(UC.getSMSCInfo().getHost())%>&nbsp;</td>
</tr>
<tr>
	<th nowrap>port</th>
	<td><%=UC.getSMSCInfo().getPort()%>&nbsp;</td>
</tr>
<tr>
	<th nowrap>password</th>
	<td><%=StringEncoderDecoder.encode(UC.getSMSCInfo().getPassword())%>&nbsp;</td>
</tr>
<tr>
	<th nowrap>sid</th>
	<td><%=StringEncoderDecoder.encode(UC.getSMSCInfo().getSid())%>&nbsp;</td>
</tr>
<tr>
	<th nowrap>timeout</th>
	<td><%=UC.getSMSCInfo().getTimeout()%>&nbsp;</td>
</tr>
</tbody>
</table>
<br>
<%--!~~~~~~~~~~~~~~~~~~~~~~~~~~ Global messages ~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=h1>Global messages</div>
<table class=V2 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<%
MessageSet dbSmeMessages = UC.getDbSmeMessages();
for (Iterator i = dbSmeMessages.getMessages().keySet().iterator(); i.hasNext(); )
{
	String name = (String) i.next();
	String value = (String) dbSmeMessages.getMessages().get(name);
	%><tr>
		<th><%=StringEncoderDecoder.encode(name)%></th>
		<td><%=StringEncoderDecoder.encode(value)%></td>
	</tr><%
}
%>
</tbody>
</table>
<br>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~ Data source drivers ~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=h1>Data Source Drivers</div>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tr>
	<th>Type</th>
	<th>Loadup</th>
</tr>
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
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Providers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
List providers = UC.getProviders();
for (Iterator i = providers.iterator(); i.hasNext(); )
{
	DataProviderInfo provider = (DataProviderInfo) i.next();
	DataSourceInfo dataSource = provider.getDataSourceInfo();
	%>
<div class=h1>Data provider: <%=StringEncoderDecoder.encode(provider.getName())%></div>
	<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Provider ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
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
<div class=h1>Data source</div>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="3%">
<col nowrap align=center width="4%">
<col nowrap align=center width="8%">
<tbody>
<tr>
	<th colspan=3>type</th>
	<td><%=StringEncoderDecoder.encode(dataSource.getType())%></td>
</tr>
<tr>
	<th colspan=3>connections</th>
	<td><%=dataSource.getConnections()%></td>
</tr>
<tr>
	<th rowspan=3>DB</th>
	<th colspan=2>instance</th>
	<td><%=StringEncoderDecoder.encode(dataSource.getDbInstance())%></td>
</tr>
<tr>
	<th rowspan=2>user</th>
	<th>name</th>
	<td><%=StringEncoderDecoder.encode(dataSource.getDbUserName())%></td>
</tr>
<tr>
	<th>password</th>
	<td><%=StringEncoderDecoder.encode(dataSource.getDbUserPassword())%></td>
</tr>
</tbody></table>
<div class=h1>Messages</div>
<table class=V2 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<tbody>
<%
MessageSet providerMessages = provider.getMessages();
for (Iterator j = providerMessages.getMessages().keySet().iterator(); j.hasNext(); )
{
	String name = (String) j.next();
	String value = (String) providerMessages.getMessages().get(name);
	%><tr>
		<th><%=StringEncoderDecoder.encode(name)%></th>
		<td><%=StringEncoderDecoder.encode(value)%></td>
	</tr><%
}
%>
</tbody>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Jobs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%
Collection jobs = provider.getJobs();
for (Iterator j = jobs.iterator(); j.hasNext(); )
{
	JobInfo job = (JobInfo) j.next();
	%>
	<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Job ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=h1>Job: <%=StringEncoderDecoder.encode(job.getName())%></div>
<table class=V1 width="100%" cellpadding=1 cellspacing=1 border="0">
<col nowrap align=center width="15%">
<col nowrap align=left>
<tbody>
<tr>
	<th>type</th>
	<td><%=StringEncoderDecoder.encode(job.getType())%></td>
</tr>
<tr>
	<th>query</th>
	<td><%=job.isQuery()%></td>
</tr>
<tr>
	<th>sql</th>
	<td><pre><%=StringEncoderDecoder.encode(job.getSql())%></pre></td>
</tr>
<tr>
	<th>input</th>
	<td><pre><%=StringEncoderDecoder.encode(job.getInput())%></pre></td>
</tr>
<tr>
	<th>output</th>
	<td><pre><%=StringEncoderDecoder.encode(job.getOutput())%></pre></td>
</tr>
</tbody></table>
<%}%>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<%}%>
</body>
</html>
