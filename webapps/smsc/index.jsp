<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
%><%@ page import="ru.novosoft.smsc.jsp.smsc.Index,"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
if (request.isUserInRole("super-admin"))
{
TITLE = "Configuration status";
}

switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case Index.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case Index.RESULT_OK:
		//STATUS.append("Ok");
		break;
	case Index.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
MENU0_SELECTION = "MENU0_HOME";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<% if (request.isUserInRole("super-admin"))
{%>
<table class=secRep cellspacing=1 cellpadding=0 width="100%">
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
<tr class=row<%=(rowN++)&1%>>
	<th>Services</th>
	<td><%=bean.isServicesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
	<td><input class=btn type=submit name=mbServicesApply value="Apply" <%=!bean.isServicesChanged() ? "disabled" : ""%>></td>
</tr>
</tbody>
</table>
<%}%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>