<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.AliasesFilter,
					  ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.AliasesFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Edit Alias Filter";
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
<table class=secRep cellspacing=0 cellspadding=1 width="100%">
<col width="15%" align=right>
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Aliases ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secList>Aliases</div></td></tr>
<%
int rowN = 0;
for (int i=0; i<bean.getAliases().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label></th>
	<td><input class=txtW name=aliases value="<%=bean.getAliases()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label><input class=btn type=submit name=mbAdd value="Add" title="Add new alias to filter"></th>
	<td><input class=txtW name=aliases validation="mask" onkeyup="resetValidation(this)"></td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Addresses ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secList>Addresses</div></td></tr>
<%
rowN = 0;
for (int i=0; i<bean.getAddresses().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>address:</th>
	<td><input class=txtW name=addresses value="<%=bean.getAddresses()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label><input class=btn type=submit name=mbAdd value="Add" title="Add new address to filter"></th>
	<td><input class=txtW name=addresses validation="mask" onkeyup="resetValidation(this)"></td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=secView>Options</div></td></tr>
<%rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th class=label>hide option:</th>
	<td><div class=select><select class=txt name=hide>
			<option value="<%=AliasFilter.HIDE_NOFILTER%>" <%=AliasFilter.HIDE_NOFILTER == bean.getHide() ? "selected" : ""%>>all</option>
			<option value="<%=AliasFilter.HIDE_SHOW_HIDE%>" <%=AliasFilter.HIDE_SHOW_HIDE == bean.getHide() ? "selected" : ""%>>show hided only</option>
			<option value="<%=AliasFilter.HIDE_SHOW_NOHIDE%>" <%=AliasFilter.HIDE_SHOW_NOHIDE == bean.getHide() ? "selected" : ""%>>show not hided only</option>
		</select></div></td>
</tr>
</table>
<div class=secButtons>
<input class=btn type=submit name=mbApply value="Apply" title="Apply filter">
<input class=btn type=submit name=mbClear value="Clear" title="Clear filter">
<input class=btn type=submit name=mbCancel value="Cancel" onClick="clickCancel()">
</div>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>