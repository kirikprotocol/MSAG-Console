<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.AliasesEdit"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.AliasesEdit"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case AliasesEdit.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case AliasesEdit.RESULT_OK:
		STATUS.append("Ok");
		break;
	case AliasesEdit.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ALIASES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Edit alias</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<input type=hidden name=oldAlias value="<%=bean.getOldAlias()%>">
<input type=hidden name=oldAddress value="<%=bean.getOldAddress()%>">
<input type=hidden name=oldHide value=<%=bean.isOldHide()%>>
<table class=frm0 cellspacing=0 width="100%">
<col width="15%" align=right>
<col width="85%">
<tr class=row0>
	<th>alias:</th>
	<td><input class=txtW name=alias value="<%=bean.getAlias()%>"></td>
</tr>
<tr class=row0>
	<th>address:</th>
	<td><input class=txtW name=address value="<%=bean.getAddress()%>"></td>
</tr>
<tr class=rowLast>
	<th>hide:</th>
	<td><input class=txtW type=checkbox name=hide <%=bean.isHide() ? "checked" : ""%>></td>
</tr>
</table>
<div class=but0>
<input class=btn type=submit name=mbSave value="Save" title="Save alias">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>