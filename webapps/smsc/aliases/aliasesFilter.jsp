<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.AliasesFilter,
					  ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.AliasesFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Alias Filter";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
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
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "clickClear()");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Aliases ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=page_subtitle>Aliases</div></td></tr>
<%
int rowN = 0;
for (int i=0; i<bean.getAliases().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th></th>
	<td><input class=txtW name=aliases value="<%=bean.getAliases()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th><input class=btn type=submit name=mbAdd value="Add" title="Add new alias to filter"></th>
	<td><input class=txtW name=aliases validation="mask" onkeyup="resetValidation(this)"></td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Addresses ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=page_subtitle>Addresses</div></td></tr>
<%
rowN = 0;
for (int i=0; i<bean.getAddresses().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<th>address:</th>
	<td><input class=txtW name=addresses value="<%=bean.getAddresses()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<th><input class=btn type=submit name=mbAdd value="Add" title="Add new address to filter"></th>
	<td><input class=txtW name=addresses validation="mask" onkeyup="resetValidation(this)"></td>
</tr>

<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=2 class=secInner><div class=page_subtitle>Options</div></td></tr>
<%rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th>hide option:</th>
	<td><div class=select><select class=txt name=hide>
			<option value="<%=AliasFilter.HIDE_NOFILTER%>" <%=AliasFilter.HIDE_NOFILTER == bean.getHide() ? "selected" : ""%>>all</option>
			<option value="<%=AliasFilter.HIDE_SHOW_HIDE%>" <%=AliasFilter.HIDE_SHOW_HIDE == bean.getHide() ? "selected" : ""%>>show hided only</option>
			<option value="<%=AliasFilter.HIDE_SHOW_NOHIDE%>" <%=AliasFilter.HIDE_SHOW_NOHIDE == bean.getHide() ? "selected" : ""%>>show not hided only</option>
		</select></div></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "clickClear()");
page_menu_button(out, "mbCancel", "Cancel", "Cancel filter editing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>