<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ include file="/WEB-INF/inc/buttons.jsp"%>
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
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Aliases ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>Aliases</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width=1px>
<col align=left>
<%
int rowN = 0;
for (int i=0; i<bean.getAliases().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=txt name=aliases value="<%=bean.getAliases()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
	<td>&nbsp;</td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=txt name=aliases validation="mask" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "Add new alias to filter");%></td>
</tr>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Addresses ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>Addresses</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width=1px>
<col align=left>
<%
rowN = 0;
for (int i=0; i<bean.getAddresses().length; i++)
{
%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=txt name=addresses value="<%=bean.getAddresses()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
	<td></td>
</tr>
<%}%>
<tr class=row<%=(rowN++)&1%>>
	<td><input class=txt name=addresses validation="mask" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "Add new address to filter");%></td>
</tr>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle>Options</div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width=1px>
<%rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th>hide&nbsp;option:</th>
	<td><div class=select><select class=txt name=hide>
			<option value="<%=AliasFilter.HIDE_NOFILTER%>" <%=AliasFilter.HIDE_NOFILTER == bean.getHideByte() ? "selected" : ""%>>all</option>
			<option value="<%=AliasFilter.HIDE_SHOW_HIDE%>" <%=AliasFilter.HIDE_SHOW_HIDE == bean.getHideByte() ? "selected" : ""%>>show hided only</option>
			<option value="<%=AliasFilter.HIDE_SHOW_NOHIDE%>" <%=AliasFilter.HIDE_SHOW_NOHIDE == bean.getHideByte() ? "selected" : ""%>>show not hided only</option>
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