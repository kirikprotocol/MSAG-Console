<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesFilter,
                 ru.novosoft.smsc.admin.profiler.Profile"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.profiles.ProfilesFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = "Profiles filter";
switch(bean.process(appContext, errorMessages, loginedUserPrincipal))
{
	case ProfilesFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProfilesFilter.RESULT_OK:
		STATUS.append("Ok");
		break;
	case ProfilesFilter.RESULT_ERROR:
		STATUS.append("<span class=CF00>Error</span>");
		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROFILES";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "clickClear()");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="15%">
<col width="85%">
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Masks ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=3><div class=page_subtitle>Masks</div></td></tr>
<%
int row = 0;
for (int i=0; i<bean.getMasks().length; i++)
{
%>
<tr class=row<%=(row++)&1%>>
	<th>&nbsp;</th>
	<td colspan=2><input class=txt name=masks value="<%=bean.getMasks()[i]%>" validation="mask" onkeyup="resetValidation(this)"></td>
</tr>
<%}%>
<tr class=row<%=(row++)&1%>>
	<th>&nbsp;</th>
	<td><input class=txt name=masks validation="mask" onkeyup="resetValidation(this)"></td>
	<td><%addButton(out, "mbAdd", "Add", "Add new mask to filter");%></td>
</tr>
<%--~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Options ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%>
<tr><td colspan=3 class=secInner><div class=page_subtitle>Options</div></td></tr>
<%row = 0;%>
<tr class=row<%=(row++)&1%>>
	<th>codepage:</th>
	<td colspan=2><div class=select><select class=txt name=codepage>
			<option value="-1" <%=-1 == bean.getCodepageByte() ? "selected" : ""%>>all</option>
			<option value="<%=Profile.CODEPAGE_Default%>" <%=Profile.CODEPAGE_Default == bean.getCodepageByte() ? "selected" : ""%>>Default</option>
			<option value="<%=Profile.CODEPAGE_UCS2   %>" <%=Profile.CODEPAGE_UCS2    == bean.getCodepageByte() ? "selected" : ""%>>UCS2</option>
		</select></div></td>
</tr>
<tr class=row<%=(row++)&1%>>
	<th>report&nbsp;mode:</th>
	<td colspan=2><div class=select><select class=txt name=reportinfo>
			<option value="-1" <%=-1 == bean.getReportinfoByte() ? "selected" : ""%>>all</option>
			<option value="<%=Profile.REPORT_OPTION_None%>" <%=Profile.REPORT_OPTION_None == bean.getReportinfoByte() ? "selected" : ""%>>none</option>
			<option value="<%=Profile.REPORT_OPTION_Full%>" <%=Profile.REPORT_OPTION_Full == bean.getReportinfoByte() ? "selected" : ""%>>full</option>
		</select></div></td>
</tr>
<tr><td colspan=3><div class=page_subtitle>Locales</div></td></tr>
<%for (Iterator i = bean.getRegisteredLocales().iterator(); i.hasNext();)
{
	String registeredLocale = (String) i.next();
	String encRegisteredLocale = StringEncoderDecoder.encode(registeredLocale);
	%>
	<tr class=row<%=(row++)&1%>>
		<th>&nbsp;</th>
		<td colspan=2><input class=check id="reg_loc_<%=encRegisteredLocale%>" type=checkbox name=checkedLocales value="<%=encRegisteredLocale%>" <%=bean.isLocaleChecked(registeredLocale) ? "checked" : ""%>>&nbsp;<label for="reg_loc_<%=encRegisteredLocale%>"><%=encRegisteredLocale%></label></td>
	</tr><%
}%>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(out, "mbApply",  "Apply",  "Apply filter");
page_menu_button(out, "mbClear", "Clear", "Clear filter", "clickClear()");
page_menu_button(out, "mbCancel", "Cancel", "Cancel", "clickCancel()");
page_menu_space(out);
page_menu_end(out);
%><%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>
