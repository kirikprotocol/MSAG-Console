<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
STATUS.append("no changes");
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Configuration status</h1>
<table class=rep0 cellspacing=1 width="50%">
<col width="70%">
<col width="30%" align=center>
<col width="1%">
<col width="1%">
<thead>
<tr>
	<th>Configuration</th>
	<th>Status</th>
	<th>Apply</th>
	<th>Reset</th>
</tr>
</thead>
<tbody>
<tr class=row0>
	<th>SMSC</th>
	<td><span class=Cf00>changed</span></td>
	<td><input class=btn type=submit name=mbSMSCApply value="Apply"></td>
	<td><input class=btn type=submit name=mbSMSCReset value="Reset"></td>
</tr>
<tr class=row1>
	<th>Hosts</th>
	<td><span class=C000>changed</span></td>
	<td><input class=btn type=submit name=mbHostsApply value="Apply"></td>
	<td><input class=btn type=submit name=mbHostsReset value="Reset"></td>
</tr>
<tr class=row0>
	<th>Services</th>
	<td><span class=C000>changed</span></td>
	<td><input class=btn type=submit name=mbServicesApply value="Apply"></td>
	<td><input class=btn type=submit name=mbServicesReset value="Reset"></td>
</tr>
</tbody>
</table>
<h1>Status summary</h1>
<table class=rep0 cellspacing=1 width="50%">
<col width="30%">
<col width="70%">
<thead>
<tr>
	<th>Category</th>
	<th>Value</th>
</tr>
</thead>
<tbody>
<tr class=row0>
	<th class=Cf00>errors</th>
	<td class=C080>no</th>
</tr>
<tr class=row1>
	<th class=C800>warngins</th>
	<td class=C080>12 warnings</th>
</tr>
<tr class=row0>
	<th>services</th>
	<td class=C080>ok</th>
</tr>
</tbody>
</table>

<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

<%--@include file="/common/header.jsp"%>
	<h1 align="center">SMSC administration</h1>
<%@include file="/common/footer.jsp"--%>
