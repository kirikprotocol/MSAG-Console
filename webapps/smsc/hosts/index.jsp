<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.Index"/>
<jsp:setProperty name="bean" property="*"/>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.Index"%>
<%
TITLE = "Hosts";
switch (bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case Index.RESULT_OK:
		STATUS.append("hosts:").append(bean.getHostNames().size());
		break;
	case Index.RESULT_DONE:
		response.sendRedirect(CPATH+"/index.jsp");
		return;
	case Index.RESULT_VIEW:
		response.sendRedirect("hostView.jsp?hostName=" + URLEncoder.encode(bean.getHostName()));
		return;
	case Index.RESULT_ADD:
		response.sendRedirect("hostAdd.jsp");
		return;
	case Index.RESULT_ERROR:
		// do nothing there, show errors below
		STATUS.append("<span class=CF00>Error</span>");
		break;
	default:
		errorMessages.add(new SMSCJspException(SMSCErrors.error.services.unknownAction));
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbAdd",  "Add host",  "Add new host");
page_menu_button(out, "mbDelete", "Delete host(s)", "Delete selected host(s)");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<input type=hidden name=hostName>

<script>
function viewHost(hostName)
{
	document.all.jbutton.value = "view";
	document.all.jbutton.name = "mbView";
	opForm.hostName.value = hostName;
	opForm.submit();
	return false;
}
</script>
<span class=CF00></span>
<table class=list cellspacing=0 cellpadding=0>
<col width="1%">
<col width="79%">
<col width="1%">
<col width="10%">
<thead>
<tr>
	<th class=ico><img src="<%=CPATH%>/img/ico16_checked_sa.gif" class=ico16 alt=""></th>
	<th>name</th>
	<th>port</th>
	<th title="services ( total / running)">services</th>
</tr>
</thead>
<tbody>
<%{
int row=0;
List hostIds = Arrays.asList(bean.getHostIds());
Collection hostNames = bean.getHostNames();
for(Iterator i = hostNames.iterator(); i.hasNext();row++)
{
String hostName = (String) i.next();
%>
<tr class=row<%=row&1%>>
	<td class=check><input class=check type=checkbox name=hostIds value="<%=hostName%>" <%=hostIds.contains(hostName) ? "checked" : ""%>></td>
	<td class=name><a href="#" title="View host info" onClick='return viewHost("<%=hostName%>")'><%=hostName%></a></td>
	<td class=num><%=bean.getHostPort(hostName)%></td>
	<td><span class=C00F title="total"><%=bean.getServicesTotal(hostName)%></span> / <span class=C080 title="running"><%=bean.getServicesRunning(hostName)%></span></td>
</tr>
<%}}%>
</tbody>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbAdd",  "Add host",  "Add new host");
page_menu_button(out, "mbDelete", "Delete host(s)", "Delete selected host(s)");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>