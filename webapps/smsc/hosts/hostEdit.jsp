<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.HostEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.HostEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%bean.setAppContext((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"));%>
<%
switch(bean.process())
{
	case HostEdit.RESULT_ERROR:
		STATUS.append("Error:<span class=statusError>"+bean.getErrorMessage()+"</span>");
		break;
	case HostEdit.RESULT_DONE:
		response.sendRedirect("hostView.jsp?hostName="+bean.getHostName());
		return;
	case HostEdit.RESULT_OK:
		break;
	default:
		STATUS.append("Error:<span class=statusError>unknown status</span>");
}

%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<input type=hidden ID=jbutton value="jbutton">
<h1>Edit host</h1>
<table class=frm0 cellspacing=0 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr class=row0>
	<th>host name:</th>
	<td><input class=txtW name=hostName value="<%=bean.getHostName()%>" readonly></td>
</tr>
<tr class=rowLast>
	<th>host port:</th>
	<td><input class=txt name=port value="<%=bean.getPort()%>"></td>
</tr>
</table><div class=but0>
<input type="submit" name="mbSave"   value="Save"   class="btn64" title="Save information">
<input type="submit" name="mbCancel" value="Cancel" class="btn64" title="Return to host view">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
