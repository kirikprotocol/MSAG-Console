<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.HostEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.HostEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Host \""+bean.getHostName()+":"+bean.getPort()+"\" edit";
switch (bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case HostEdit.RESULT_ERROR:
		break;
	case HostEdit.RESULT_DONE:
		response.sendRedirect("hostView.jsp?hostName="+bean.getHostName());
		return;
	case HostEdit.RESULT_OK:
		break;
	default:
}

%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Save",  "Apply changes");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<table class=list cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr class=row0>
	<th>host name:</th>
	<td><input class=txtW name=hostName value="<%=bean.getHostName()%>" readonly></td>
</tr>
<tr class=rowLast>
	<th>host port:</th>
	<td><input class=txt name=port value="<%=bean.getPort()%>" validation="port" onkeyup="resetValidation(this)"></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbSave",  "Save",  "Apply changes");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
