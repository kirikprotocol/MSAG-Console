<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.AliasesFilter,
                 ru.novosoft.smsc.jsp.util.tables.impl.AliasFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.AliasesFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
switch(bean.process((ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext"), errorMessages))
{
	case AliasesFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case AliasesFilter.RESULT_OK:
		STATUS.append("Ok");
		break;
	case AliasesFilter.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ALIASES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%@ include file="/WEB-INF/inc/html_3_middle.jsp"%>
<h1>Aliases filter</h1>
<%@ include file="/WEB-INF/inc/messages.jsp"%>
<table class=frm0 cellspacing=0 width="100%">
<col width="10%" align=right>
<col width="85%">
<col width="10%">
<%
for (int i=0; i<bean.getAliases().length; i++)
{
%>
<tr class=row0>
	<th>alias:</th>
	<td><input class=txtW name=aliases value="<%=bean.getAliases()[i]%>"></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr class=row0>
	<th>alias:</th>
	<td><input class=txtW name=aliases></td>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new alias to filter"></td>
</tr>
<%
for (int i=0; i<bean.getAddresses().length; i++)
{
%>
<tr class=row0>
	<th>address:</th>
	<td><input class=txtW name=addresses value="<%=bean.getAddresses()[i]%>"></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr class=row0>
	<th>address:</th>
	<td><input class=txtW name=addresses></td>
	<td><input class=btn type=submit name=mbAdd value="Add" title="Add new address to filter"></td>
</tr>
<tr class=row0>
	<th>hide option:</th>
	<td><select class=txt name=hide>
			<option value="<%=AliasFilter.HIDE_NOFILTER%>" <%=AliasFilter.HIDE_NOFILTER == bean.getHide() ? "selected" : ""%>>all</option>
			<option value="<%=AliasFilter.HIDE_SHOW_HIDE%>" <%=AliasFilter.HIDE_SHOW_HIDE == bean.getHide() ? "selected" : ""%>>show hided only</option>
			<option value="<%=AliasFilter.HIDE_SHOW_NOHIDE%>" <%=AliasFilter.HIDE_SHOW_NOHIDE == bean.getHide() ? "selected" : ""%>>show not hided only</option>
		</select></td>
	<td>&nbsp;</td>
</tr>
</table>
<div class=but0>
<input class=btn type=submit name=mbApply value="Apply" title="Apply filter">
<input class=btn type=submit name=mbCancel value="Cancel">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>