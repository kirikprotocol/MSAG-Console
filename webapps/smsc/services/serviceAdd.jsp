<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%
TITLE = "Add service: step 1";
String hostName = request.getParameter("hostName");
if (request.getParameter("mbCancel") != null)
{
	System.err.println("AddService page :Cancel\n");
	if (hostName != null)
		response.sendRedirect(CPATH+"/hosts/hostView.jsp?hostName=" + hostName);
	else
		response.sendRedirect("index.jsp");
	return;
} else if (request.getParameter("mbNext") != null)
{
	System.err.println("AddService page :next\n");
	if ("internal".equals(request.getParameter("serviceType"))) {
		response.sendRedirect("serviceAddInternal.jsp" + (hostName != null ? ("?hostName="+hostName) : ""));
		return;
	} else if ("external".equals(request.getParameter("serviceType"))) {
		response.sendRedirect("serviceAddExternal.jsp" + (hostName != null ? ("?hostName="+hostName) : ""));
		return;
	} else if ("external_adm".equals(request.getParameter("serviceType"))) {
		response.sendRedirect("serviceAddExternalAdm.jsp?stage=1" + (hostName != null ? ("&hostName="+hostName) : ""));
		return;
	}
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbNext",  "Next",  "Next page");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel");
page_menu_space(out);
page_menu_end(out);
%><div class=content>
<%if (hostName != null)
{
%><input type=hidden name=hostName value="<%=hostName%>"><%
}%>
<div class=secQuestion>Select service type:</div>
<table class=list cellspacing=1 width="100%">
<tbody>
<tr class=row0>
	<td><input class=radio type=radio name=serviceType id=internal value=internal>&nbsp;<label for=internal>Internal</label></td>
</tr>
<tr class=row1>
	<td><input class=radio type=radio name=serviceType id=regular value=internal checked>&nbsp;<label for=external_adm>Regular</label></td>
</tr>
<tr class=row0>
	<td><input class=radio type=radio name=serviceType id=external_adm value="external_adm">&nbsp;<label for=external_adm>External&nbsp;Administrable</label></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbNext",  "Next",  "Next page");
page_menu_button(session, out, "mbCancel", "Cancel", "Cancel");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
