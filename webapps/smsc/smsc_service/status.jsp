<%@ include file="/WEB-INF/inc/code_header.jsp"%><%
%><%@ page import="ru.novosoft.smsc.jsp.smsc.Index,
                   ru.novosoft.smsc.admin.Constants"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%
ServiceIDForShowStatus = Constants.SMSC_SME_ID;
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
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply");
page_menu_space(out);
page_menu_end(out);
%><div class="content">
<% if (request.isUserInRole("apply"))
{%>
<table cellspacing=0 cellpadding=0 class=list>
<col width=1%>
<col width="70%">
<col width="30%" align=center>
<col width="1%">
<thead>
<tr>
	<th>&nbsp;</th>
	<th>Configuration</th>
	<th>Status</th>
</tr>
</thead>
<tbody>
<%int rowN = 0;%>
<% if (request.isUserInRole("routes"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check type=checkbox name=checks value=routes <%=!bean.isRoutesChanged() ? "disabled" : ""%>></td>
	<td>Routes</td>
	<td><%=bean.isRoutesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
</tr>
<%}
if (request.isUserInRole("subjects"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check type=checkbox name=checks value=subjects <%=!bean.isSubjectsChanged() ? "disabled" : ""%>></td>
	<td>Subjects</td>
	<td><%=bean.isSubjectsChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
</tr>
<%}
if (request.isUserInRole("aliases"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check type=checkbox name=checks value=aliases <%=!bean.isAliasesChanged() ? "disabled" : ""%>></td>
	<td>Aliases</td>
	<td><%=bean.isAliasesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
</tr>
<%}%>
<%--if (request.isUserInRole("profiles"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check type=checkbox name=checks value=profiles <%=!bean.isProfilesChanged() ? "disabled" : ""%>></td>
	<td>Profiles</td>
	<td><%=bean.isProfilesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
</tr>
<%}--%>
<%if (request.isUserInRole("hosts"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check type=checkbox name=checks value=hosts <%=!bean.isHostsChanged() ? "disabled" : ""%>></td>
	<td>Hosts</td>
	<td><%=bean.isHostsChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
</tr>
<%}%>
<%--if (request.isUserInRole("services"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check type=checkbox name=checks value=services <%=!bean.isServicesChanged() ? "disabled" : ""%>></td>
	<td>Services</td>
	<td><%=bean.isServicesChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
</tr>
<%}--%>
<%if (request.isUserInRole("users"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check type=checkbox name=checks value=users <%=!bean.isUsersChanged() ? "disabled" : ""%>></td>
	<td>Users</td>
	<td><%=bean.isUsersChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
</tr>
<%}
if (request.isUserInRole("smsc_service"))
{%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=check type=checkbox name=checks value=smsc <%=!bean.isSmscChanged() ? "disabled" : ""%>></td>
	<td>SMS Center</td>
	<td><%=bean.isSmscChanged() ? "<span class=Cf00>changed</span>" : "clear"%></td>
</tr>
<%}%>
</tbody>
</table>
<%}%>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>