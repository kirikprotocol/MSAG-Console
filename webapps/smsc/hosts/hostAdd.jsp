<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.HostEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.HostAdd"/>
<%try {%><jsp:setProperty name="bean" property="*"/><%} catch (Throwable t){}%>
<%
TITLE = "Add new host";
switch (bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case HostEdit.RESULT_ERROR:
		break;
	case HostEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case HostEdit.RESULT_OK:
		STATUS.append("Ok");
		break;
	default:
}

%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Add host",  "Add new host");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<div class=page_subtitle>Host infromation</div>
<table class=properties_list cellspacing=0 cellpadding=0>
<col width="15%" align=right>
<col width="85%">
<tbody>
<tr class=row0>
	<th>host name:</th>
	<td><input class=txt name=hostName value="<%=bean.getHostName()%>"></td>
</tr>
<tr class=rowLast>
	<th>host port:</th>
	<td><input class=txt name=port size=10 value="<%=bean.getPort()%>" validation="port" onkeyup="resetValidation(this)"></td>
</tr>
<tr class=row0>
	<th>host services folder:</th>
	<td><input class=txt name=hostName value="<%=bean.getHostServicesFolder()%>"></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Add host",  "Add new host");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
