<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%
String hostName = request.getParameter("hostName");
if (request.getParameter("mbCancel") != null)
{
	if (hostName != null)
		response.sendRedirect(CPATH+"/hosts/hostView.jsp?hostName=" + hostName);
	else
		response.sendRedirect("index.jsp");
	return;
} else if (request.getParameter("mbNext") != null)
{
	if ("internal".equals(request.getParameter("serviceType"))) {
		response.sendRedirect("serviceAddInternal.jsp" + (hostName != null ? ("?hostName="+hostName) : ""));
		return;
	} else if ("external".equals(request.getParameter("serviceType"))) {
		response.sendRedirect("serviceAddExternal.jsp" + (hostName != null ? ("?hostName="+hostName) : ""));
		return;
	} else if ("external_adm".equals(request.getParameter("serviceType"))) {
		response.sendRedirect("serviceAddExternalAdm.jsp" + (hostName != null ? ("?hostName="+hostName) : ""));
		return;
	}
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_SERVICES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<%@ include file="/WEB-INF/inc/messages.jsp"%>

<%if (hostName != null)
{
%><input type=hidden name=hostName value="<%=hostName%>"><%
}%>
<h1>Add service: step 1</h1>
Select service type:<br>
<input type=radio name=serviceType value="internal">Internal<br>
<!--input type=radio name=serviceType value="external">External<br-->
<input type=radio name=serviceType value="external_adm" checked>External Administrable<br>

<div class=but0>
<input class=btn type=submit name=mbNext value="Next" title="Next page">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
