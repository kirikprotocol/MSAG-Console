<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.HostEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.HostEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch (bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
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

TITLE = "Editing host \""+bean.getHostName()+":"+bean.getPort()+"\"";
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_HOSTS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>

 
<span class=CF00></span>
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
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
</table><div class=secButtons>
<input type="submit" name="mbSave"   value="Save"   class="btn64" title="Save information">
<input type="submit" name="mbCancel" value="Cancel" class="btn64" title="Return to host view" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
