<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
%><%@ page import="ru.novosoft.smsc.jsp.smsc.Index"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
is_SMSC_status_needed = true;
if (request.isUserInRole("super-admin"))
{
TITLE = "Configuration status";
}

switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case Index.RESULT_DONE:
		response.sendRedirect(CPATH+"/index.jsp");
		return;
	case Index.RESULT_OK:
		//STATUS.append("Ok");
		break;
	case Index.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
MENU0_SELECTION = "MENU0_SMSC_Status";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<div class="content">
<% if (request.isUserInRole("apply"))
{%>
<table cellspacing=0 cellpadding=0 class=list>
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
<% if (request.isUserInRole("routes"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td>Routes</td>
	<td><%=bean.isRoutesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbRoutesApply value="Apply" <%=!bean.isRoutesChanged() ? "disabled" : ""%>></td>
</tr>
<%}
if (request.isUserInRole("subjects"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td>Subjects</td>
	<td><%=bean.isSubjectsChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbSubjectsApply value="Apply" <%=!bean.isSubjectsChanged() ? "disabled" : ""%>></td>
</tr>
<%}
if (request.isUserInRole("aliases"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td>Aliases</td>
	<td><%=bean.isAliasesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbAliasesApply value="Apply" <%=!bean.isAliasesChanged() ? "disabled" : ""%>></td>
</tr>
<%}%>
<%--if (request.isUserInRole("profiles"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td>Profiles</td>
	<td><%=bean.isProfilesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbProfilesApply value="Apply" <%=!bean.isProfilesChanged() ? "disabled" : ""%>></td>
</tr>
<%}--%>
<%if (request.isUserInRole("hosts"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td>Hosts</td>
	<td><%=bean.isHostsChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbHostsApply value="Apply" <%=!bean.isHostsChanged() ? "disabled" : ""%>></td>
</tr>
<%}%>
<%--if (request.isUserInRole("services"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td>Services</td>
	<td><%=bean.isServicesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbServicesApply value="Apply" <%=!bean.isServicesChanged() ? "disabled" : ""%>></td>
</tr>
<%}--%>
<%if (request.isUserInRole("users"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td>Users</td>
	<td><%=bean.isUsersChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbUsersApply value="Apply" <%=!bean.isUsersChanged() ? "disabled" : ""%>></td>
</tr>
<%}
if (request.isUserInRole("smsc_service"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td>SMS Center</td>
	<td><%=bean.isSmscChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbSmscApply value="Apply" <%=!bean.isSmscChanged() ? "disabled" : ""%>></td>
</tr>
<%}%>
</tbody>
</table>
<%}%>
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>