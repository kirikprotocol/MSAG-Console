<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.aliases.AliasesFilter,
					  ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.aliases.AliasesFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("aliases.filterTitle");
switch(bean.process(request))
{
	case AliasesFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_ALIASES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbApply",  "common.buttons.apply",  "common.buttons.applyFilter");
page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearFilter", "clickClear()");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelFilterEditing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Aliases ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle><%=getLocString("common.titles.aliases")%></div>
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
	<td><%addButton(out, "mbAdd", "Add", "aliases.addAliasHint");%></td>
</tr>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Addresses ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle><%=getLocString("common.titles.addresses")%></div>
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
	<td><%addButton(out, "mbAdd", "Add", "aliases.addAddressHint");%></td>
</tr>
</table>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<div class=page_subtitle><%=getLocString("common.titles.options")%></div>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width=1px>
<%rowN = 0;%>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("aliases.hideOption")%>:</th>
	<td><div class=select><select class=txt name=hide>
			<option value="<%=AliasFilter.HIDE_NOFILTER%>" <%=AliasFilter.HIDE_NOFILTER == bean.getHideByte() ? "selected" : ""%>><%=getLocString("aliases.hideOption.all")%></option>
			<option value="<%=AliasFilter.HIDE_SHOW_HIDE%>" <%=AliasFilter.HIDE_SHOW_HIDE == bean.getHideByte() ? "selected" : ""%>><%=getLocString("aliases.hideOption.hide")%></option>
			<option value="<%=AliasFilter.HIDE_SHOW_NOHIDE%>" <%=AliasFilter.HIDE_SHOW_NOHIDE == bean.getHideByte() ? "selected" : ""%>><%=getLocString("aliases.hideOption.noHide")%></option>
		</select></div></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbApply",  "common.buttons.apply",  "common.buttons.applyFilter");
page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearFilter", "clickClear()");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelFilterEditing", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>