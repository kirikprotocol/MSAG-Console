<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
STATUS.append("no changes");
%><%@ page import="ru.novosoft.smsc.jsp.smsc.Index,"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		STATUS.append("Ok");
		break;
	case Index.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>

<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Configuration status</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
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
</tr>
</thead>
<tbody>
<%int rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th>Routes</th>
	<td><%=bean.isRoutesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbRoutesApply value="Apply" <%=!bean.isRoutesChanged() ? "disabled" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>Subjects</th>
	<td><%=bean.isSubjectsChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbSubjectsApply value="Apply" <%=!bean.isSubjectsChanged() ? "disabled" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>Aliases</th>
	<td><%=bean.isAliasesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbAliasesApply value="Apply" <%=!bean.isAliasesChanged() ? "disabled" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>Profiles</th>
	<td><%=bean.isProfilesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbProfilesApply value="Apply" <%=!bean.isProfilesChanged() ? "disabled" : ""%>></td>
</tr>
<tr class=row<%=(rowN++)&1%>>
	<th>Hosts</th>
	<td><%=bean.isHostsChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbHostsApply value="Apply" <%=!bean.isHostsChanged() ? "disabled" : ""%>></td>
</tr>
<tr class=rowLast>
	<th>Services</th>
	<td><%=bean.isServicesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbServicesApply value="Apply" <%=!bean.isServicesChanged() ? "disabled" : ""%>></td>
</tr>
</tbody>
</table><%--
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
--%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>

<%--@include file="/common/header.jsp"%>
	<h1 align="center">SMSC administration</h1>
<%@include file="/common/footer.jsp"--%>
