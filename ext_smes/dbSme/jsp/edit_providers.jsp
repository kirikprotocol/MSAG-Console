<jsp:useBean id="UC" class="ru.novosoft.smsc.jsp.dbSme.pl.CCShowConfig"/>
<%@include file="header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.dbSme.pl.CCEditProvider,
                ru.novosoft.smsc.jsp.dbSme.pl.CCEditDatasource"%>
<%UC.process(request, response);%>
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
<br>
<div align="right">
	<div class=h1b>
		<a href="edit_datasource.jsp?<%=CCEditDatasource.BUTTON_Create%>=new">Create new provider</a>
	</div>
</div>
</body>
</html>
