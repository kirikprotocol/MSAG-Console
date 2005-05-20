<%@ include file="/WEB-INF/inc/code_header.jsp"%>
<%@ page import="ru.novosoft.smsc.jsp.smsc.providers.ProvidersFilter"%>
<jsp:useBean id="bean" class="ru.novosoft.smsc.jsp.smsc.providers.ProvidersFilter"/>
<jsp:setProperty name="bean" property="*"/>
<%
TITLE = getLocString("providers.filterTitle");
switch(bean.process(request))
{
	case ProvidersFilter.RESULT_DONE:
		response.sendRedirect("index.jsp");
		return;
	case ProvidersFilter.RESULT_OK:

		break;
	case ProvidersFilter.RESULT_ERROR:

		break;
}
%><%--DESING PARAMETERS~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~--%><%
MENU0_SELECTION = "MENU0_PROVIDERS";
%><%@ include file="/WEB-INF/inc/html_3_header.jsp"%>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbApply",  "common.buttons.apply",  "common.buttons.applyFilter");
page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearFilter");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelFilterEditing");
page_menu_space(out);
page_menu_end(out);
%>
<div class=content>
<input type="Hidden" name=initialized value="true">
<%int rowN = 0;%>
<table class=properties_list cellspacing=0 cellspadding=0>
<col width="15%">
<col width="85%">
<%--<tr><td colspan=2 class=secInner><div class=secList>Default SMEs</div></td></tr>--%>
<tr class=row<%=(rowN++)&1%>>
	<th><%=getLocString("common.util.Name")%>:</th>
	<td><input title="<%=getLocString("common.hints.filter")%>" class=txt name=loginFilter value="<%=bean.getNameFilter()%>"></td>
</tr>
</table>
</div>
<%
page_menu_begin(out);
page_menu_button(session, out, "mbApply",  "common.buttons.apply",  "common.buttons.applyFilter");
page_menu_button(session, out, "mbClear", "common.buttons.clear", "common.buttons.clearFilter");
page_menu_button(session, out, "mbCancel", "common.buttons.cancel", "common.buttons.cancelFilterEditing");
page_menu_space(out);
page_menu_end(out);
%>
<%@ include file="/WEB-INF/inc/html_3_footer.jsp"%>
<%@ include file="/WEB-INF/inc/code_footer.jsp"%>