<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@page import="ru.novosoft.smsc.jsp.smsc.hosts.HostEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.hosts.HostAdd"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch (bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
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
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<input type=hidden ID=jbutton value="jbutton">
<span class=CF00><%@ include file="/WEB-INF/inc/messages.jsp"%></span>
<div class=secInfo>Host infromation</div>
<table class=secRep cellspacing=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<tbody>
<tr class=row0>
	<th class=label>host name:</th>
	<td><input class=txtW name=hostName value="<%=bean.getHostName()%>"></td>
</tr>
<tr class=rowLast>
	<th>host port:</th>
	<td><input class=txt name=port size=10 value="<%=bean.getPort()%>"></td>
</tr>
</table><div class=secButtons>
<input type="submit" name="mbSave"   value="Save"   class="btn64" title="Save information">
<input type="submit" name="mbCancel" value="Cancel" class="btn64" title="Return to host view">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
